#include "libopendrop/primitive/polyline.h"

#include <vector>

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace opendrop {

Polyline::Polyline(absl::Span<const glm::vec2> vertices, float width)
    : vertices_(vertices), width_(width) {
  indices_.resize(vertices_.size(), 0);
  for (int i = 0; i < vertices_.size(); ++i) {
    indices_[i] = i;
  }
}

void Polyline::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glLineWidth(width_);
  glEnable(GL_LINE_SMOOTH);
  glDisable(GL_DEPTH_TEST);
  glColor4f(1, 1, 1, 1);
  glVertexPointer(2, GL_FLOAT, 0, vertices_.data());
  glDrawElements(GL_LINE_STRIP, vertices_.size(), GL_UNSIGNED_SHORT,
                 indices_.data());
  glDisableClientState(GL_VERTEX_ARRAY);
}

}  // namespace opendrop
