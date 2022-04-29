#ifndef PRESET_COMMON_OUTLINE_MODEL_H_
#define PRESET_COMMON_OUTLINE_MODEL_H_

#include <memory>

#include "primitive/model.h"
#include "third_party/glm_helper.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"

namespace opendrop {

class OutlineModel {
 public:
  enum ModelToDraw {
    kPill = 0,
    kAlpaca = 1,
    kStar = 2,
    kCube = 3,
    kLoX = 4,
    kEyeball = 5,
    kHead = 6,
    kDenseLastValue = kHead,
  };

  struct Params {
    glm::mat4 model_transform;
    glm::vec4 color_a;
    glm::vec4 color_b;
    std::shared_ptr<gl::GlRenderTarget> render_target;
    float alpha;
    float energy;
    float blend_coeff;
    ModelToDraw model_to_draw;

    float pupil_size = 1.0f;

    std::shared_ptr<gl::GlRenderTarget> black_render_target;
    float black_alpha;

    float mouth_open = 0.0f;
  };

  static absl::StatusOr<std::shared_ptr<OutlineModel>> MakeShared();

  void Draw(const Params& params);

 protected:
  OutlineModel(std::shared_ptr<gl::GlProgram> model_program);

 private:
  std::shared_ptr<gl::GlProgram> model_program_;

  Model pill_end_top_;
  Model pill_end_bottom_;
  Model pill_center_;
  Model pill_shadow_;
  Model alpaca_;
  Model alpaca_outline_;
  Model star_;
  Model star_outline_;
  Model cube_;
  Model cube_outline_;
  Model lo_x_;
  Model eyeball_pupil_;
  Model eyeball_iris_;
  Model eyeball_ball_;
  Model head_outer_;
  Model head_inner_;
  Model jaw_outer_;
  Model jaw_inner_;
};

}  // namespace opendrop

#endif  // PRESET_COMMON_OUTLINE_MODEL_H_
