#include "preset/thin_film/thin_film.h"

#include <algorithm>
#include <cmath>

#include "debug/control_injector.h"
#include "preset/thin_film/bubble.fsh.h"
#include "preset/thin_film/passthrough.vsh.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/graphics/colors.h"
#include "util/graphics/gl_util.h"
#include "util/logging/logging.h"
#include "util/math/vector.h"
#include "util/status/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 0.5f;
}

ThinFilm::ThinFilm(std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager) {
  thin_film_program_ =
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), bubble_fsh::Code())
          .value();
}

absl::StatusOr<std::shared_ptr<Preset>> ThinFilm::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  return std::shared_ptr<ThinFilm>(new ThinFilm(texture_manager));
}

void ThinFilm::OnUpdateGeometry() { glViewport(0, 0, width(), height()); }

void ThinFilm::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy();
  float power = state->power();

  // Write a test to assert that rendering is done into the back buffer (zero
  // frame delay to presentation).
  // output_render_target->swap();
  {
    auto output_activation = output_render_target->Activate();
    thin_film_program_->Use();

    if (SIGINJECT_TRIGGER("invert_screen")) {
      invert_screen_ = !invert_screen_;
    }
    if (SIGINJECT_TRIGGER("invert_hue")) {
      invert_hue_ = !invert_hue_;
    }
    if (SIGINJECT_TRIGGER("invert_coords")) {
      invert_coords_ = !invert_coords_;
    }
    if (SIGINJECT_TRIGGER("invert_recursive_coords")) {
      invert_recursive_coords_ = !invert_recursive_coords_;
    }
    if (SIGINJECT_TRIGGER("swap_coords")) {
      swap_coords_ = !swap_coords_;
    }
    if (SIGINJECT_TRIGGER("swap_recursive_coords")) {
      swap_recursive_coords_ = !swap_recursive_coords_;
    }

    gl::GlBindUniform(thin_film_program_, "pole",
                      UnitVectorAtAngle(energy * 2.0f) / 2.0f);

    gl::GlBindRenderTargetTextureToUniform(
        thin_film_program_, "last_frame", output_render_target, {.back = true});
    gl::GlBindUniform(thin_film_program_, "rotate_coeff",
                      rot_filter_->ProcessSample(SIGINJECT_OVERRIDE(
                          "thin_film_rotate_coeff", 0.0f, 0.0f, 1.0f)));
    gl::GlBindUniform(
        thin_film_program_, "phase_x_coeff",
        SIGINJECT_OVERRIDE("thin_film_phase_x_coeff", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(
        thin_film_program_, "phase_y_coeff",
        SIGINJECT_OVERRIDE("thin_film_phase_y_coeff", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(
        thin_film_program_, "ripple_hue",
        SIGINJECT_OVERRIDE("thin_film_ripple_hue", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(
        thin_film_program_, "shift_hue_coeff",
        SIGINJECT_OVERRIDE("thin_film_shift_hue_coeff", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(
        thin_film_program_, "min_value_coeff",
        SIGINJECT_OVERRIDE("thin_film_min_value_coeff", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(
        thin_film_program_, "fisheye_coeff",
        SIGINJECT_OVERRIDE("thin_film_fisheye_coeff", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(thin_film_program_, "folds_coeff",
                      folds_filter_->ProcessSample(SIGINJECT_OVERRIDE(
                          "thin_film_folds_coeff", 0.0f, 0.02f, 1.5f)));
    gl::GlBindUniform(
        thin_film_program_, "force_mix_coeff",
        SIGINJECT_OVERRIDE("thin_film_force_mix_coeff", 0.0f, 0.02f, 1.5f));
    gl::GlBindUniform(
        thin_film_program_, "mix_prescale_coeff",
        SIGINJECT_OVERRIDE("thin_film_mix_prescale_coeff", 0.0f, 0.0f, 1.5f));
    gl::GlBindUniform(thin_film_program_, "invert_screen", invert_screen_);
    gl::GlBindUniform(thin_film_program_, "invert_hue", invert_hue_);
    gl::GlBindUniform(thin_film_program_, "invert_coords", invert_coords_);
    gl::GlBindUniform(thin_film_program_, "invert_recursive_coords",
                      invert_recursive_coords_);
    gl::GlBindUniform(thin_film_program_, "swap_coords", swap_coords_);
    gl::GlBindUniform(thin_film_program_, "swap_recursive_coords",
                      swap_recursive_coords_);

    glViewport(0, 0, width(), height());
    rectangle_.Draw();
    output_render_target->swap();
  }
}

}  // namespace opendrop
