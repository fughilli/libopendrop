#ifndef PRIMITIVES_NGON_H_
#define PRIMITIVES_NGON_H_

#include <stdint.h>

#include <vector>

#include "primitive/primitive.h"

namespace opendrop {

class Ngon : public Primitive {
 public:
  Ngon(int n);
  void Draw() override;

 private:
  std::vector<float> vertices_;
  std::vector<int32_t> indices_;
};

}  // namespace opendrop

#endif  // PRIMITIVES_NGON_H_
