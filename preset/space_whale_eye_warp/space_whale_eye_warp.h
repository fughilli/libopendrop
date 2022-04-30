#ifndef PRESET_SPACE_WHALE_EYE_WARP_SPACE_WHALE_EYE_WARP_H_
#define PRESET_SPACE_WHALE_EYE_WARP_SPACE_WHALE_EYE_WARP_H_

#include <vector>

#include "absl/status/statusor.h"
#include "preset/common/outline_model.h"
#include "preset/preset.h"
#include "primitive/polyline.h"
#include "primitive/rectangle.h"
#include "third_party/glm_helper.h"
#include "util/audio/beat_estimator.h"
#include "util/graphics/gl_interface.h"
#include "util/graphics/gl_render_target.h"
#include "util/graphics/gl_texture_manager.h"
#include "util/signal/filter.h"
#include "util/signal/transition_controller.h"

namespace opendrop {

class SpaceWhaleEyeWarp : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "SpaceWhaleEyeWarp"; }

  int max_count() const override { return 1; }

 protected:
  SpaceWhaleEyeWarp(
      std::shared_ptr<gl::GlProgram> warp_program,
      std::shared_ptr<gl::GlProgram> composite_program,
      std::shared_ptr<gl::GlProgram> passthrough_program,
      std::shared_ptr<gl::GlRenderTarget> model_texture_target,
      std::shared_ptr<gl::GlRenderTarget> back_front_render_target,
      std::shared_ptr<gl::GlRenderTarget> back_back_render_target,
      std::shared_ptr<gl::GlRenderTarget> front_render_target,
      std::shared_ptr<gl::GlRenderTarget> back_render_target,
      std::shared_ptr<gl::GlRenderTarget> depth_output_target,
      std::shared_ptr<OutlineModel> outline_model,
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  void OnDrawFrame(
      absl::Span<const float> samples, std::shared_ptr<GlobalState> state,
      float alpha,
      std::shared_ptr<gl::GlRenderTarget> output_render_target) override;
  void OnUpdateGeometry() override;

 private:
  constexpr static float kCutoff = 0.1f;

  void DrawEyeball(GlobalState& state, glm::vec3 zoom_vec, float pupil_size,
                   float eye_scale, float black_alpha, bool back,
                   glm::vec3 offset);
  void DrawWhale(GlobalState& state, glm::vec3 zoom_vec, float scale, float mouth_open);

  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlProgram> passthrough_program_;
  std::shared_ptr<gl::GlRenderTarget> model_texture_target_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_back_render_target_;
  std::shared_ptr<gl::GlRenderTarget> depth_output_target_;
  std::shared_ptr<OutlineModel> outline_model_;

  std::vector<glm::vec2> vertices_;
  Rectangle rectangle_;
  Polyline polyline_;

  float position_accum_ = 0.0f;
  float background_hue_ = 0;
  float rot_arg_ = 0.0f;

  float zoom_angle_ = 0.0f;

  bool texture_trigger_ = false;

  int num_eyeballs_ = 3;

  BeatEstimator beat_estimators_[3] = {{0.99f}, {0.99f}, {0.99f}};

  TransitionController transition_controller_{
      TransitionController::Options{.decay_rate = 0.05f,
                                    .input_decay_zone = 0.2f,
                                    .threshold = 0.6f,
                                    .closeness_threshold = 0.01f}};

  std::shared_ptr<IirFilter> zoom_filters_[3] = {
      IirSinglePoleFilter(kCutoff, IirSinglePoleFilterType::kLowpass),
      IirSinglePoleFilter(kCutoff, IirSinglePoleFilterType::kLowpass),
      IirSinglePoleFilter(kCutoff, IirSinglePoleFilterType::kLowpass)};
};

}  // namespace opendrop

#endif  // PRESET_SPACE_WHALE_EYE_WARP_SPACE_WHALE_EYE_WARP_H_
