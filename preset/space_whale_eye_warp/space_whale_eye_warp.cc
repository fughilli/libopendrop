#include "preset/space_whale_eye_warp/space_whale_eye_warp.h"

#include <algorithm>
#include <cmath>

#include "debug/control_injector.h"
#include "debug/signal_scope.h"
#include "preset/common/outline_model.h"
#include "preset/space_whale_eye_warp/composite.fsh.h"
#include "preset/space_whale_eye_warp/passthrough_frag.fsh.h"
#include "preset/space_whale_eye_warp/passthrough_vert.vsh.h"
#include "preset/space_whale_eye_warp/warp.fsh.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/enums.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"
#include "util/math/math.h"
#include "util/math/perspective.h"
#include "util/math/vector.h"
#include "util/signal/signals.h"
#include "util/signal/transition_controller.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 2.0f;
}  // namespace

SpaceWhaleEyeWarp::SpaceWhaleEyeWarp(
    std::shared_ptr<gl::GlProgram> warp_program,
    std::shared_ptr<gl::GlProgram> composite_program,
    std::shared_ptr<gl::GlProgram> passthrough_program,
    std::shared_ptr<gl::GlRenderTarget> model_texture_target,
    std::shared_ptr<gl::GlRenderTarget> front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_back_render_target,
    std::shared_ptr<gl::GlRenderTarget> depth_output_target,
    std::shared_ptr<OutlineModel> outline_model,
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      passthrough_program_(passthrough_program),
      model_texture_target_(model_texture_target),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target),
      back_front_render_target_(back_front_render_target),
      back_back_render_target_(back_back_render_target),
      depth_output_target_(depth_output_target),
      outline_model_(outline_model)

{}

absl::StatusOr<std::shared_ptr<Preset>> SpaceWhaleEyeWarp::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(auto warp_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto passthrough_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             passthrough_frag_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto depth_output_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager,
                                                  {.enable_depth = true}));
  ASSIGN_OR_RETURN(auto outline_model, OutlineModel::MakeShared());

  return std::shared_ptr<SpaceWhaleEyeWarp>(new SpaceWhaleEyeWarp(
      warp_program, composite_program, passthrough_program, nullptr,
      front_render_target, back_render_target, back_front_render_target,
      back_back_render_target, depth_output_target, outline_model,
      texture_manager));
}

void SpaceWhaleEyeWarp::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (model_texture_target_ != nullptr) {
    model_texture_target_->UpdateGeometry(longer_dimension(),
                                          longer_dimension());
  }
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(longer_dimension(),
                                         longer_dimension());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(longer_dimension(), longer_dimension());
  }
  if (back_front_render_target_ != nullptr) {
    back_front_render_target_->UpdateGeometry(longer_dimension(),
                                              longer_dimension());
  }
  if (back_back_render_target_ != nullptr) {
    back_back_render_target_->UpdateGeometry(longer_dimension(),
                                             longer_dimension());
  }
  if (depth_output_target_ != nullptr) {
    depth_output_target_->UpdateGeometry(longer_dimension(),
                                         longer_dimension());
  }
}

std::tuple<int, float> CountAndScale(float arg, int max_count) {
  int n_clusters = 1.0f + std::fmod(arg, max_count);
  float cluster_scale = (cos(arg * 2.0f * kPi) + 1.0f) / 2.0f;
  return std::make_tuple(n_clusters, cluster_scale);
}
float EstimateBeatPhase(GlobalState& state) { return 0; }

