#include "libopendrop/primitive/rectangle.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace opendrop {

namespace {
constexpr float kFullscreenVertices[] = {
    -1, -1, 0.1,  // Lower-left
    -1, 1,  0.1,  // Upper-left
    1,  1,  0.1,  // Upper-right
    1,  -1, 0.1,  // Lower-right
};

constexpr uint8_t kFullscreenIndices[] = {
    0,
    1,
    2,
    3,
};
}  // namespace

void Rectangle::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);
  // TODO: Parameterize the color.
  glDisable(GL_DEPTH_TEST);
  glColor4f(0., 0., 0., 1.);
  glVertexPointer(3, GL_FLOAT, 0, kFullscreenVertices);
  glDrawElements(GL_QUADS, 4, GL_UNSIGNED_BYTE, kFullscreenIndices);
  glDisableClientState(GL_VERTEX_ARRAY);
}

}  // namespace opendrop
