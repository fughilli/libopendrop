#include "preset/preset_blender.h"

#include <sstream>

#include "shader/blit.fsh.h"
#include "shader/blit.vsh.h"
#include "primitive/rectangle.h"
#include "third_party/gl_helper.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"

namespace opendrop {
PresetActivation::PresetActivation(
    std::shared_ptr<Preset> preset,
    std::shared_ptr<gl::GlRenderTarget> render_target, float minimum_duration_s,
    float transition_duration_s)
    : preset_(std::move(preset)),
      render_target_(std::move(render_target)),
      state_(PresetActivationState::kTransitionIn),
      expiry_timer_(minimum_duration_s - transition_duration_s * 2),
      transition_timer_(transition_duration_s) {
  CHECK(minimum_duration_s >= (transition_duration_s * 2))
      << "Minimum duration must be at least twice the transition duration.";
}

PresetActivationState PresetActivation::Update(float dt) {
  switch (state_) {
    case PresetActivationState::kTransitionIn:
      if (transition_timer_.Update(dt).IsDue()) {
        state_ = kIn;
        expiry_timer_.Reset();
      }
      break;

    case PresetActivationState::kIn:
      if (expiry_timer_.Update(dt).IsDue()) {
        state_ = kAwaitingTransitionOut;
      }
      break;

    case PresetActivationState::kAwaitingTransitionOut:
      break;

    case PresetActivationState::kTransitionOut:
      if (transition_timer_.Update(dt).IsDue()) {
        state_ = PresetActivationState::kOut;
      }
      break;

    case PresetActivationState::kOut:
      break;
  }

  return state_;
}

void PresetActivation::TriggerTransitionOut() {
  if (state_ == PresetActivationState::kOut) return;
  if (state_ == PresetActivationState::kTransitionIn)
    maximal_mix_coeff_ = transition_timer_.FractionDue();

  state_ = kTransitionOut;
  transition_timer_.Reset();
}

float PresetActivation::GetMixingCoefficient() const {
  switch (state_) {
    case PresetActivationState::kTransitionIn:
      return transition_timer_.FractionDue();

    case PresetActivationState::kIn:
    case PresetActivationState::kAwaitingTransitionOut:
      return maximal_mix_coeff_;

    case PresetActivationState::kTransitionOut:
      return (1.0f - transition_timer_.FractionDue()) * maximal_mix_coeff_;

    case PresetActivationState::kOut:
      return 0.0f;
  }
}

PresetBlender::PresetBlender(int width, int height)
    : width_(width), height_(height) {
  absl::StatusOr<std::shared_ptr<gl::GlProgram>> status_or_blit_program =
      gl::GlProgram::MakeShared(blit_vsh::Code(), blit_fsh::Code());
  CHECK(status_or_blit_program.ok()) << "Failed to create blit program";
  blit_program_ = *status_or_blit_program;
}

// Draws a single frame of blended preset output.
void PresetBlender::DrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  Update(state->dt());

  {
    std::stringstream print_stream;
    for (auto activation : preset_activations_) {
      print_stream << activation.GetMixingCoefficient() << ", ";
    }
    LOG(DEBUG) << "mixing coefficients: " << print_stream.str();
  }

  for (auto activation : preset_activations_) {
    if (activation.GetMixingCoefficient() == 0) {
      continue;
    }

    activation.preset()->DrawFrame(samples, state, 1.0f,
                                   activation.render_target());
  }

  {
    auto output_activation = output_render_target->Activate();

    unsigned int black_color[4] = {0, 0, 0, 0};
    glClearBufferuiv(GL_COLOR, 0, black_color);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    for (auto activation : preset_activations_) {
      // If the mixing coefficient is 0, skip blitting the preset output.
      if (activation.GetMixingCoefficient() == 0) {
        continue;
      }

      blit_program_->Use();
      // Bind the source texture and alpha value.
      GlBindRenderTargetTextureToUniform(blit_program_, "source_texture",
                                         activation.render_target(),
                                         gl::GlTextureBindingOptions());
      glUniform1f(
          glGetUniformLocation(blit_program_->program_handle(), "alpha"),
          activation.GetMixingCoefficient());

      rectangle_.Draw();
    }

    glDisable(GL_BLEND);
  }
}

void PresetBlender::UpdateGeometry(int width, int height) {
  width_ = width;
  height_ = height;
  for (auto activation : preset_activations_) {
    LOG(DEBUG) << "UpdateGeometry on activation for preset "
               << activation.preset()->name();
    activation.preset()->UpdateGeometry(width_, height_);
    activation.render_target()->UpdateGeometry(width_, height_);
  }
}

void PresetBlender::Update(float dt) {
  static std::vector<PresetActivation*> activations_waiting;
  activations_waiting.clear();

  int activations_in = 0;
  for (auto activation_iter = preset_activations_.begin();
       activation_iter != preset_activations_.end();) {
    PresetActivationState state = activation_iter->Update(dt);

    if (state == kTransitionIn || state == kIn) {
      ++activations_in;
    }

    // If the preset is already transitioned out, clean it up.
    if (state == PresetActivationState::kOut) {
      activation_iter = preset_activations_.erase(activation_iter);
      continue;
    }

    // The preset is awaiting a transition out. Add it to the waiting list.
    if (state == PresetActivationState::kAwaitingTransitionOut) {
      activations_waiting.push_back(&(*activation_iter));
    }

    ++activation_iter;
  }

  // If there's at least one preset that's in, we can transition out all
  // waiting presets.
  if (activations_in > 0) {
    LOG(DEBUG) << "Transitioning out " << activations_waiting.size()
               << " presets.";
    for (auto activation : activations_waiting) {
      LOG(DEBUG) << "Transitioned out activation for preset "
                 << activation->preset()->name();
      activation->TriggerTransitionOut();
    }
    return;
  }

  // Otherwise, transition out all but 1.
  if (!activations_waiting.empty()) {
    for (auto activation_iter = std::next(activations_waiting.begin());
         activation_iter != activations_waiting.end(); ++activation_iter) {
      LOG(DEBUG) << "Transitioned out activation for preset "
                 << (*activation_iter)->preset()->name();
      (*activation_iter)->TriggerTransitionOut();
    }
  }
}

int PresetBlender::QueryPresetCount(std::string_view name) {
  int count = 0;
  for (PresetActivation& activation : preset_activations_) {
    if (activation.preset()->name() == name) ++count;
  }
  return count;
}

void PresetBlender::TransitionOutAll() {
  for (PresetActivation& activation : preset_activations_) {
    activation.TriggerTransitionOut();
  }
}

}  // namespace opendrop
