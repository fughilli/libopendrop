#ifndef UTIL_GRAPH_TUPLE_H_
#define UTIL_GRAPH_TUPLE_H_

#include <stdint.h>

#include <memory>
#include <type_traits>
#include <utility>

#include "absl/strings/str_format.h"
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

struct OpaqueTuple {
  struct Cell {
    std::shared_ptr<uint8_t> buffer;
    Type type;
  };

  std::vector<Cell> cells{};
  std::vector<Type> types{};

  template <typename T1, typename T2, typename... Ts>
  void FromTypesHelper() {
    FromTypesHelper<T1>();
    FromTypesHelper<T2, Ts...>();
  }
  template <typename T>
  void FromTypesHelper() {
    cells.push_back(
        Cell{.buffer = TupleStorage<T>::Allocate(), .type = ToType<T>()});
    types.push_back(ToType<T>());
  }

  template <typename... Ts>
  static OpaqueTuple FromTypes() {
    OpaqueTuple opaque_tuple{};
    opaque_tuple.FromTypesHelper<Ts...>();
    return opaque_tuple;
  }

  template <typename T>
  T& Get(int index) {
    if (index < -0 || index >= cells.size())
      LOG(FATAL) << absl::StrFormat(
          "index out of bounds (index = %d; cells.size() = %d)", index,
          cells.size());
    Cell& cell = cells[index];
    if (cell.type != ToType<T>())
      LOG(FATAL) << absl::StrFormat(
          "requesting incorrect type (T = %s) for cell %d of type %s",
          ToString(ToType<T>()), index, ToString(cell.type));

    return *reinterpret_cast<T*>(cell.buffer.get());
  }

  template <size_t index, typename TupleType, typename T1, typename T2,
            typename... Ts>
  void AssignFromHelper(TupleType tuple) {
    AssignFromHelper<index, TupleType, T1>(tuple);
    AssignFromHelper<index + 1, TupleType, T2, Ts...>(tuple);
  }
  template <size_t index, typename TupleType, typename T1>
  void AssignFromHelper(TupleType tuple) {
    Get<T1>(index) = std::get<index>(tuple);
  }

  template <typename... Ts>
  void AssignFrom(std::tuple<Ts...>&& tuple) {
    AssignFromHelper<0, std::tuple<Ts...>, Ts...>(tuple);
  }
  template <typename... Ts>
  void AssignFrom(std::tuple<Ts...>& tuple) {
    AssignFromHelper<0, std::tuple<Ts...>, Ts...>(tuple);
  }

  template <typename... Ts, size_t... I>
  std::tuple<Ts...> ToTupleImpl(std::index_sequence<I...>) {
    return std::tuple<Ts...>(Get<std::remove_reference_t<Ts>>(I)...);
  }

  template <typename... Ts,
            typename Indices = std::make_index_sequence<sizeof...(Ts)>>
  std::tuple<Ts...> ToTuple() {
    return ToTupleImpl<Ts...>(Indices{});
  }
};

// Constructs a std::function() that:
//   + Accepts opaque pointers to buffers of A and B (where A = InputTuple, B =
//     OutputTuple).
//   + Computes `B = function(A)`.
template <typename... InputTypes, typename OutputTuple>
static std::function<void(OpaqueTuple&, OpaqueTuple&)> ToOpaqueFunction(
    std::function<OutputTuple(std::tuple<InputTypes...>)> function) {
  return [function](OpaqueTuple& input, OpaqueTuple& output) {
    // TODO: Determine if this violates strict aliasing rules. My
    // understanding is that we can use some flag to disable this
    // requirement.
    output.AssignFrom(function(input.ToTuple<InputTypes...>()));
  };
}

}  // namespace opendrop

#endif  // UTIL_GRAPH_TUPLE_H_
