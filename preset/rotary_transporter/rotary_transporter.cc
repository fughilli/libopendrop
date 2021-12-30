#include "preset/rotary_transporter/rotary_transporter.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/mat4x4.hpp>

#include "preset/rotary_transporter/composite.fsh.h"
#include "preset/rotary_transporter/passthrough.vsh.h"
#include "preset/rotary_transporter/warp.fsh.h"
#include "util/coefficients.h"
#include "util/colors.h"
#include "util/gl_util.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/math/perspective.h"
#include "util/math/vector.h"
#include "util/status_macros.h"

namespace opendrop {

namespace {

constexpr float kFramerateScale = 60.0f / 60.0f;
constexpr float kScaleFactor = 0.5f;

}  // namespace

RotaryTransporter::RotaryTransporter(
    std::shared_ptr<gl::GlProgram> warp_program,
    std::shared_ptr<gl::GlProgram> composite_program,
    std::shared_ptr<gl::GlRenderTarget> front_render_target,
    std::shared_ptr<gl::GlRenderTarget> back_render_target,
    std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target) {
  zoom_angle_ = Coefficients::Random<1>(0.f, M_PI * 2)[0];
  border_color_phase_ = Coefficients::Random<1>(0.f, 1.f)[0];
}

absl::StatusOr<std::shared_ptr<Preset>> RotaryTransporter::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(
      auto warp_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));

  return std::shared_ptr<RotaryTransporter>(new RotaryTransporter(
      warp_program, composite_program, front_render_target, back_render_target,
      texture_manager));
}

void RotaryTransporter::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
}

void RotaryTransporter::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  if (bass_filter_ == nullptr) {
    // TODO: Refactor into constructor. Plumb GlobalState.
    constexpr float kCenterFrequency = 300.0f;
    constexpr float kBandwidth = 50.0f;
    bass_filter_ = IirBandFilter(50.0f / state->sampling_rate(),
                                 40.0f / state->sampling_rate(),
                                 IirBandFilterType::kBandpass);
    vocal_filter_ = IirBandFilter(kCenterFrequency / state->sampling_rate(),
                                  kBandwidth / state->sampling_rate(),
                                  IirBandFilterType::kBandpass);
    left_vocal_filter_ = IirBandFilter(
        kCenterFrequency / state->sampling_rate(),
        kBandwidth / state->sampling_rate(), IirBandFilterType::kBandpass);
    right_vocal_filter_ = IirBandFilter(
        kCenterFrequency / state->sampling_rate(),
        kBandwidth / state->sampling_rate(), IirBandFilterType::kBandpass);
  }

  float energy = state->energy();
  float normalized_energy = state->normalized_energy();
  float power = state->power();
  float average_power = state->average_power();
  float normalized_power = SafeDivide(power, average_power);

  bass_power_ = bass_filter_->ComputePower(state->left_channel());
  bass_energy_ += bass_power_ * state->dt();

  auto buffer_size = samples.size() / 2;
  vertices_.resize(buffer_size);

  float total_scale_factor =
      kScaleFactor *
      (0.7 + 0.3 * cos((energy * 10 + power * 10) * kFramerateScale));

  for (int i = 0; i < buffer_size; ++i) {
    float x_scaled =
        left_vocal_filter_->ProcessSample(samples[i * 2]) * total_scale_factor;
    float y_scaled = right_vocal_filter_->ProcessSample(samples[i * 2 + 1]) *
                     total_scale_factor;

    vertices_[i] = glm::vec2(x_scaled, y_scaled);
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    float zoom_speed = 1.f + normalized_power / 10 * kFramerateScale;

    GlBindUniform(warp_program_, "power", bass_power_);
    GlBindUniform(warp_program_, "energy", bass_energy_);
    GlBindUniform(warp_program_, "zoom_angle", zoom_angle_);
    GlBindUniform(warp_program_, "zoom_speed", zoom_speed);
    GlBindUniform(warp_program_, "framerate_scale", kFramerateScale);
    GlBindUniform(warp_program_, "last_frame_size",
                  glm::ivec2(width(), height()));
    GlBindRenderTargetTextureToUniform(
        warp_program_, "last_frame", front_render_target_,
        {.sampling_mode = gl::GlTextureSamplingMode::kClampToBorder,
         .border_color =
             glm::vec4(HsvToRgb({normalized_energy / 100 * kFramerateScale +
                                     border_color_phase_,
                                 1, normalized_power / 2}),
                       1)});

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();

    // Draw the waveform.
    constexpr int kMaxSymmetry = 5;
    constexpr int kMinSymmetry = 3;

    glm::vec3 look_vec_3d(-UnitVectorAtAngle(zoom_angle_) / 2.0f, zoom_speed);
    glm::mat3x3 rotation = OrientTowards(look_vec_3d);

    int petals =
        static_cast<int>(kMinSymmetry + (kMaxSymmetry - kMinSymmetry) *
                                            ((sin(energy * 10) + 1) / 2));
    float tube_scale =
        0.25f + vocal_filter_->ComputePower(state->left_channel()) * 20;
    for (int i = 0; i < petals; ++i) {
      std::vector<glm::vec2> vertices_rotary(vertices_.size(), glm::vec2(0, 0));
      float angular_displacement = i * (M_PI * 2.0f / petals);
      glm::vec2 displacement_vector = UnitVectorAtAngle(
          angular_displacement + energy * -40 * kFramerateScale);

      for (int j = 0; j < vertices_rotary.size(); ++j) {
        vertices_rotary[j] =
            Rotate2d(vertices_[j],
                     angular_displacement - energy * 60 * kFramerateScale) +
            displacement_vector * tube_scale;
        vertices_rotary[j].x *= aspect_ratio();
        glm::vec3 rotated = (rotation * glm::vec3(vertices_rotary[j], 0));
        vertices_rotary[j] = glm::vec2(rotated.x, rotated.y);
      }
      polyline_.UpdateVertices(vertices_rotary);
      polyline_.UpdateWidth(std::max(3.0f, 10.0f / (petals - 2)) + power * 20);
      polyline_.UpdateColor(HsvToRgb(glm::vec3(
          energy * kFramerateScale + i / static_cast<float>(petals), 1, 1)));
      polyline_.Draw();
    }

    zoom_angle_ +=
        sin(normalized_power * kFramerateScale) * bass_power_ * kFramerateScale;
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();
    GlBindUniform(composite_program_, "render_target_size",
                  glm::ivec2(width(), height()));
    GlBindUniform(composite_program_, "alpha", alpha);
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());

    glViewport(0, 0, width(), height());
    rectangle_.Draw();

    back_render_target_->swap_texture_unit(front_render_target_.get());
  }
}

}  // namespace opendrop
