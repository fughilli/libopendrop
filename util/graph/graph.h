#ifndef UTIL_GRAPH_GRAPH_H_
#define UTIL_GRAPH_GRAPH_H_

#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "util/logging/logging.h"

namespace opendrop {

struct Node {
  std::vector<std::shared_ptr<Node>> incoming_edges = {};
};

class Graph {
 public:
 private:
  std::vector<std::shared_ptr<Node>> nodes_ = {};
};

enum class Type {
  kFloatGeneric,
  kUnitary,
  kMonotonic,
};

}  // namespace opendrop

std::ostream& operator<<(std::ostream& os,
                         const std::vector<opendrop::Type>& types) {
  for (const auto type : types) os << static_cast<int>(type) << ", ";
  return os;
}

namespace opendrop {

struct Monotonic {
  float value;

  Monotonic() : value(0) {}
  Monotonic(float value) : value(value) {}
  operator float() const { return value; }
};

struct Unitary {
  float value;

  Unitary() : value(0) {}
  Unitary(float value) : value(value) {}
  operator float() const { return value; }
};

template <typename T>
Type ToType();

template <>
Type ToType<Unitary>() {
  return Type::kUnitary;
}
template <>
Type ToType<Monotonic>() {
  return Type::kMonotonic;
}
template <>
Type ToType<float>() {
  return Type::kFloatGeneric;
}

template <typename First>
void ConstructTypesHelper(std::vector<Type>& v) {
  v.push_back(ToType<First>());
}

template <typename First, typename Second, typename... Remaining>
void ConstructTypesHelper(std::vector<Type>& v) {
  v.push_back(ToType<First>());
  ConstructTypesHelper<Second, Remaining...>(v);
}

template <typename Arg, typename... Args>
std::vector<Type> ConstructTypes() {
  std::vector<Type> v;
  v.reserve(sizeof...(Args) + 1);
  ConstructTypesHelper<Arg, Args...>(v);
  return v;
}
std::vector<Type> ConstructTypes() { return {}; }

template <typename Arg, typename... Args>
std::vector<Type> ConstructTypesFromTuple(const std::tuple<Arg, Args...>&) {
  return ConstructTypes<Arg, Args...>();
}
std::vector<Type> ConstructTypesFromTuple(const std::tuple<>&) { return {}; }

template <typename T>
struct ConversionStorage {
  constexpr static size_t alignment = alignof(T);
  constexpr static size_t size = sizeof(T);

  static std::shared_ptr<uint8_t> Allocate() {
    return std::shared_ptr<uint8_t>(
        reinterpret_cast<uint8_t*>(std::aligned_alloc(alignment, size)),
        std::free);
  }
};

// A conversion is a symbolic representation of a transformation, A -> B.
// It contains:
//   + A name.
//   + An implementation (function `B(*)(A)`).
//   + A location to store results from the implementation (previous B).
struct Conversion {
  // Constructs a std::function() that:
  //   + Accepts opaque pointers to buffers of A and B.
  //   + Computes `B = function(A)`.
  template <typename InputTuple, typename OutputTuple>
  static std::function<void(const void*, void*)> ToGenericFunction(
      std::function<OutputTuple(InputTuple)> function) {
    return [function](const void* input, void* output) {
      // TODO: Determine if this violates strict aliasing rules. My
      // understanding is that we can use some flag to disable this requirement.
      auto t_output = reinterpret_cast<OutputTuple*>(output);
      const auto t_input = reinterpret_cast<const InputTuple*>(input);
      *t_output = function(*t_input);
    };
  }

  // Determines whether or not this Conversion can produce an output compatible
  // with the input of `other`.
  bool CanOutputTo(const Conversion& other) {
    return other.input_types == output_types;
  }

  template <typename... InputTupleArgs, typename... OutputTupleArgs>
  Conversion(std::string name, std::function<std::tuple<OutputTupleArgs...>(
                                   std::tuple<InputTupleArgs...>)>
                                   convert)
      : name(name),
        convert(Conversion::ToGenericFunction(convert)),
        input_types(ConstructTypes<InputTupleArgs...>()),
        output_types(ConstructTypes<OutputTupleArgs...>()),
        output_storage(
            ConversionStorage<std::tuple<OutputTupleArgs...>>::Allocate()) {}

  template <typename... InputTupleArgs>
  Conversion& Invoke(const std::tuple<InputTupleArgs...>& input) {
    if (input_types != ConstructTypes<InputTupleArgs...>())
      LOG(FATAL) << "Input types do not match!";

    convert(reinterpret_cast<const void*>(&input),
            reinterpret_cast<void*>(output_storage.get()));

    return *this;
  }

  template <typename OutputTuple>
  OutputTuple Result() {
    OutputTuple result =
        *reinterpret_cast<const OutputTuple*>(output_storage.get());
    return result;
  }

  std::string name;
  std::function<void(const void*, void*)> convert;
  std::vector<Type> input_types;
  std::vector<Type> output_types;

  std::shared_ptr<uint8_t> output_storage;
};

class ComputeGraph {
 public:
  template <typename InputTuple, typename OutputTuple>
  void DeclareConversion(
      std::string name,
      std::function<OutputTuple(InputTuple)> conversion_function) {
    auto conversion = std::make_shared<Conversion>(name, conversion_function);

    InsertConversion(conversion);
  }

  void Construct(absl::string_view graph) {
    conversion_ = conversions_by_name_[std::string(graph)];
  }

  template <typename... OutputTypes, typename InputTuple>
  std::tuple<OutputTypes...> Evaluate(InputTuple input,
                                      std::tuple<OutputTypes...>& output) {
    if (conversion_->output_types != ConstructTypes<OutputTypes...>())
      LOG(FATAL) << "Incorrect output type of evaluation";
    //<< conversion_->output_types;
    conversion_->Invoke(input);
    return conversion_->Result<std::tuple<OutputTypes...>>();
  }

  template <typename OutputTuple, typename InputTuple>
  OutputTuple Evaluate(InputTuple input) {
    OutputTuple output;
    return Evaluate(input, output);
    return output;
  }

 private:
  void InsertConversion(std::shared_ptr<Conversion> conversion) {
    if (conversions_by_name_.find(conversion->name) !=
        conversions_by_name_.end())
      LOG(FATAL) << "Duplicate conversion found for name " << conversion->name;

    conversions_by_name_[conversion->name] = conversion;

    if (conversions_by_input_.find(conversion->input_types) ==
        conversions_by_input_.end())
      conversions_by_input_[conversion->input_types] = {};
    if (conversions_by_output_.find(conversion->output_types) ==
        conversions_by_output_.end())
      conversions_by_output_[conversion->output_types] = {};

    conversions_by_input_[conversion->input_types].push_back(conversion);
    conversions_by_output_[conversion->output_types].push_back(conversion);
  }
  using ConversionVector = std::vector<std::shared_ptr<Conversion>>;

  std::shared_ptr<Conversion> conversion_ = {};

  std::map<std::string, std::shared_ptr<Conversion>> conversions_by_name_;
  std::map<std::vector<Type>, ConversionVector> conversions_by_input_;
  std::map<std::vector<Type>, ConversionVector> conversions_by_output_;
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_GRAPH_H_
