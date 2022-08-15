#ifndef UTIL_GRAPH_TYPES_OPAQUE_STORABLE_H_
#define UTIL_GRAPH_TYPES_OPAQUE_STORABLE_H_

#include <memory>
#include <typeinfo>

#include "util/logging/logging.h"

namespace opendrop {

template <typename Derived>
struct OpaqueStorable {
  // Deletes an object of type `T` and frees the associated memory.
  static void DestructAndFree(void* ptr) {
    delete reinterpret_cast<Derived*>(ptr);
  }

  // Allocates memory with correct alignment for storing an object of type `T`
  // and invokes the constructor on it, forwarding any constructor arguments.
  template <typename... Args>
  static std::shared_ptr<uint8_t> Allocate(Args&&... args) {
    Derived* memory = new Derived(std::forward<Args>(args)...);
    return std::shared_ptr<uint8_t>(reinterpret_cast<uint8_t*>(memory),
                                    DestructAndFree);
  }
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_OPAQUE_STORABLE_H_
