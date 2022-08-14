#ifndef UTIL_GRAPH_NEW_GRAPH_H_
#define UTIL_GRAPH_NEW_GRAPH_H_

namespace opendrop {

class Graph {};

class GraphBuilder {
 public:
  Graph Bridge(std::vector<Type> input_types, std::vector<Type> output_types);

  // Storage for objects needed by conversions.
  std::list<TypedPtr> storage_ = {};

 private:
  // Pointer to an object stored opaquely as bytes. The `opendrop::Type` is
  // recorded alongside the pointer.
  struct TypedPtr {
    Type type;
    std::shared_ptr<uint8_t> opaque;
  };
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_NEW_GRAPH_H_
