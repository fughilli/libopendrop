#ifndef PRIMITIVES_RIBBON_H_
#define PRIMITIVES_RIBBON_H_

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>

#include "absl/types/span.h"
#include "primitive/primitive.h"

namespace opendrop {

template <typename T>
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
  void AppendSegment(std::pair<T, T> segment);

  // Updates the color of the ribbon.
  void UpdateColor(glm::vec3 color);

 private:
  void RenderTriangleStrip(absl::Span<T> vertices);

  glm::vec3 color_;
  int num_segments_;
  std::vector<T> vertices_;
  std::vector<uint16_t> indices_;

  // The location where the next segment will be added.
  intptr_t head_pointer_;
  // Whether or not the buffer has been filled with segments yet.
  bool filled_;
};

}  // namespace opendrop

#include "primitive/ribbon.cc"

#endif  // PRIMITIVES_RIBBON_H_
