#include "libopendrop/global_state.h"

namespace opendrop {

void GlobalState::Update(absl::Span<const float> samples, float dt) {
  // TODO: Implement using Eigen.
  properties_.dt = dt;
  properties_.time += dt;

  properties_.power = 0;
  // Note that this buffer is interleaved samples. Computing the power assuming
  // this is a mono buffer has the same outcome as averaging the power of the
  // left and right channels independently.
  for (auto sample : samples) {
    properties_.power += pow(sample, 2);
  }
  properties_.power /= samples.size();

  properties_.energy += properties_.power * dt;
}

}  // namespace opendrop
