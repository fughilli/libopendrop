#ifndef PRIMITIVES_RECTANGLE_H_
#define PRIMITIVES_RECTANGLE_H_

#include "libopendrop/primitive/primitive.h"

namespace opendrop {

class Rectangle : public Primitive {
 public:
  void Draw() override;
};

}  // namespace opendrop

#endif  // PRIMITIVES_RECTANGLE_H_
