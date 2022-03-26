#ifndef PRIMITIVE_RECTANGLE_H_
#define PRIMITIVE_RECTANGLE_H_

#include "primitive/primitive.h"
#include "third_party/glm_helper.h"

namespace opendrop {

class Rectangle : public Primitive {
 public:
  void Draw() override;

  void SetColor(glm::vec4 color) { color_ = color; }

 private:
  glm::vec4 color_ = {0.0f, 0.0f, 0.0f, 1.0f};
};

}  // namespace opendrop

#endif  // PRIMITIVE_RECTANGLE_H_
