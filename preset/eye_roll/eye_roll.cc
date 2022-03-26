#include "preset/eye_roll/eye_roll.h"

#include <algorithm>
#include <cmath>

#include "absl/strings/str_format.h"
#include "preset/eye_roll/composite.fsh.h"
#include "preset/eye_roll/line.fsh.h"
#include "preset/eye_roll/ngon.fsh.h"
#include "preset/eye_roll/passthrough.vsh.h"
#include "preset/eye_roll/warp.fsh.h"
#include "util/math/coefficients.h"
#include "util/graphics/colors.h"
#include "third_party/gl_helper.h"
#include "util/graphics/gl_util.h"
#include "third_party/glm_helper.h"
#include "util/logging/logging.h"
#include "util/math/math.h"
#include "util/status/status_macros.h"

namespace opendrop {
namespace {
enum FlameInputShape {
  kLine,
  kPolygon,
};
constexpr FlameInputShape kFlameInputShape = kPolygon;

constexpr int kCircleSegments = 32;

constexpr bool kEnableWinkDebugging = false;
}  // namespace

EyeRoll::EyeRoll(std::shared_ptr<gl::GlProgram> warp_program,
                 std::shared_ptr<gl::GlProgram> composite_program,
                 std::shared_ptr<gl::GlProgram> ngon_program,
                 std::shared_ptr<gl::GlProgram> line_program,
                 std::shared_ptr<gl::GlRenderTarget> front_render_target,
                 std::shared_ptr<gl::GlRenderTarget> back_render_target,
                 std::shared_ptr<gl::GlTextureManager> texture_manager, int n)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      ngon_program_(ngon_program),
      line_program_(line_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target),
      ngon_(n),
      polyline_({1, 1, 1}, {}, 3),
      blink_event_{0.5, 15.0},
      wink_event_{5.0, 45.0},
      left_eye_tweener_{
          {.ramp_on_length = 0.5, .on_length = 0.1, .ramp_off_length = 0.3}},
      right_eye_tweener_{
          {.ramp_on_length = 0.5, .on_length = 0.1, .ramp_off_length = 0.3}} {}

absl::StatusOr<std::shared_ptr<Preset>> EyeRoll::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(
      auto warp_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto ngon_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), ngon_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto line_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), line_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));

  return std::shared_ptr<EyeRoll>(
      new EyeRoll(warp_program, composite_program, ngon_program, line_program,
                  front_render_target, back_render_target, texture_manager,
                  kCircleSegments));
}

void EyeRoll::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void EyeRoll::DrawEye(glm::vec2 center, float scale, float scale_coeff,
                      float angle, float eyelid_pos) {
  const float ngon_scale = MapValue<float>(scale_coeff, 0, 1, 0.4, 1) * scale;
  glm::mat4 transform = glm::mat4x4(ngon_scale, 0, 0, 0,      // Row 1
                                    0, ngon_scale, 0, 0,      // Row 2
                                    0, 0, ngon_scale, 0,      // Row 3
                                    center.x, center.y, 0, 1  // Row 4
  );

  const float y_cutoff =
      MapValue<float>(eyelid_pos, 0, 1, center.y + scale, center.y - scale);

  GlBindUniform(ngon_program_, "model_transform", transform);
  GlBindUniform(ngon_program_, "energy", angle);
  GlBindUniform(ngon_program_, "center", center);
  GlBindUniform(ngon_program_, "y_cutoff", y_cutoff);

  ngon_.Draw();
}

