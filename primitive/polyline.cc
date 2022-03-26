#include "primitive/polyline.h"

#include <array>
#include <vector>

#include "third_party/gl_helper.h"
#include "third_party/glm_helper.h"

namespace opendrop {

namespace {
constexpr std::array<glm::vec2, 0> kEmptyVertices = {};
}

Polyline::Polyline()
    : color_(glm::vec3(0.0f, 0.0f, 0.0f)),
      vertices_(kEmptyVertices),
      width_(0.0f) {}

Polyline::Polyline(glm::vec3 color, absl::Span<const glm::vec2> vertices,
                   float width)
    : color_(color), width_(width) {
  UpdateVertices(vertices);
}

void Polyline::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glLineWidth(width_);
  glEnable(GL_LINE_SMOOTH);
  glDisable(GL_DEPTH_TEST);
  glColor4f(color_.x, color_.y, color_.z, 1);
  glVertexPointer(2, GL_FLOAT, 0, vertices_.data());
  glDrawElements(GL_LINE_STRIP, indices_.size(), GL_UNSIGNED_SHORT,
                 indices_.data());
  glDisableClientState(GL_VERTEX_ARRAY);
}

void Polyline::UpdateVertices(absl::Span<const glm::vec2> vertices) {
  vertices_ = vertices;

  if (indices_.size() == vertices_.size()) {
    return;
  }

  indices_.resize(vertices_.size(), 0);
  for (int i = 0; i < vertices_.size(); ++i) {
    indices_[i] = i;
  }
}

void Polyline::UpdateColor(glm::vec3 color) { color_ = color; }

void Polyline::UpdateWidth(float width) { width_ = width; }

}  // namespace opendrop
