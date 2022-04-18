#ifndef UTIL_GRAPH_CONVERSION_H_
#define UTIL_GRAPH_CONVERSION_H_

#include <functional>
#include <memory>
#include <ostream>
#include <tuple>

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
struct ConversionStorage {
  constexpr static size_t alignment = alignof(T);
  constexpr static size_t size = sizeof(T);

  // Allocates memory with correct alignment for storing an object of type `T`
  // and invokes the constructor on it, forwarding any constructor arguments.
  template <typename... Args>
  static std::shared_ptr<uint8_t> Allocate(Args&&... args) {
    uint8_t* memory =
        reinterpret_cast<uint8_t*>(std::aligned_alloc(alignment, size));
    new (memory) T(std::forward<Args>(args)...);
    return std::shared_ptr<uint8_t>(memory, DestructAndFree<T>);
  }
};

// Constructs a std::function() that:
//   + Accepts opaque pointers to buffers of A and B (where A = InputTuple, B =
//     OutputTuple).
//   + Computes `B = function(A)`.
template <typename InputTuple, typename OutputTuple>
static std::function<void(const void*, void*)> ToGenericFunction(
    std::function<OutputTuple(InputTuple)> function) {
  return [function](const void* input, void* output) {
    // TODO: Determine if this violates strict aliasing rules. My
    // understanding is that we can use some flag to disable this
    // requirement.
    auto t_output = reinterpret_cast<OutputTuple*>(output);
    const auto t_input = reinterpret_cast<const InputTuple*>(input);
    *t_output = function(*t_input);
  };
}

// A conversion is a symbolic representation of a transformation, A -> B.
// It contains:
//   + A name.
//   + An implementation (function `B(*)(A)`).
//   + A location to store results from the implementation (previous B).
struct Conversion {
  // Determines whether or not this Conversion can produce an output
  // compatible with the input of `other`.
  bool CanOutputTo(const Conversion& other) {
    return other.input_types == output_types;
  }

  template <typename... InputTupleArgs, typename... OutputTupleArgs,
            typename... OutputConstructorArgs>
  Conversion(
      std::string name,
      std::function<
          std::tuple<OutputTupleArgs...>(std::tuple<InputTupleArgs...>)>
          convert,
      std::tuple<OutputConstructorArgs...>&& output_constructor_args = {})
      : name(name),
        convert(ToGenericFunction(convert)),
        input_types(ConstructTypes<InputTupleArgs...>()),
        output_types(ConstructTypes<OutputTupleArgs...>()),
        output_storage(
            std::apply(ConversionStorage<std::tuple<OutputTupleArgs...>>::
                           template Allocate<OutputConstructorArgs...>,
                       output_constructor_args)) {}

  template <typename... InputTupleArgs>
  Conversion& Invoke(const std::tuple<InputTupleArgs...>& input) {
    LOG(DEBUG) << "Invoking function";
    if (input_types != ConstructTypes<InputTupleArgs...>())
      LOG(FATAL) << "Input types do not match!";

    convert(reinterpret_cast<const void*>(&input),
            reinterpret_cast<void*>(output_storage.get()));

    return *this;
  }

  Conversion& InvokeOpaque(const std::shared_ptr<uint8_t>& input) {
    LOG(DEBUG) << "Invoking function";
    convert(reinterpret_cast<const void*>(input.get()),
            reinterpret_cast<void*>(output_storage.get()));

    return *this;
  }

  std::shared_ptr<uint8_t> ResultOpaque() { return output_storage; }

  template <typename... OutputTypes>
  const std::tuple<OutputTypes...>& Result() const {
    LOG(DEBUG) << "Fetching result";
    using OutputTuple = std::tuple<OutputTypes...>;
    if (output_types != ConstructTypes<OutputTypes...>())
      LOG(FATAL) << "Output types do not match!";
    return *reinterpret_cast<OutputTuple*>(output_storage.get());
  }

  std::string name;
  std::function<void(const void*, void*)> convert;
  std::vector<Type> input_types;
  std::vector<Type> output_types;

  std::shared_ptr<uint8_t> output_storage;
};

std::ostream& operator<<(std::ostream& os, const Conversion& conversion);
std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<Conversion>& conversion);

}  // namespace opendrop

#endif  // UTIL_GRAPH_CONVERSION_H_