void EyeRoll::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  if (bass_filter_ == nullptr) {
    // TODO: Refactor into constructor. Plumb GlobalState.
    bass_filter_ = IirBandFilter(30.0f / state->sampling_rate(),
                                 20.0f / state->sampling_rate(),
                                 IirBandFilterType::kBandpass);
    bass_power_filter_ = std::make_shared<HystereticMapFilter>(
        IirSinglePoleFilter(1.0f / state->sampling_rate(),
                            IirSinglePoleFilterType::kLowpass),
        0.999f);
    // TODO: Refactor into constructor. Plumb GlobalState.
    treble_filter_ = IirBandFilter(600.0f / state->sampling_rate(),
                                   100.0f / state->sampling_rate(),
                                   IirBandFilterType::kBandpass);
    treble_power_filter_ = std::make_shared<HystereticMapFilter>(
        IirSinglePoleFilter(1.0f / state->sampling_rate(),
                            IirSinglePoleFilterType::kLowpass),
        0.999f);
  }
  float energy = state->energy();
  float power = state->power();

  line_energy_ += sin(energy * 5) * sin(energy * 17) * 10 * state->dt();

  const float bass_power = bass_filter_->ComputePower(state->left_channel());
  const float mapped_bass_power = bass_power_filter_->ProcessSample(bass_power);
  const float treble_power =
      treble_filter_->ComputePower(state->left_channel());
  const float mapped_treble_power =
      treble_power_filter_->ProcessSample(treble_power);

  rotary_velocity_l_ += mapped_treble_power * 50 * state->dt() *
                        sin(energy * 3.15) * sin(energy * 8.75);
  rotary_velocity_l_ *= 0.9;
  rotary_velocity_r_ += mapped_treble_power * 50 * state->dt() *
                        sin(energy * 3.51) * sin(energy * 8.57);
  rotary_velocity_r_ *= 0.9;

  eye_angle_l_ += rotary_velocity_l_ * state->dt();
  eye_angle_r_ += rotary_velocity_r_ * state->dt();

  line_points_.resize(state->left_channel().size());
  for (int i = 0; i < state->left_channel().size(); ++i) {
    line_points_[i] = {
        MapValue<float>(i, 0, state->left_channel().size() - 1, -1, 1),
        state->left_channel()[i] / 10};
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    GlBindUniform(warp_program_, "power", power);
    GlBindUniform(warp_program_, "energy", line_energy_);
    GlBindUniform(warp_program_, "last_frame_size",
                  glm::ivec2(width(), height()));
    GlBindRenderTargetTextureToUniform(warp_program_, "last_frame",
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());
    GlBindUniform(warp_program_, "model_transform", glm::mat4(1.0f));

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();

    line_program_->Use();
    GlBindUniform(line_program_, "power", power);
    GlBindUniform(line_program_, "energy", energy);

    switch (kFlameInputShape) {
      case kLine:
        GlBindUniform(line_program_, "model_transform", glm::mat4(1.0f));
        polyline_.UpdateVertices(line_points_);
        polyline_.Draw();
        break;
      case kPolygon: {
        const float dot_scale =
            MapValue<float>(mapped_bass_power, 0, 1, 0.1, 0.2);
        const float dot_translation =
            sin(energy * 5.77) * cos(energy * 21.13) * 0.2 +
            power / 10 * cos(energy * 10);
        GlBindUniform(line_program_, "model_transform",
                      glm::mat4x4(dot_scale, 0, 0, 0,       // Row 1
                                  0, dot_scale, 0, 0,       // Row 2
                                  0, 0, dot_scale, 0,       // Row 3
                                  dot_translation, 0, 0, 1  // Row 4
                                  ));
        ngon_.Draw();
        break;
      }
    }
  }

  {
    auto output_activation = output_render_target->Activate();

    composite_program_->Use();

    GlBindUniform(composite_program_, "power", power);
    GlBindUniform(composite_program_, "energy", energy);
    GlBindUniform(composite_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(composite_program_, "alpha", alpha);
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());
    GlBindUniform(composite_program_, "model_transform", glm::mat4(1.0f));

    glViewport(0, 0, width(), height());
    rectangle_.Draw();

    ngon_program_->Use();
    glViewport(0, 0, width(), height());
    DrawEye({-0.4583, -0.5936}, 0.4, mapped_bass_power, eye_angle_l_,
            SineEase(left_eye_tweener_.Value(state->t())));
    DrawEye({0.4583, -0.5936}, 0.4, mapped_bass_power, eye_angle_r_,
            SineEase(right_eye_tweener_.Value(state->t())));
  }

  back_render_target_->swap_texture_unit(front_render_target_.get());

  if (blink_event_.IsDue(state->t())) {
    left_eye_tweener_.Start(state->t());
    right_eye_tweener_.Start(state->t());
  }
  if (wink_event_.IsDue(state->t())) {
    if (Coefficients::Random<1>(-1, 1)[0] < 0) {
      left_eye_tweener_.Start(state->t());
    } else {
      right_eye_tweener_.Start(state->t());
    }
  }
  if constexpr (kEnableWinkDebugging) {
    LOG(INFO) << absl::StrFormat(
        "Blink: %1.3f, Wink: %1.3f (L: %1.3f, R: %1.3f)",
        blink_event_.oneshot().FractionDue(state->t()),
        wink_event_.oneshot().FractionDue(state->t()),
        left_eye_tweener_.Value(state->t()),
        right_eye_tweener_.Value(state->t()));
  }
}

}  // namespace opendrop
