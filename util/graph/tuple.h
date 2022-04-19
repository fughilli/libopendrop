#ifndef UTIL_GRAPH_TUPLE_H_
#define UTIL_GRAPH_TUPLE_H_

#include <stdint.h>

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "util/graph/types/types.h"
#include "util/logging/logging.h"

namespace opendrop {

// Deletes an object of type `T` and frees the associated memory.
template <typename T>
void DestructAndFree(void* ptr) {
  delete reinterpret_cast<T*>(ptr);
}

// Convenience interface for constructing storage for an object of type `T`.
template <typename T>
struct TupleStorage {
  constexpr static size_t alignment = alignof(T);
  constexpr static size_t size = sizeof(T);

  // Allocates memory with correct alignment for storing an object of type `T`
  // and invokes the ructor on it, forwarding any ructor arguments.
  template <typename... Args>
  static std::shared_ptr<uint8_t> Allocate(Args&&... args) {
    uint8_t* memory =
        reinterpret_cast<uint8_t*>(std::aligned_alloc(alignment, size));
    new (memory) T(std::forward<Args>(args)...);
    return std::shared_ptr<uint8_t>(memory, DestructAndFree<T>);
  }
};

// Class which stores pointers to opaque buffers containing values of types
// specified in util/graph/types/types.h:Type. The buffers can be accessed as
// their given type with Get<T>(), but can also be manipulated by working with
// the pointers opaquely (e.g., to reorder the values or construct mappings into
// other OpaqueTuples).
class OpaqueTuple {
 public:
  // Returns the list of types contained in this OpaqueTuple.
  const std::vector<Type>& Types() const { return types_; }

  // Constructs an OpaqueTuple from the given set of types. Each type must be
  // default-constructible.
  template <typename... Ts>
  static OpaqueTuple ConstructFromTypes() {
    OpaqueTuple opaque_tuple{};
    opaque_tuple.FromTypesImpl</*empty=*/false, Ts...>();
    return opaque_tuple;
  }

  // Constructs an OpaqueTuple from the given set of types, leaving all cells
  // empty (dangling pointers).
  template <typename... Ts>
  static OpaqueTuple EmptyFromTypes() {
    OpaqueTuple opaque_tuple{};
    opaque_tuple.FromTypesImpl</*empty=*/true, Ts...>();
    return opaque_tuple;
  }

  // Constructs an OpaqueTuple from the given set of types and associated
  // memory. The memory is assumed to be properly sized and aligned and have a
  // correctly constructed object of the specified type within it.
  static OpaqueTuple FromTypesAndMemory(
      absl::Span<const std::pair<Type, std::shared_ptr<uint8_t>>>
          types_and_memory) {
    OpaqueTuple opaque_tuple{};
    for (auto& [type, memory] : types_and_memory) {
      opaque_tuple.types_.push_back(type);
      opaque_tuple.cells_.push_back(Cell{.buffer = memory, .type = type});
    }
    return opaque_tuple;
  }

  // Accesses the element of type `T` at `index`. If the requested type is does
  // not match the stored value, or if the index is out of bounds, LOG(FATAL)s.
  template <typename T>
  T& Get(int index) {
    CheckIndex(index);
    Cell& cell = cells_[index];
    if (cell.type != ToType<T>())
      LOG(FATAL) << absl::StrFormat(
          "requesting incorrect type (T = %s) for cell %d of type %s",
          ToString(ToType<T>()), index, ToString(cell.type));
    if (cell.buffer == nullptr)
      LOG(FATAL) << absl::StrFormat("value in cell %d of type %s is nullptr",
                                    index, ToString(cell.type));

    return *reinterpret_cast<T*>(cell.buffer.get());
  }

  // Assigns the elements in this OpaqueTuple from an std::tuple rvalue-ref
  // containing matching types.
  template <typename... Ts>
  void AssignFrom(std::tuple<Ts...>&& tuple) {
    AssignFromImpl<0, std::tuple<Ts...>, Ts...>(tuple);
  }

  // Assigns the elements in this OpaqueTuple from an std::tuple containing
  // matching types.
  template <typename... Ts>
  void AssignFrom(std::tuple<Ts...>& tuple) {
    AssignFromImpl<0, std::tuple<Ts...>, Ts...>(tuple);
  }

  // Returns an std::tuple of references to the contained values in this
  // OpaqueTuple.
  template <typename... Ts,
            typename Indices = std::make_index_sequence<sizeof...(Ts)>>
  std::tuple<Ts...> ToRefTuple() {
    return ToRefTupleImpl<Ts...>(Indices{});
  }

  // Reassigns the pointer in a cell of this OpaqueTuple to a cell in another
  // OpaqueTuple (aliasing). LOG(FATAL)s if the cells are of incompatible types.
  // Any value currently in the cell being aliased is destructed and the memory
  // freed.
  void Alias(int index, OpaqueTuple& target, int target_index) {
    CheckIndex(index);
    target.CheckIndex(target_index);

    auto& cell = cells_[index];
    auto& target_cell = target.cells_[target_index];
    if (cell.type != target_cell.type)
      LOG(FATAL) << absl::StrFormat(
          "attempting to alias cell %d to target cell of incorrect type "
          "(target cell %d type is %s, expected %s)",
          index, target_index, ToString(target_cell.type), ToString(cell.type));
    if (target_cell.buffer == nullptr)
      LOG(FATAL) << absl::StrFormat(
          "value in target cell %d of type %s is nullptr", target_index,
          ToString(target_cell.type));

    cell.buffer = target_cell.buffer;
  }

