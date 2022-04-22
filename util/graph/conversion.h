#ifndef UTIL_GRAPH_CONVERSION_H_
#define UTIL_GRAPH_CONVERSION_H_

#include <functional>
#include <memory>
#include <ostream>
#include <tuple>

#include "util/graph/tuple.h"
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
        input_factory(OpaqueTupleFactory::FromTypes<InputTupleArgs...>()),
        output_factory(OpaqueTupleFactory::FromTypes<OutputTupleArgs...>()),
        input_tuple(input_factory.Construct()),
        output_tuple(output_factory.Construct()) {}

  // Constructs a `Conversion` which produces a value of type
  // std::tuple<OutputTupleArgs...>.
  template <typename... OutputTupleArgs, typename... OutputConstructorArgs>
  Conversion(
      std::string name, std::function<std::tuple<OutputTupleArgs...>()> produce,
      std::tuple<OutputConstructorArgs...>&& output_constructor_args = {})
      : name(name),
        produce(ProductionToOpaqueFunction(produce)),
        input_factory(OpaqueTupleFactory::FromEmpty()),
        output_factory(OpaqueTupleFactory::FromTypes<OutputTupleArgs...>()),
        input_tuple(input_factory.Construct()),
        output_tuple(output_factory.Construct()) {}

  template <typename... InputTupleArgs>
  Conversion& Invoke(const std::tuple<InputTupleArgs...>& input) {
    LOG(DEBUG) << "Invoking function for conversion " << name;

    input_tuple.AssignFrom(input);

    if (output_factory.Types().empty())
      consume(input_tuple);
    else
      convert(input_tuple, output_tuple);

    return *this;
  }

  Conversion& Invoke() {
    LOG(DEBUG) << "Invoking function for conversion " << name;
    if (!input_factory.Types().empty())
      LOG(FATAL) << "Input types do not match!";
    produce(output_tuple);
    return *this;
  }

  Conversion& InvokeOpaque(OpaqueTuple& input) {
    LOG(DEBUG) << "Invoking function for conversion " << name;
    convert(input, output_tuple);
    return *this;
  }

  Conversion& InvokeOpaque(OpaqueTuple& input, OpaqueTuple& output) {
    LOG(DEBUG) << "Invoking function for conversion " << name;
    convert(input, output);
    return *this;
  }

  OpaqueTuple& ResultOpaque() { return output_tuple; }

  template <typename... OutputTypes>
  const std::tuple<OutputTypes...> Result() const {
    LOG(DEBUG) << "Fetching result";
    using OutputTuple = std::tuple<OutputTypes...>;
    if (output_factory.Types() != ConstructTypes<OutputTypes...>())
      LOG(FATAL) << "Output types do not match!";
    return output_tuple.ToTuple<OutputTypes...>();
  }

  const std::vector<Type>& InputTypes() const { return input_factory.Types(); }
  const std::vector<Type>& OutputTypes() const {
    return output_factory.Types();
  }

  std::string name;
  std::function<void(OpaqueTuple&, OpaqueTuple&)> convert = nullptr;
  std::function<void(OpaqueTuple&)> produce = nullptr;
  std::function<void(OpaqueTuple&)> consume = nullptr;

  OpaqueTupleFactory input_factory;
  OpaqueTupleFactory output_factory;

  // Storage for native invocations. For invocations in a graph, dedicated
  // storage shall be collected from the factories.
  OpaqueTuple input_tuple;
  OpaqueTuple output_tuple;
};

std::ostream& operator<<(std::ostream& os, const Conversion& conversion);
std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<Conversion>& conversion);

}  // namespace opendrop

#endif  // UTIL_GRAPH_CONVERSION_H_
