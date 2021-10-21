#include "libopendrop/primitive/ngon.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

#include <cmath>
#include <vector>

namespace opendrop {

namespace {
constexpr float kDepth = 0.1;

void AddNgonPoint(float theta, std::vector<float>* vertices) {
  vertices->push_back(std::cos(theta));
  vertices->push_back(std::sin(theta));
  vertices->push_back(kDepth);
}
}  // namespace

Ngon::Ngon(int n) {
  vertices_ = {0.0, 0.0, kDepth};
  indices_.push_back(0);
  for (int i = 0; i < n; ++i) {
    AddNgonPoint(2 * M_PI * i / n, &vertices_);
    indices_.push_back(i + 1);
  }
  // Close the polygon by adding a final triangle to vertex #1.
  indices_.push_back(1);
}

void Ngon::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);
  // TODO: Parameterize the color.
  glDisable(GL_DEPTH_TEST);
  glColor4f(0., 0., 0., 1.);
  glVertexPointer(3, GL_FLOAT, 0, vertices_.data());
  glDrawElements(GL_TRIANGLE_FAN, indices_.size(), GL_UNSIGNED_INT, indices_.data());
  glDisableClientState(GL_VERTEX_ARRAY);
}

}  // namespace opendrop