void SpaceWhaleEyeWarp::DrawEyeball(GlobalState& state, glm::vec3 zoom_vec,
                                    float pupil_size, float scale,
                                    float black_alpha, bool back,
                                    glm::vec3 offset) {
  glm::mat4 model_transform =
      glm::mat4(OrientTowards(zoom_vec)) * TranslationTransform(offset) *
      glm::mat4(OrientTowards(zoom_vec / 2.0f)) * ScaleTransform(scale) *
      RotateAround(Directions::kUp, kPi / 2);
  const glm::vec4 color_a =
      glm::vec4(HsvToRgb(glm::vec3(state.energy(), 1, 1)), 1);
  const glm::vec4 color_b =
      glm::vec4(HsvToRgb(glm::vec3(state.energy() + 0.5, 1, 1)), 1);
  outline_model_->Draw({
      .model_transform = model_transform,
      .color_a = color_a,
      .color_b = color_b,
      .render_target = back_render_target_,
      .alpha = 1,
      .energy = state.energy(),
      .blend_coeff = 0.3f,
      .model_to_draw = OutlineModel::ModelToDraw::kEyeball,
      .pupil_size = pupil_size,
      .black_render_target =
          back ? back_back_render_target_ : back_render_target_,
      .black_alpha = black_alpha,
  });
}

void SpaceWhaleEyeWarp::DrawWhale(GlobalState& state, glm::vec3 zoom_vec,
                                  float scale, float mouth_open) {
  glm::mat4 model_transform = glm::mat4(OrientTowards(zoom_vec)) *
                              ScaleTransform(scale) *
                              RotateAround(Directions::kUp, kPi);
  const glm::vec4 color_a =
      glm::vec4(HsvToRgb(glm::vec3(state.energy(), 1, 1)), 1);
  const glm::vec4 color_b =
      glm::vec4(HsvToRgb(glm::vec3(state.energy() + 0.5, 1, 1)), 1);
  outline_model_->Draw({
      .model_transform = model_transform,
      .color_a = color_a,
      .color_b = color_b,
      .render_target = back_render_target_,
      .alpha = 1,
      .energy = state.energy(),
      .blend_coeff = 0.3f,
      .model_to_draw = OutlineModel::ModelToDraw::kHead,
      .pupil_size = 0.0f,
      .black_render_target = back_back_render_target_,
      .black_alpha = 0.0f,
      .mouth_open = mouth_open,
  });
}

