#include "libopendrop/primitive/ribbon.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include "libopendrop/util/logging.h"

namespace opendrop {

namespace {
constexpr int kSegmentVertexCount = 2;
}

Ribbon::Ribbon(glm::vec3 color, int num_segments)
    : color_(color), num_segments_(num_segments) {
  indices_.resize(num_segments * 2);
  int i = 0;
  for (auto& index : indices_) {
    index = i++;
  }
}

void Ribbon::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_DEPTH_TEST);
  glColor4f(color_.x, color_.y, color_.z, 1);
  glVertexPointer(2, GL_FLOAT, 0, vertices_.data());
  CHECK(vertices_.size() <= indices_.size()) << "More vertices than indices.";
  glDrawElements(GL_TRIANGLE_STRIP, vertices_.size(), GL_UNSIGNED_SHORT,
                 indices_.data());
  glDisableClientState(GL_VERTEX_ARRAY);
}

void Ribbon::AppendSegment(std::pair<glm::vec2, glm::vec2> segment) {
  // If we have already accumulated the configured number of segments, move the
  // segments to the beginning and resize down.
  //
  // TODO: Optimize this implementation by maintaining a circular buffer with a
  // repeated "head", such that the ribbon can be drawn in two segments. This
  // will remove the need to copy all of the elements backwards in the vertex
  // list. This can also be implemented by playing games with `indices_` while
  // always writing into `vertices_` like a normal circular buffer.
  if (vertices_.size() >= (num_segments_ * kSegmentVertexCount)) {
    std::copy(std::next(vertices_.begin(), kSegmentVertexCount),
              vertices_.end(), vertices_.begin());
    vertices_.resize(vertices_.size() - kSegmentVertexCount);
  }

  vertices_.push_back(segment.first);
  vertices_.push_back(segment.second);
}

void Ribbon::UpdateColor(glm::vec3 color) { color_ = color; }

}  // namespace opendrop
