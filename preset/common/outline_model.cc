#include "preset/common/outline_model.h"

#include <cmath>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <glm/mat4x4.hpp>

#include "preset/common/alpaca.obj.h"
#include "preset/common/alpaca_outline.obj.h"
#include "preset/common/cube.obj.h"
#include "preset/common/cube_outline.obj.h"
#include "preset/common/lo_x.obj.h"
#include "preset/common/model.fsh.h"
#include "preset/common/passthrough_vert.vsh.h"
#include "preset/common/pill_center.obj.h"
#include "preset/common/pill_end_bottom.obj.h"
#include "preset/common/pill_end_top.obj.h"
#include "preset/common/pill_shadow.obj.h"
#include "preset/common/star.obj.h"
#include "preset/common/star_outline.obj.h"
#include "util/gl_util.h"
#include "util/status_macros.h"
#include "debug/control_injector.h"

namespace opendrop {

OutlineModel::OutlineModel(std::shared_ptr<gl::GlProgram> model_program)
    : model_program_(model_program),
      pill_end_top_(pill_end_top_obj::Vertices(), pill_end_top_obj::Normals(),
                    pill_end_top_obj::Uvs(), pill_end_top_obj::Triangles()),
      pill_end_bottom_(
          pill_end_bottom_obj::Vertices(), pill_end_bottom_obj::Normals(),
          pill_end_bottom_obj::Uvs(), pill_end_bottom_obj::Triangles()),
      pill_center_(pill_center_obj::Vertices(), pill_center_obj::Normals(),
                   pill_center_obj::Uvs(), pill_center_obj::Triangles()),
      pill_shadow_(pill_shadow_obj::Vertices(), pill_shadow_obj::Normals(),
                   pill_shadow_obj::Uvs(), pill_shadow_obj::Triangles()),
      alpaca_(alpaca_obj::Vertices(), alpaca_obj::Normals(), alpaca_obj::Uvs(),
              alpaca_obj::Triangles()),
      alpaca_outline_(alpaca_outline_obj::Vertices(),
                      alpaca_outline_obj::Normals(), alpaca_outline_obj::Uvs(),
                      alpaca_outline_obj::Triangles()),
      star_(star_obj::Vertices(), star_obj::Normals(), star_obj::Uvs(),
            star_obj::Triangles()),
      star_outline_(star_outline_obj::Vertices(), star_outline_obj::Normals(),
                    star_outline_obj::Uvs(), star_outline_obj::Triangles()),
      cube_(cube_obj::Vertices(), cube_obj::Normals(), cube_obj::Uvs(),
            cube_obj::Triangles()),
      cube_outline_(cube_outline_obj::Vertices(), cube_outline_obj::Normals(),
                    cube_outline_obj::Uvs(), cube_outline_obj::Triangles()),
      lo_x_(lo_x_obj::Vertices(), lo_x_obj::Normals(), lo_x_obj::Uvs(),
            lo_x_obj::Triangles())

{}

absl::StatusOr<std::shared_ptr<OutlineModel>> OutlineModel::MakeShared() {
  ASSIGN_OR_RETURN(auto model_program,
                   gl::GlProgram::MakeShared(passthrough_vert_vsh::Code(),
                                             model_fsh::Code()));
  return std::shared_ptr<OutlineModel>(new OutlineModel(model_program));
}

void OutlineModel::Draw(const Params& params) {
  auto program_activation = model_program_->Activate();

  GlBindUniform(model_program_, "render_target_size",
                glm::ivec2(params.render_target->width(),
                           params.render_target->height()));
  GlBindUniform(model_program_, "alpha", params.alpha);
  GlBindRenderTargetTextureToUniform(model_program_, "render_target",
                                     params.render_target,
                                     gl::GlTextureBindingOptions());

  GlBindUniform(model_program_, "energy", params.energy);
  GlBindUniform(model_program_, "model_transform", params.model_transform);

  model_to_draw_ = SIGINJECT_ENUM("model_to_draw", kCube);  

  switch (model_to_draw_) {
    case kCube:
      GlBindUniform(model_program_, "black", true);
      cube_outline_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "light_color_a",
                    glm::mix(params.color_b, params.color_a, 0.7f));
      GlBindUniform(model_program_, "light_color_b", params.color_a);
      cube_.Draw();
      break;
    case kStar:
      GlBindUniform(model_program_, "black", true);
      star_outline_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "light_color_a",
                    glm::mix(params.color_b, params.color_a, 0.7f));
      GlBindUniform(model_program_, "light_color_b", params.color_a);
      star_.Draw();
      break;
    case kAlpaca:
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      alpaca_outline_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "max_negative_z", false);
      GlBindUniform(model_program_, "light_color_a",
                    glm::mix(params.color_b, params.color_a, 0.7f));
      GlBindUniform(model_program_, "light_color_b", params.color_a);
      alpaca_.Draw();
      break;
    case kPill:
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      pill_shadow_.Draw();
      GlBindUniform(model_program_, "max_negative_z", false);
      pill_center_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "light_color_a",
                    glm::mix(params.color_a, params.color_b, 0.7f));
      GlBindUniform(model_program_, "light_color_b", params.color_b);
      pill_end_top_.Draw();
      GlBindUniform(model_program_, "light_color_a",
                    glm::mix(params.color_b, params.color_a, 0.7f));
      GlBindUniform(model_program_, "light_color_b", params.color_a);
      pill_end_bottom_.Draw();
      break;
    case kLoX:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(50);
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      lo_x_.Draw();
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINTS);
      glPointSize(50);
      lo_x_.Draw();
      GlBindUniform(model_program_, "max_negative_z", false);
      GlBindUniform(model_program_, "black", false);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      GlBindUniform(model_program_, "light_color_a",
                    glm::mix(params.color_b, params.color_a, 0.7f));
      GlBindUniform(model_program_, "light_color_b", params.color_a);
      lo_x_.Draw();
      break;
  }
}

}  // namespace opendrop
