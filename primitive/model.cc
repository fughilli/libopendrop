#include "libopendrop/primitive/model.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace opendrop {

void Model::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);

  // TODO: Enable depth testing. Needs an additional attachment to the render
  // context. glEnable(GL_DEPTH_TEST);

  glVertexPointer(3, GL_FLOAT, 0, vertices_);
  glTexCoordPointer(2, GL_FLOAT, 0, uvs_);
  glDrawElements(GL_QUADS, 4, GL_INT, triangles_);

  glDisableClientState(GL_VERTEX_ARRAY);
}

}  // namespace opendrop
