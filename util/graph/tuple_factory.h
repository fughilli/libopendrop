#ifndef UTIL_GRAPH_TUPLE_FACTORY_H_
#define UTIL_GRAPH_TUPLE_FACTORY_H_

#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "util/graph/tuple.h"
#include "util/graph/types/color.h"
#include "util/graph/types/monotonic.h"
#include "util/graph/types/opaque_storable.h"
#include "util/graph/types/samples.h"
#include "util/graph/types/texture.h"
#include "util/graph/types/types.h"
#include "util/graph/types/unitary.h"

namespace opendrop {

using OpaqueTupleFactoryStorageAndTupleType =
    std::pair<std::list<std::shared_ptr<uint8_t>>, OpaqueTuple>;
using OpaqueTupleFactoryFnType =
    OpaqueTupleFactoryStorageAndTupleType (*)(void);

class OpaqueTupleFactory {
 public:
  static std::pair<std::list<std::shared_ptr<uint8_t>>, OpaqueTuple>
  StorageAndTupleFromTypes(absl::Span<const Type> types) {
    std::vector<std::pair<Type, std::shared_ptr<uint8_t>>> types_and_memory{};

    for (auto type : types) {
      types_and_memory.push_back({type, MemoryFromType(type)});
    }

    std::list<std::shared_ptr<uint8_t>> storage{};
    for (auto& [_, memory] : types_and_memory) {
      storage.push_back(memory);
    }

    return std::make_pair(storage,
                          OpaqueTuple::FromTypesAndMemory(types_and_memory));
  }

  template <typename... Ts>
  static std::pair<std::list<std::shared_ptr<uint8_t>>, OpaqueTuple>
  StorageAndTupleFromTypes() {
    std::vector<std::pair<Type, std::shared_ptr<uint8_t>>> types_and_memory{};

    TypesAndMemoryFromTypesHelper<Ts...>(types_and_memory);

    std::list<std::shared_ptr<uint8_t>> storage{};
    for (auto& [_, memory] : types_and_memory) {
      storage.push_back(memory);
    }

    return std::make_pair(storage,
                          OpaqueTuple::FromTypesAndMemory(types_and_memory));
  }

 private:
  OpaqueTupleFactory() {
    allocators_by_type_[Type::kInteger] =
        OpaqueStorable<int>::template Allocate<>;
    allocators_by_type_[Type::kFloatGeneric] =
        OpaqueStorable<float>::template Allocate<>;
    allocators_by_type_[Type::kMonotonic] = Monotonic::template Allocate<>;
    allocators_by_type_[Type::kUnitary] = Unitary::template Allocate<>;
    allocators_by_type_[Type::kTexture] = Texture::template Allocate<>;
    allocators_by_type_[Type::kColor] = Color::template Allocate<>;
    allocators_by_type_[Type::kSamples] = Samples::template Allocate<>;
  }

  static OpaqueTupleFactory* GetInstance() {
    static OpaqueTupleFactory opaque_tuple_factory{};
    return &opaque_tuple_factory;
  }

  static std::shared_ptr<uint8_t> MemoryFromType(Type type) {
    return GetInstance()->allocators_by_type_[type]();
  }

  template <typename T, void (*)(void) = T::Allocate>
  static std::shared_ptr<uint8_t> AllocateHelper() {
    return T::Allocate();
  }

  template <typename T>
  static std::shared_ptr<uint8_t> AllocateHelper() {
    return OpaqueStorable<T>::Allocate();
  }

  template <typename T>
  static void TypesAndMemoryFromTypesHelper(
      std::vector<std::pair<Type, std::shared_ptr<uint8_t>>>&
          types_and_memory) {
    types_and_memory.push_back({ToType<T>(), AllocateHelper<T>()});
  }
  template <typename T1, typename T2, typename... Ts>
  static void TypesAndMemoryFromTypesHelper(
      std::vector<std::pair<Type, std::shared_ptr<uint8_t>>>&
          types_and_memory) {
    TypesAndMemoryFromTypesHelper<T1>(types_and_memory);
    TypesAndMemoryFromTypesHelper<T2, Ts...>(types_and_memory);
  }

  absl::flat_hash_map<Type, std::shared_ptr<uint8_t> (*)(void)>
      allocators_by_type_;
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_TUPLE_FACTORY_H_
