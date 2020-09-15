#ifndef PRIMITIVES_PRIMITIVE_H_
#define PRIMITIVES_PRIMITIVE_H_

namespace opendrop {

class Primitive {
 public:
  virtual ~Primitive() {}
  virtual void Draw() = 0;
};

}  // namespace opendrop

#endif  // PRIMITIVES_PRIMITIVE_H_