void SpaceWhaleEyeWarp::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  for (int i = 0; i < 3; ++i) {
    beat_estimators_[i].Estimate(state->channel_band(i), state->dt());
  }
  float transition_input =
      SIGINJECT_OVERRIDE("transition_input", 0.0f, -1.0f, 1.0f);
  transition_controller_.Update(transition_input);
  scale_controller_.Update(-transition_input);

  SIGPLOT_ON("lead_in_value", transition_controller_.LeadInValue());
  SIGPLOT_ON("lead_out_value", transition_controller_.LeadOutValue());
  SIGPLOT_ON("transition_count", transition_controller_.TransitionCount());

  float pupil_size =
      0.5f + (1.0f + beat_estimators_[0].triangle_phase()) / 2.0f;

  glm::vec3 zoom_vec =
      glm::vec3(UnitVectorAtAngle(zoom_angle_), 0) + Directions::kIntoScreen;

  zoom_vec = glm::vec3(

      SIGPLOT("zoom_vec_x_filtered",
              zoom_filters_[0]->ProcessSample(SIGPLOT(
                  "zoom_vec_x",
                  SIGINJECT_OVERRIDE("zoom_vec_x", zoom_vec.x, -1.0f, 1.0f)))),
      zoom_filters_[1]->ProcessSample(
          SIGINJECT_OVERRIDE("zoom_vec_y", zoom_vec.y, -1.0f, 1.0f)),
      zoom_filters_[2]->ProcessSample(
          SIGINJECT_OVERRIDE("zoom_vec_z", zoom_vec.z, -1.0f, 1.0f)));

  zoom_vec = glm::normalize(zoom_vec);

  glm::vec3 look_zoom_vec =
      Lerp(zoom_vec, Directions::kIntoScreen,
           std::min(transition_controller_.LeadOutValue() * 3.0f, 1.0f));

  zoom_angle_ += (0.3 + (beat_estimators_[0].triangle_phase() *
                         sin(state->energy() * 10))) /
                 10;

  float eye_scale =
      SIGPLOT("eye_scale",
              (0.4 + SineEase(beat_estimators_[0].triangle_phase()) * 0.2) *
                  transition_controller_.LeadInValue());
  // Return the whale scale to 0.4 by the lead-out value so that we don't get
  // the whale clipping through the pupil plane.
  float whale_scale =
      Lerp(eye_scale, 0.4f,
           std::min(transition_controller_.LeadOutValue() * 3, 1.0f));

  if (scale_controller_.TransitionCount() % 2 == 0) {
    eye_scale *= (1.0f - scale_controller_.LeadOutValue());
    whale_scale *= (1.0f - scale_controller_.LeadOutValue());
  } else {
    if (scale_controller_.LeadOutValue() > 0.1f) {
      eye_scale *= scale_controller_.LeadOutValue();
      whale_scale *= scale_controller_.LeadOutValue();
    } else {
      eye_scale = 0.0f;
      whale_scale = 0.0f;
    }
  }

  SIGPLOT_ON("modified_eye_scale", eye_scale);
  SIGPLOT_ON("modified_whale_scale", whale_scale);

  {
    auto depth_output_activation = depth_output_target_->Activate();
    glViewport(0, 0, longer_dimension(), longer_dimension());
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthRange(0, 10);
    glEnable(GL_DEPTH_TEST);

    if (eye_scale != 0.0f || whale_scale != 0.0f) {
      if (transition_controller_.TransitionCount() % 2 == 0) {
        const float scale_modifier = (2.0f / (num_eyeballs_ + 1));
        pupil_size = Lerp(pupil_size, 20.0f / scale_modifier,
                          transition_controller_.LeadOutValue());
        for (int i = 0; i < num_eyeballs_; ++i) {
          glm::vec2 displacement(0, 0);
          if (num_eyeballs_ > 1) {
            displacement = UnitVectorAtAngle(IndexToAngle(i, num_eyeballs_) +
                                             state->treble_energy() * 100 *
                                                 energy_coefficient_) *
                           Lerp(0.2f, 0.6f,
                                Lerp(beat_estimators_[1].triangle_phase(),
                                     UnitarySin(state->mid_energy() * 100 *
                                                energy_coefficient_),
                                     UnitarySin(state->energy() * 10)));
          }
          DrawEyeball(
              *state, look_zoom_vec, pupil_size, eye_scale * scale_modifier,
              std::min(1.0f, 0.1f + transition_controller_.LeadOutValue()),
              true
              /*transition_controller_.TransitionCount() % 2 == 0*/,
              glm::vec3(displacement, 0));
        }
      } else {
        pupil_size =
            Lerp(pupil_size, 100.0f, transition_controller_.LeadOutValue());
        DrawEyeball(
            *state, look_zoom_vec, pupil_size, eye_scale / 4,
            std::min(1.0f, 0.1f + transition_controller_.LeadOutValue()),
            true /*transition_controller_.TransitionCount() % 2 == 0*/,
            glm::vec3(-0.2, 0.15, -0.25));
        DrawEyeball(
            *state, look_zoom_vec, pupil_size, eye_scale / 4,
            std::min(1.0f, 0.1f + transition_controller_.LeadOutValue()),
            true /*transition_controller_.TransitionCount() % 2 == 0*/,
            glm::vec3(0.2, 0.15, -0.25));
        DrawWhale(*state, look_zoom_vec, whale_scale,
                  std::clamp((1 + sin(state->mid_energy() * 200)) / 2 +
                                 state->mid() * 3,
                             0.0f, 1.0f));
      }
    }

    glDisable(GL_DEPTH_TEST);
  }

  glm::vec4 rainbow_border =
      glm::vec4(HsvToRgb(glm::vec3(background_hue_, 1, 1)), 1);

  glm::vec4 black_and_white_border = glm::vec4(0, 0, 0, 1);
  {
    bool white = std::fmod(background_hue_ * 10.0f, 1.0f) > 0.5f;
    if (white) black_and_white_border = glm::vec4(1, 1, 1, 1);
  }

  glm::vec4 front_border, back_border;
  if (transition_controller_.Transitioned()) {
    front_render_target_->swap_texture_unit(back_front_render_target_.get());
    back_back_render_target_->swap_texture_unit(back_render_target_.get());

    num_eyeballs_ = Coefficients::Random<1, int>(1, 6)[0];
    energy_coefficient_ = Coefficients::Random<1, float>(0.05f, 1.0f)[0];
  }
  if (transition_controller_.TransitionCount() % 2 == 1) {
    front_border = black_and_white_border;
    back_border = rainbow_border;

  } else {
    back_border = black_and_white_border;
    front_border = rainbow_border;
  }

  {
    auto front_activation = front_render_target_->Activate();

    auto program_activation = warp_program_->Activate();

    GlBindUniform(warp_program_, "frame_size", glm::ivec2(width(), height()));
    GlBindUniform(warp_program_, "power", state->power());
    GlBindUniform(warp_program_, "energy", state->energy());
    // Figure out how to keep it from zooming towards the viewer when the line
    // is moving
    GlBindUniform(warp_program_, "zoom_vec", zoom_vec);
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));
    auto binding_options = gl::GlTextureBindingOptions();
    background_hue_ +=
        state->power() *
        SIGINJECT_OVERRIDE("space_whale_eye_warp_border_hue_coeff", 0.1f, 0.0f,
                           3.0f);
    binding_options.border_color = front_border;
    binding_options.sampling_mode = gl::GlTextureSamplingMode::kClampToBorder;
    GlBindRenderTargetTextureToUniform(warp_program_, "last_frame",
                                       back_render_target_, binding_options);
    GlBindRenderTargetTextureToUniform(warp_program_, "input",
                                       depth_output_target_, binding_options);
    GlBindUniform(warp_program_, "input_enable",
                  transition_controller_.TransitionCount() % 2 == 0);

    glViewport(0, 0, longer_dimension(), longer_dimension());
    rectangle_.Draw();
  }
  {
    auto back_front_activation = back_front_render_target_->Activate();

    auto program_activation = warp_program_->Activate();

    GlBindUniform(warp_program_, "frame_size", glm::ivec2(width(), height()));
    GlBindUniform(warp_program_, "power", state->power());
    GlBindUniform(warp_program_, "energy", state->energy());
    // Figure out how to keep it from zooming towards the viewer when the line
    // is moving
    GlBindUniform(warp_program_, "zoom_vec", zoom_vec);
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));
    auto binding_options = gl::GlTextureBindingOptions();
    background_hue_ +=
        state->power() *
        SIGINJECT_OVERRIDE("space_whale_eye_warp_border_hue_coeff", 0.1f, 0.0f,
                           3.0f);
    binding_options.border_color = back_border;
    binding_options.sampling_mode = gl::GlTextureSamplingMode::kClampToBorder;
    GlBindRenderTargetTextureToUniform(
        warp_program_, "last_frame", back_back_render_target_, binding_options);
    GlBindUniform(warp_program_, "input_enable", false);

    glViewport(0, 0, longer_dimension(), longer_dimension());
    rectangle_.Draw();
  }

  {
    auto output_activation = output_render_target->Activate();
    auto program_activation = composite_program_->Activate();

    GlBindUniform(composite_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(composite_program_, "model_transform", glm::mat4(1.0f));
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());
    GlBindRenderTargetTextureToUniform(composite_program_, "input",
                                       depth_output_target_,
                                       gl::GlTextureBindingOptions());
    GlBindUniform(composite_program_, "input_enable",
                  transition_controller_.TransitionCount() % 2 == 1);

    SquareViewport();
    rectangle_.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
    back_back_render_target_->swap_texture_unit(
        back_front_render_target_.get());
  }
}

}  // namespace opendrop
