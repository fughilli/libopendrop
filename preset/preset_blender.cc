#include "libopendrop/preset/preset_blender.h"

#include <sstream>

#include "libopendrop/blit.fsh.h"
#include "libopendrop/blit.vsh.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"

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
  CHECK(state_ == kAwaitingTransitionOut || state_ == kIn)
      << "`TriggerTransitionOut` called when state is not one of "
         "`kAwaitingTransitionOut` or `kIn`. Actual state is: "
      << state_;

  state_ = kTransitionOut;
  transition_timer_.Reset();
}

float PresetActivation::GetMixingCoefficient() const {
  switch (state_) {
    case PresetActivationState::kTransitionIn:
      return transition_timer_.FractionDue();

    case PresetActivationState::kIn:
    case PresetActivationState::kAwaitingTransitionOut:
      return 1.0f;

    case PresetActivationState::kTransitionOut:
      return 1.0f - transition_timer_.FractionDue();

    case PresetActivationState::kOut:
      return 0.0f;
  }
}

PresetBlender::PresetBlender(int width, int height)
    : width_(width), height_(height) {
  blit_program_ = gl::GlProgram::MakeShared(blit_vsh::Code(), blit_fsh::Code());
  CHECK_NULL(blit_program_) << "Failed to create blit program";
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

    activation.preset()->DrawFrame(samples, state,
                                   activation.GetMixingCoefficient(),
                                   activation.render_target());
  }

  {
    auto output_activation = output_render_target->Activate();

    unsigned int black_color[4] = {0, 0, 0, 0};
    glClearBufferuiv(GL_COLOR, 0, black_color);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
    glEnable(GL_BLEND);

    blit_program_->Use();
    for (auto activation : preset_activations_) {
      if (activation.GetMixingCoefficient() == 0) {
        continue;
      }

      GlBindRenderTargetTextureToUniform(blit_program_, "source_texture",
                                         activation.render_target());
      static Rectangle rectangle;
      rectangle.Draw();
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

}  // namespace opendrop
