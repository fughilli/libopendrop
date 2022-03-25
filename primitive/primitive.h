#ifndef PRIMITIVE_PRIMITIVE_H_
#define PRIMITIVE_PRIMITIVE_H_

namespace opendrop {

class Primitive {
 public:
  virtual ~Primitive() {}
  virtual void Draw() = 0;
};

}  // namespace opendrop

#endif  // PRIMITIVE_PRIMITIVE_H_
