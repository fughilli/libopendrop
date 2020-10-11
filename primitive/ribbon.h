#ifndef PRIMITIVES_RIBBON_H_
#define PRIMITIVES_RIBBON_H_

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "libopendrop/primitive/primitive.h"

namespace opendrop {

class Ribbon : Primitive {
 public:
  Ribbon(glm::vec3 color, int num_segments);

  void Draw() override;

  // Appends a segment to the ribbon. If there are already `num_segments`
  // segments, this operation also deletes the oldest segment. The order of the
  // segment vertices should be consistent, i.e., a ribbon must be constructed
  // like:
  //
  // first      first             first      first
  //  |          |                 |          |
  //  V          V                 V          V
  //  #----------#----- . . . -----#----------#
  //  |          |                 |          |
  //  #----------#----- . . . -----#----------#
  //  ^          ^                 ^          ^
  //  |          |                 |          |
  // second     second            second     second
  //
  // Crossing vertices can be used to add a twist to the ribbon:
  //
  // first      first   second     second
  //  |          |       |          |
  //  V          V       V          V
  //  #----------#--\ /--#----------#
  //  |          |   X   |          |
  //  #----------#--/ \--#----------#
  //  ^          ^       ^          ^
  //  |          |       |          |
  // second     second  first      first
  void AppendSegment(std::pair<glm::vec2, glm::vec2> segment);

  // Updates the color of the ribbon.
  void UpdateColor(glm::vec3 color);

 private:
  glm::vec3 color_;
  int num_segments_;
  std::vector<glm::vec2> vertices_;
  std::vector<uint16_t> indices_;
};

}  // namespace opendrop

#endif  // PRIMITIVES_RIBBON_H_
