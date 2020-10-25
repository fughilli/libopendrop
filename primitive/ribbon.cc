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
    : color_(color),
      num_segments_(num_segments),
      head_pointer_(kSegmentVertexCount),
      filled_(false) {
  // Reserve an extra segment at the beginning to be able to render the ribbon
  // circular buffer in two passes.
  vertices_.resize((num_segments + 1) * kSegmentVertexCount);
  indices_.resize((num_segments + 1) * kSegmentVertexCount);
  int i = 0;
  for (auto& index : indices_) {
    index = i++;
  }
}

void Ribbon::RenderTriangleStrip(absl::Span<glm::vec2> vertices) {
  if (vertices.size() <= kSegmentVertexCount) {
    return;
  }

  glEnableClientState(GL_VERTEX_ARRAY);
  glDisable(GL_DEPTH_TEST);
  glColor4f(color_.x, color_.y, color_.z, 1);
  glVertexPointer(2, GL_FLOAT, 0, vertices.data());
  CHECK(vertices.size() <= indices_.size()) << "More vertices than indices.";
  glDrawElements(GL_TRIANGLE_STRIP, vertices.size(), GL_UNSIGNED_SHORT,
                 indices_.data());
  glDisableClientState(GL_VERTEX_ARRAY);
}

void Ribbon::Draw() {
  if (filled_) {
    RenderTriangleStrip(absl::Span<glm::vec2>(vertices_.data(), head_pointer_));
    RenderTriangleStrip(absl::Span<glm::vec2>(
        vertices_.data() + head_pointer_, vertices_.size() - head_pointer_));
  } else {
    RenderTriangleStrip(absl::Span<glm::vec2>(
        &vertices_[kSegmentVertexCount], head_pointer_ - kSegmentVertexCount));
  }
}

void Ribbon::AppendSegment(std::pair<glm::vec2, glm::vec2> segment) {
  vertices_[head_pointer_++] = segment.first;
  vertices_[head_pointer_++] = segment.second;

  if (head_pointer_ == vertices_.size()) {
    filled_ = true;
    vertices_[0] = segment.first;
    vertices_[1] = segment.second;
    head_pointer_ = kSegmentVertexCount;
  }
}

void Ribbon::UpdateColor(glm::vec3 color) { color_ = color; }

}  // namespace opendrop
