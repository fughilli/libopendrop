#include "preset/dcolor/dcolor.h"

#define _USE_MATH_DEFINES
#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <algorithm>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>

#include "preset/dcolor/composite.fsh.h"
#include "preset/dcolor/cube.obj.h"
#include "preset/dcolor/model.fsh.h"
#include "preset/dcolor/monkey.obj.h"
#include "preset/dcolor/passthrough.vsh.h"
#include "preset/dcolor/shrek.obj.h"
#include "preset/dcolor/warp.fsh.h"
#include "util/colors.h"
#include "util/gl_util.h"
#include "util/logging.h"
#include "util/status_macros.h"

namespace opendrop {

namespace {
constexpr float kScaleFactor = 2.0f;
}

Dcolor::Dcolor(std::shared_ptr<gl::GlProgram> warp_program,
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
      cube_(cube_obj::Vertices(), cube_obj::Normals(), cube_obj::Uvs(),
            cube_obj::Triangles()),
      monkey_(monkey_obj::Vertices(), monkey_obj::Uvs(),
              monkey_obj::Triangles()),
      shrek_(shrek_obj::Vertices(), shrek_obj::Uvs(), shrek_obj::Triangles()) {}

absl::StatusOr<std::shared_ptr<Preset>> Dcolor::MakeShared(
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

  return std::shared_ptr<Dcolor>(new Dcolor(
      warp_program, composite_program, model_program, front_render_target,
      back_render_target, depth_output_target, texture_manager));
}

void Dcolor::OnUpdateGeometry() {
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

void Dcolor::OnDrawFrame(
    absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
    float alpha, std::shared_ptr<gl::GlRenderTarget> output_render_target) {
  float energy = state->user_x();
  float power = 0;//state->power();

  glm::vec2 pole_loc  = glm::vec2(cos(energy), sin(energy));
  glm::vec2 zero_loc  = glm::vec2(cos(energy * 2), sin(energy * 2));
  glm::vec2 pole_loc2 = glm::vec2(cos(energy * 3), sin(energy)) * (1.0f + power);
  glm::vec2 zero_loc2 = glm::vec2(cos(energy - 5), sin(energy * 4) * (1.0f - power));
  glm::vec2 pole_loc3 = glm::vec2(cos(energy * -3), sin(energy * 10)) * (0.5f + power * 3.0f);
  glm::vec2 zero_loc3 = glm::vec2(cos(energy * -2), sin(energy * 5)) * sin(energy * 20);
  glm::vec2 pole_loc4 = glm::vec2(cos(energy), sin(energy)) * cos(energy * 5);
  glm::vec2 zero_loc4 = glm::vec2(cos(energy * 2), sin(energy * 2)) * power;

  {
    auto output_activation = output_render_target->Activate();
    composite_program_->Use();

    GlBindUniform(composite_program_, "aspect_ratio",
                  static_cast<float>(height()) / width());
    GlBindUniform(composite_program_, "alpha", alpha);
    GlBindUniform(composite_program_, "model_transform", glm::mat4x4(1.0f));
    GlBindUniform(composite_program_, "zero_loc", zero_loc);
    GlBindUniform(composite_program_, "pole_loc", pole_loc);
    GlBindUniform(composite_program_, "zero_loc2", zero_loc2);
    GlBindUniform(composite_program_, "pole_loc2", pole_loc2);
    GlBindUniform(composite_program_, "zero_loc3", zero_loc3);
    GlBindUniform(composite_program_, "pole_loc3", pole_loc3);
    GlBindUniform(composite_program_, "zero_loc4", zero_loc4);
    GlBindUniform(composite_program_, "pole_loc4", pole_loc4);

    glViewport(0, 0, width(), height());
    rectangle_.Draw();
  }
}

}  // namespace opendrop
