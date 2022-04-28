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

namespace opendrop {

class SpaceWhaleEyeWarp : public Preset {
 public:
  static absl::StatusOr<std::shared_ptr<Preset>> MakeShared(
      std::shared_ptr<gl::GlTextureManager> texture_manager);

  std::string name() const override { return "SpaceWhaleEyeWarp"; }

  int max_count() const override { return 1; }

 protected:
  SpaceWhaleEyeWarp(std::shared_ptr<gl::GlProgram> warp_program,
                    std::shared_ptr<gl::GlProgram> composite_program,
                    std::shared_ptr<gl::GlProgram> passthrough_program,
                    std::shared_ptr<gl::GlRenderTarget> model_texture_target,
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
  void DrawCubes(float scale, float power, float bass, float energy, float dt, float time,
                 float zoom_coeff, glm::vec3 zoom_vec, int num_cubes);

  std::shared_ptr<gl::GlProgram> warp_program_;
  std::shared_ptr<gl::GlProgram> composite_program_;
  std::shared_ptr<gl::GlProgram> passthrough_program_;
  std::shared_ptr<gl::GlRenderTarget> model_texture_target_;
  std::shared_ptr<gl::GlRenderTarget> front_render_target_;
  std::shared_ptr<gl::GlRenderTarget> back_render_target_;
  std::shared_ptr<gl::GlRenderTarget> depth_output_target_;
  std::shared_ptr<OutlineModel> outline_model_;

  std::vector<glm::vec2> vertices_;
  Rectangle rectangle_;
  Polyline polyline_;

  float position_accum_ = 0.0f;
  float background_hue_ = 0;
  float rot_arg_ = 0.0f;

  bool texture_trigger_ = false;

  BeatEstimator beat_estimators_[3] = {{0.99f}, {0.99f}, {0.99f}};
  const char* beat_estimator_signal_names_[3] = {"bass_beat", "mid_beat",
                                                 "treble_beat"};
  const char* beat_estimator_signal_names_binned_[3] = {
      "bass_beat_bin", "mid_beat_bin", "treble_beat_bin"};
  const char* beat_estimator_signal_names_phase_[3] = {
      "bass_beat_phase", "mid_beat_phase", "treble_beat_phase"};
  const char* beat_estimator_signal_names_threshold_[3] = {
      "bass_beat_threshold", "mid_beat_threshold", "treble_beat_threshold"};
};

}  // namespace opendrop

#endif  // PRESET_SPACE_WHALE_EYE_WARP_SPACE_WHALE_EYE_WARP_H_
