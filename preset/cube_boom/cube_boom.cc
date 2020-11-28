#include "libopendrop/preset/cube_boom/cube_boom.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "libopendrop/preset/cube_boom/composite.fsh.h"
#include "libopendrop/preset/cube_boom/cube.obj.h"
#include "libopendrop/preset/cube_boom/model.fsh.h"
#include "libopendrop/preset/cube_boom/monkey.obj.h"
#include "libopendrop/preset/cube_boom/passthrough.vsh.h"
#include "libopendrop/preset/cube_boom/shrek.obj.h"
#include "libopendrop/preset/cube_boom/warp.fsh.h"
#include "libopendrop/util/colors.h"
#include "libopendrop/util/gl_util.h"
#include "libopendrop/util/logging.h"
#include "libopendrop/util/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 2.0f;
}

CubeBoom::CubeBoom(std::shared_ptr<gl::GlProgram> warp_program,
                   std::shared_ptr<gl::GlProgram> composite_program,
                   std::shared_ptr<gl::GlProgram> model_program,
                   std::shared_ptr<gl::GlRenderTarget> front_render_target,
                   std::shared_ptr<gl::GlRenderTarget> back_render_target,
                   std::shared_ptr<gl::GlRenderTarget> depth_output_target,
                   std::shared_ptr<gl::GlTextureManager> texture_manager)
    : Preset(texture_manager),
      warp_program_(warp_program),
      composite_program_(composite_program),
      model_program_(model_program),
      front_render_target_(front_render_target),
      back_render_target_(back_render_target),
      depth_output_target_(depth_output_target),
      monkey_(monkey_obj::Vertices(), monkey_obj::Uvs(),
              monkey_obj::Triangles()),
      shrek_(shrek_obj::Vertices(), shrek_obj::Uvs(), shrek_obj::Triangles()),
      cube_(cube_obj::Vertices(), cube_obj::Uvs(), cube_obj::Triangles()) {}

absl::StatusOr<std::shared_ptr<Preset>> CubeBoom::MakeShared(
    std::shared_ptr<gl::GlTextureManager> texture_manager) {
  ASSIGN_OR_RETURN(
      auto warp_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), warp_fsh::Code()));
  ASSIGN_OR_RETURN(auto composite_program,
                   gl::GlProgram::MakeShared(passthrough_vsh::Code(),
                                             composite_fsh::Code()));
  ASSIGN_OR_RETURN(
      auto model_program,
      gl::GlProgram::MakeShared(passthrough_vsh::Code(), model_fsh::Code()));
  ASSIGN_OR_RETURN(auto front_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto back_render_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager));
  ASSIGN_OR_RETURN(auto depth_output_target,
                   gl::GlRenderTarget::MakeShared(0, 0, texture_manager,
                                                  {.enable_depth = true}));

  return std::shared_ptr<CubeBoom>(new CubeBoom(
      warp_program, composite_program, model_program, front_render_target,
      back_render_target, depth_output_target, texture_manager));
}

void CubeBoom::OnUpdateGeometry() {
  glViewport(0, 0, width(), height());
  if (front_render_target_ != nullptr) {
    front_render_target_->UpdateGeometry(width(), height());
  }
  if (back_render_target_ != nullptr) {
    back_render_target_->UpdateGeometry(width(), height());
  }
  if (depth_output_target_ != nullptr) {
    depth_output_target_->UpdateGeometry(width(), height());
  }
}