 private:
  OpaqueTuple() = default;

  // Storage for an element of OpaqueTuple.
  struct Cell {
    std::shared_ptr<uint8_t> buffer;
    Type type;
  };

  // Checks whether or not an index is valid. LOG(FATAL)s if it is not.
  void CheckIndex(int index) {
    if (index < 0 || index >= cells_.size())
      LOG(FATAL) << absl::StrFormat(
          "index out of bounds (index = %d; cells_.size() = %d)", index,
          cells_.size());
  }

  // Implementation for FromTypes<Ts...>().
  template <bool empty, typename T1, typename T2, typename... Ts>
  void FromTypesImpl() {
    FromTypesImpl<empty, T1>();
    FromTypesImpl<empty, T2, Ts...>();
  }
  template <bool empty, typename T>
  void FromTypesImpl() {
    if constexpr (empty) {
      cells_.push_back(Cell{.buffer = nullptr, .type = ToType<T>()});
    } else {
      cells_.push_back(
          Cell{.buffer = TupleStorage<T>::Allocate(), .type = ToType<T>()});
    }
    types_.push_back(ToType<T>());
  }

  // Implementation for AssignFrom<Ts...>().
  template <size_t index, typename TupleType, typename T1, typename T2,
            typename... Ts>
  void AssignFromImpl(TupleType tuple) {
    AssignFromImpl<index, TupleType, T1>(tuple);
    AssignFromImpl<index + 1, TupleType, T2, Ts...>(tuple);
  }
  template <size_t index, typename TupleType, typename T1>
  void AssignFromImpl(TupleType tuple) {
    Get<T1>(index) = std::get<index>(tuple);
  }

  // Implementation for ToRefTuple<Ts...>().
  template <typename... Ts, size_t... I>
  std::tuple<Ts...> ToRefTupleImpl(std::index_sequence<I...>) {
    return std::tuple<Ts...>(Get<std::remove_reference_t<Ts>>(I)...);
  }

  // Storage of all elements of OpaqueTuple.
  std::vector<Cell> cells_{};

  // Convenience types list.
  std::vector<Type> types_{};
};

class OpaqueTupleFactory {
 public:
  // Constructs an OpaqueTupleFactory which produces OpaqueTuple of the
  // specified types.
  template <typename... Ts>
  static OpaqueTupleFactory FromTypes() {
    OpaqueTupleFactory factory{};
    factory.FromTypesImpl<Ts...>();
    return factory;
  }

  // Constructs an OpaqueTuple of the types configured for this factory.
  OpaqueTuple Construct() {
    std::vector<std::pair<Type, std::shared_ptr<uint8_t>>> types_and_memory{};
    for (int i = 0; i < types_.size(); ++i) {
      types_and_memory.push_back(std::make_pair(types_[i], allocators_[i]()));
    }

    return OpaqueTuple::FromTypesAndMemory(types_and_memory);
  }

  // Produces an empty OpaqueTuple of the types configured for this factory.
  OpaqueTuple Empty() {
    std::vector<std::pair<Type, std::shared_ptr<uint8_t>>> types_and_memory{};
    for (int i = 0; i < types_.size(); ++i) {
      types_and_memory.push_back(std::make_pair(types_[i], nullptr));
    }

    return OpaqueTuple::FromTypesAndMemory(types_and_memory);
  }

 private:
  OpaqueTupleFactory() = default;

  // Implementation for FromTypes<Ts...>().
  template <typename T1, typename T2, typename... Ts>
  void FromTypesImpl() {
    FromTypesImpl<T1>();
    FromTypesImpl<T2, Ts...>();
  }
  template <typename T>
  void FromTypesImpl() {
    // TODO: Support memoizing constructor arguments.
    //
    // We work around this by only supporting objects in OpaqueTuple which are
    // default-constructible.
    allocators_.push_back(AllocatorType(TupleStorage<T>::template Allocate<>));
    types_.push_back(ToType<T>());
  }

  using AllocatorType = std::function<std::shared_ptr<uint8_t>()>;

  std::vector<Type> types_{};
  std::vector<AllocatorType> allocators_{};
};

// Constructs a std::function() that:
//   + Accepts `OpaqueTuple`s A and B (where A contains types InputTypes, and B
//   is assignable from OutputTuple).
//   + Computes `B = function(A)`.
template <typename... InputTypes, typename OutputTuple>
static std::function<void(OpaqueTuple&, OpaqueTuple&)> ToOpaqueFunction(
    std::function<OutputTuple(std::tuple<InputTypes...>)> function) {
  return [function](OpaqueTuple& input, OpaqueTuple& output) {
    // TODO: Determine if this violates strict aliasing rules. My
    // understanding is that we can use some flag to disable this
    // requirement.
    output.AssignFrom(function(input.ToRefTuple<InputTypes...>()));
  };
}

}  // namespace opendrop

#endif  // UTIL_GRAPH_TUPLE_H_
