#include "libopendrop/primitive/model.h"

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>

namespace opendrop {

void Model::Draw() {
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);

  // TODO: Enable depth testing. Needs an additional attachment to the render
  // context. glEnable(GL_DEPTH_TEST);

  glVertexPointer(3, GL_FLOAT, 0, vertices_.data());
  glTexCoordPointer(2, GL_FLOAT, 0, uvs_.data());
  if (normals_.size()) glNormalPointer(GL_FLOAT, 0, normals_.data());
  glDrawElements(GL_TRIANGLES, triangles_.size() * 3, GL_UNSIGNED_INT,
                 triangles_.data());

  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);
}

}  // namespace opendrop