void CubeBoom::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->energy();
  float power = state->power();

  auto buffer_size = samples.size() / 2;
  vertices_.resize(buffer_size);

  float cos_energy = cos(energy * 10 + power * 10);
  float sin_energy = sin(energy * 10 + power * 10);
  float total_scale_factor = kScaleFactor * (0.7 + 0.3 * cos_energy);

  for (int i = 0; i < buffer_size; ++i) {
    float x_scaled = samples[i * 2] * total_scale_factor;
    float y_scaled = samples[i * 2 + 1] * total_scale_factor;

    float x_pos = x_scaled * cos_energy - y_scaled * sin_energy;
    float y_pos = x_scaled * sin_energy + y_scaled * cos_energy;

    vertices_[i] = glm::vec2(x_pos, y_pos);
  }

  {
    auto back_activation = back_render_target_->Activate();

    warp_program_->Use();

    LOG(DEBUG) << "Using program";
    int texture_size_location = glGetUniformLocation(
        warp_program_->program_handle(), "last_frame_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    int power_location =
        glGetUniformLocation(warp_program_->program_handle(), "power");
    int energy_location =
        glGetUniformLocation(warp_program_->program_handle(), "energy");
    LOG(DEBUG) << "Got locations for power: " << power_location
               << " and energy: " << energy_location;
    glUniform1f(power_location, power);
    glUniform1f(energy_location, energy);
    LOG(DEBUG) << "Power: " << power << " energy: " << energy;
    glUniform2i(texture_size_location, width(), height());
    GlBindRenderTargetTextureToUniform(warp_program_, "last_frame",
                                       front_render_target_,
                                       gl::GlTextureBindingOptions());

    glm::mat4 model_transform(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(warp_program_->program_handle(),
                                            "model_transform"),
                       1, GL_FALSE, &model_transform[0][0]);

    // Force all fragments to draw with a full-screen rectangle.
    rectangle_.Draw();

    // Draw the waveform.
    polyline_.UpdateVertices(vertices_);
    polyline_.UpdateWidth(2 + power * 10);
    polyline_.UpdateColor(HsvToRgb(glm::vec3(energy, 1, 0.5)));
    polyline_.Draw();

    glFlush();
  }

  {
    auto output_activation = depth_output_target_->Activate();
    model_program_->Use();
    LOG(DEBUG) << "Using program";
    int texture_size_location = glGetUniformLocation(
        model_program_->program_handle(), "render_target_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    glUniform2i(texture_size_location, width(), height());
    GlBindRenderTargetTextureToUniform(model_program_, "render_target",
                                       back_render_target_,
                                       gl::GlTextureBindingOptions());
    glUniform1f(glGetUniformLocation(model_program_->program_handle(), "alpha"),
                alpha);

    glViewport(0, 0, width(), height());

    float cube_scale = std::clamp(0.2 + power / 2, 0.0, 0.5);

    glm::mat4 model_transform;
    model_transform =
        glm::mat4x4(cube_scale, 0, 0, 0,  // Row 1
                    0, cube_scale, 0, 0,  // Row 2
                    0, 0, cube_scale, 0,  // Row 3
                    0, 0, -0.5, 1         // Row 4
                    ) *
        glm::rotate(glm::mat4(1.0f), energy * 10, glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::rotate(glm::mat4(1.0f), energy * 7, glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), energy * 15, glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(model_program_->program_handle(),
                                            "model_transform"),
                       1, GL_FALSE, &model_transform[0][0]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    // Enable one of these at a time.
    shrek_.Draw();
    // cube_.Draw();
    // monkey_.Draw();
    glDisable(GL_DEPTH_TEST);

    back_render_target_->swap_texture_unit(front_render_target_.get());
    glFlush();
  }

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();
    LOG(DEBUG) << "Using program";
    int texture_size_location = glGetUniformLocation(
        composite_program_->program_handle(), "render_target_size");
    LOG(DEBUG) << "Got texture size location: " << texture_size_location;
    glUniform2i(texture_size_location, width(), height());
    GlBindRenderTargetTextureToUniform(composite_program_, "render_target",
                                       depth_output_target_,
                                       gl::GlTextureBindingOptions());
    glUniform1f(
        glGetUniformLocation(composite_program_->program_handle(), "alpha"),
        alpha);

    glViewport(0, 0, width(), height());

    glm::mat4 model_transform(1.0f);
    glUniformMatrix4fv(
        glGetUniformLocation(composite_program_->program_handle(),
                             "model_transform"),
        1, GL_FALSE, &model_transform[0][0]);
    rectangle_.Draw();
    glFlush();
  }
}

}  // namespace opendrop
