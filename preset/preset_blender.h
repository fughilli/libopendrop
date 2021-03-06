#ifndef LIBOPENDROP_PRESET_PRESET_BLENDER_H_
#define LIBOPENDROP_PRESET_PRESET_BLENDER_H_

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <list>
#include <memory>
#include <utility>
#include <vector>

#include "libopendrop/preset/preset.h"
#include "libopendrop/primitive/rectangle.h"
#include "libopendrop/util/logging.h"
#include "libopendrop/util/oneshot.h"

namespace opendrop {
enum PresetActivationState {
  kTransitionIn = 0,
  kIn,
  kAwaitingTransitionOut,
  kTransitionOut,
  kOut
};

class PresetActivation {
 public:
  PresetActivation(std::shared_ptr<Preset> preset,
                   std::shared_ptr<gl::GlRenderTarget> render_target,
                   float minimum_duration_s, float transition_duration_s);

  PresetActivationState Update(float dt);

  void TriggerTransitionOut();

  float GetMixingCoefficient() const;

  std::shared_ptr<Preset> preset() { return preset_; }
  std::shared_ptr<gl::GlRenderTarget> render_target() { return render_target_; }

 private:
  std::shared_ptr<Preset> preset_;
  std::shared_ptr<gl::GlRenderTarget> render_target_;
  PresetActivationState state_;
  OneshotIncremental<float> expiry_timer_, transition_timer_;
};

class PresetBlender {
 public:
  PresetBlender(int width, int height);

  template <typename... Args>
  void AddPreset(Args&&... args) {
    LOG(INFO) << "Adding preset";
    PresetActivation activation(std::forward<Args>(args)...);
    activation.preset()->UpdateGeometry(width_, height_);
    activation.render_target()->UpdateGeometry(width_, height_);
    preset_activations_.push_front(std::move(activation));
  }

  // Draws a single frame of blended preset output.
  void DrawFrame(absl::Span<const float> samples,
                 std::shared_ptr<GlobalState> state,
                 std::shared_ptr<gl::GlRenderTarget> output_render_target);

  void UpdateGeometry(int width, int height);

  size_t NumPresets() const { return preset_activations_.size(); }

 private:
  void Update(float dt);

  int width_, height_;
  std::shared_ptr<gl::GlProgram> blit_program_;
  std::list<PresetActivation> preset_activations_;

  Rectangle rectangle_;
};
}  // namespace opendrop

#endif  // LIBOPENDROP_PRESET_PRESET_BLENDER_H_
