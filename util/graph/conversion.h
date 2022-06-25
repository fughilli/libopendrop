#ifndef UTIL_GRAPH_CONVERSION_H_
#define UTIL_GRAPH_CONVERSION_H_

#include <functional>
#include <memory>
#include <ostream>
#include <tuple>

#include "util/graph/tuple.h"
#include "util/graph/tuple_factory.h"
#include "util/graph/types/types.h"
#include "util/logging/logging.h"

namespace opendrop {

// Constructs a std::function() that:
//   + Accepts `OpaqueTuple`s A and B (where A contains types InputTypes, and B
//   is assignable from OutputTuple).
//   + Computes `B = function(A)`.
template <typename... InputTypes, typename OutputTuple>
static std::function<void(OpaqueTuple&, OpaqueTuple&)>
ConversionToOpaqueFunction(
    std::function<OutputTuple(std::tuple<InputTypes...>)> function) {
  return [function](OpaqueTuple& input, OpaqueTuple& output) {
    // TODO: Determine if this violates strict aliasing rules. My
    // understanding is that we can use some flag to disable this
    // requirement.
    output.AssignFrom(function(input.ToRefTuple<InputTypes...>()));
  };
}

// Constructs a std::function() that:
//   + Accepts `OpaqueTuple` B (where B is assignable from OutputTuple).
//   + Computes `B = function()`.
template <typename OutputTuple>
static std::function<void(OpaqueTuple&)> ProductionToOpaqueFunction(
    std::function<OutputTuple()> function) {
  return [function](OpaqueTuple& output) { output.AssignFrom(function()); };
}

// Constructs a std::function() that:
//   + Accepts `OpaqueTuple` A (where A contains types InputTypes).
//   + Computes `function(A)`.
template <typename... InputTypes>
static std::function<void(OpaqueTuple&)> ConsumptionToOpaqueFunction(
    std::function<void(std::tuple<InputTypes...>)> function) {
  return [function](OpaqueTuple& input) {
    function(input.ToRefTuple<InputTypes...>());
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
    return other.InputTypes() == OutputTypes();
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
        convert(ConversionToOpaqueFunction(convert)),
        input_types(ConstructTypes<InputTupleArgs...>()),
        output_types(ConstructTypes<OutputTupleArgs...>()),
        input_allocator(
            OpaqueTupleFactory::StorageAndTupleFromTypes<InputTupleArgs...>),
        output_allocator(
            OpaqueTupleFactory::StorageAndTupleFromTypes<OutputTupleArgs...>) {}

  // Constructs a `Conversion` which produces a value of type
  // std::tuple<OutputTupleArgs...>.
  template <typename... OutputTupleArgs, typename... OutputConstructorArgs>
  Conversion(
      std::string name, std::function<std::tuple<OutputTupleArgs...>()> produce,
      std::tuple<OutputConstructorArgs...>&& output_constructor_args = {})
      : name(name),
        produce(ProductionToOpaqueFunction(produce)),
        input_types({}),
        output_types(ConstructTypes<OutputTupleArgs...>()),
        input_allocator(nullptr),
        output_allocator(
            OpaqueTupleFactory::StorageAndTupleFromTypes<OutputTupleArgs...>) {}

  Conversion& InvokeOpaque(OpaqueTuple& input, OpaqueTuple& output) {
    LOG(DEBUG) << "Invoking function for conversion " << name;
    if (convert != nullptr)
      convert(input, output);
    if (produce != nullptr)
      produce(output);
    return *this;
  }

  const std::vector<Type>& InputTypes() const { return input_types; }
  const std::vector<Type>& OutputTypes() const { return output_types; }

  OpaqueTupleFactoryStorageAndTupleType ConstructInputStorageAndTuple() const {
    return input_allocator();
  }
  OpaqueTupleFactoryStorageAndTupleType ConstructOutputStorageAndTuple() const {
    return output_allocator();
  }

  std::string name;
  std::function<void(OpaqueTuple&, OpaqueTuple&)> convert = nullptr;
  std::function<void(OpaqueTuple&)> produce = nullptr;
  std::function<void(OpaqueTuple&)> consume = nullptr;

  std::vector<Type> input_types{}, output_types{};

  OpaqueTupleFactoryFnType input_allocator;
  OpaqueTupleFactoryFnType output_allocator;
};

std::ostream& operator<<(std::ostream& os, const Conversion& conversion);
std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<Conversion>& conversion);

}  // namespace opendrop

#endif  // UTIL_GRAPH_CONVERSION_H_
