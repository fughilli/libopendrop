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

  {
    auto output_activation = output_render_target->Activate();
    thin_film_program_->Use();

    gl::GlBindUniform(thin_film_program_, "pole",
                      UnitVectorAtAngle(energy * 2.0f) / 2.0f);

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
        thin_film_program_, "min_value_coeff",
        SIGINJECT_OVERRIDE("thin_film_min_value_coeff", 0.0f, 0.0f, 1.0f));
    gl::GlBindUniform(
        thin_film_program_, "fisheye_coeff",
        SIGINJECT_OVERRIDE("thin_film_fisheye_coeff", 0.0f, 0.0f, 1.0f));

    glViewport(0, 0, width(), height());
    rectangle_.Draw();
  }
}

}  // namespace opendrop
