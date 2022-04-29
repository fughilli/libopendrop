#include "preset/common/outline_model.h"

#include <cmath>

#include "debug/control_injector.h"
#include "preset/common/alpaca.obj.h"
#include "preset/common/alpaca_outline.obj.h"
#include "preset/common/cube.obj.h"
#include "preset/common/cube_outline.obj.h"
#include "preset/common/eyeball_ball.obj.h"
#include "preset/common/eyeball_iris.obj.h"
#include "preset/common/eyeball_pupil.obj.h"
#include "preset/common/head_inner.obj.h"
#include "preset/common/head_outer.obj.h"
#include "preset/common/jaw_inner.obj.h"
#include "preset/common/jaw_outer.obj.h"
#include "preset/common/lo_x.obj.h"
#include "preset/common/model.fsh.h"
#include "preset/common/passthrough_vert.vsh.h"
#include "preset/common/pill_center.obj.h"
#include "preset/common/pill_end_bottom.obj.h"
#include "preset/common/pill_end_top.obj.h"
#include "preset/common/pill_shadow.obj.h"
#include "preset/common/star.obj.h"
#include "preset/common/star_outline.obj.h"
#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"
#include "util/graphics/gl_util.h"
#include "util/math/vector.h"
#include "util/status/status_macros.h"

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
            lo_x_obj::Triangles()),
      eyeball_pupil_(eyeball_pupil_obj::Vertices(),
                     eyeball_pupil_obj::Normals(), eyeball_pupil_obj::Uvs(),
                     eyeball_pupil_obj::Triangles()),
      eyeball_iris_(eyeball_iris_obj::Vertices(), eyeball_iris_obj::Normals(),
                    eyeball_iris_obj::Uvs(), eyeball_iris_obj::Triangles()),
      eyeball_ball_(eyeball_ball_obj::Vertices(), eyeball_ball_obj::Normals(),
                    eyeball_ball_obj::Uvs(), eyeball_ball_obj::Triangles()),
      head_outer_(head_outer_obj::Vertices(), head_outer_obj::Normals(),
                  head_outer_obj::Uvs(), head_outer_obj::Triangles()),
      head_inner_(head_inner_obj::Vertices(), head_inner_obj::Normals(),
                  head_inner_obj::Uvs(), head_inner_obj::Triangles()),
      jaw_outer_(jaw_outer_obj::Vertices(), jaw_outer_obj::Normals(),
                 jaw_outer_obj::Uvs(), jaw_outer_obj::Triangles()),
      jaw_inner_(jaw_inner_obj::Vertices(), jaw_inner_obj::Normals(),
                 jaw_inner_obj::Uvs(), jaw_inner_obj::Triangles())

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
  GlBindRenderTargetTextureToUniform(model_program_, "black_render_target",
                                     params.black_render_target,
                                     gl::GlTextureBindingOptions());

  GlBindUniform(model_program_, "energy", params.energy);
  GlBindUniform(model_program_, "blend_coeff", params.blend_coeff);
  GlBindUniform(model_program_, "model_transform", params.model_transform);
  GlBindUniform(model_program_, "black_alpha", 0.0f);

  switch (params.model_to_draw) {
    case kCube:
      GlBindUniform(model_program_, "black", true);
      cube_outline_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "light_color_a", params.color_a);
      GlBindUniform(model_program_, "light_color_b", params.color_b);
      cube_.Draw();
      break;
    case kStar:
      GlBindUniform(model_program_, "black", true);
      star_outline_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "light_color_a", params.color_a);
      GlBindUniform(model_program_, "light_color_b", params.color_b);
      star_.Draw();
      break;
    case kAlpaca:
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      alpaca_outline_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "max_negative_z", false);
      GlBindUniform(model_program_, "light_color_a", params.color_a);
      GlBindUniform(model_program_, "light_color_b", params.color_b);
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
      GlBindUniform(model_program_, "light_color_a", params.color_a);
      GlBindUniform(model_program_, "light_color_b", params.color_b);
      lo_x_.Draw();
      break;
    case kEyeball:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(50);
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      // GlBindUniform(
      //     model_program_, "model_transform",
      //     params.model_transform *
      //         ScaleTransform(1, params.pupil_size, params.pupil_size));
      // eyeball_pupil_.Draw();
      // GlBindUniform(model_program_, "model_transform",
      // params.model_transform);
      eyeball_iris_.Draw();
      eyeball_ball_.Draw();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      GlBindUniform(model_program_, "max_negative_z", false);
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(
          model_program_, "model_transform",
          params.model_transform *
              ScaleTransform(1, params.pupil_size, params.pupil_size));
      GlBindUniform(model_program_, "black_alpha", params.black_alpha);
      eyeball_pupil_.Draw();
      GlBindUniform(model_program_, "black_alpha", 0.0f);
      GlBindUniform(model_program_, "model_transform", params.model_transform);
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(model_program_, "light_color_a", params.color_a);
      GlBindUniform(model_program_, "light_color_b", params.color_b);
      eyeball_iris_.Draw();
      GlBindUniform(
          model_program_, "light_color_a",
          glm::mix(params.color_a, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.7f));
      GlBindUniform(
          model_program_, "light_color_b",
          glm::mix(params.color_b, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), 0.7f));
      eyeball_ball_.Draw();
      break;
    case kHead:
      GlBindUniform(model_program_, "blend_coeff", 0.0f);

      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(50);
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      head_outer_.Draw();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      GlBindUniform(model_program_, "max_negative_z", false);
      head_inner_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(
          model_program_, "light_color_a",
          glm::mix(params.color_a, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.7f));
      GlBindUniform(
          model_program_, "light_color_b",
          glm::mix(params.color_b, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.7f));
      head_outer_.Draw();

      GlBindUniform(
          model_program_, "model_transform",
          params.model_transform *
              RotateAround(glm::vec3(1, 0, 0), kPi / 2 * params.mouth_open));
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      glLineWidth(50);
      GlBindUniform(model_program_, "black", true);
      GlBindUniform(model_program_, "max_negative_z", true);
      jaw_outer_.Draw();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      GlBindUniform(model_program_, "max_negative_z", false);
      jaw_inner_.Draw();
      GlBindUniform(model_program_, "black", false);
      GlBindUniform(
          model_program_, "light_color_a",
          glm::mix(params.color_a, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.7f));
      GlBindUniform(
          model_program_, "light_color_b",
          glm::mix(params.color_b, glm::vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.7f));
      jaw_outer_.Draw();
      break;
  }
}

}  // namespace opendrop
