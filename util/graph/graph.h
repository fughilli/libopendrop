#ifndef UTIL_GRAPH_GRAPH_H_
#define UTIL_GRAPH_GRAPH_H_

#include <cstdlib>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "util/logging/logging.h"

namespace opendrop {

enum class Type {
  kFloatGeneric,
  kUnitary,
  kMonotonic,
  kTexture,
  kInteger,
};

std::ostream& operator<<(std::ostream& os, Type type) {
#define CASE(x)    \
  case Type::k##x: \
    os << "k" #x;  \
    break;
  switch (type) {
    CASE(FloatGeneric);
    CASE(Unitary);
    CASE(Monotonic);
    CASE(Texture);
    CASE(Integer);
  }
#undef CASE
  return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Type>& types) {
  for (int i = 0; i < types.size(); ++i) {
    os << types[i];
    if (i != types.size() - 1) os << ", ";
  }
  return os;
}
std::ostream& operator<<(std::ostream& os, const std::list<Type>& types) {
  for (auto iter = types.begin(); iter != types.end(); ++iter) {
    os << *iter << ", ";
  }
  return os;
}

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

class Texture {
 public:
  Texture(size_t width, size_t height) : height_(height), width_(width) {}
  Texture operator-(const Texture& other) const {
    if (width_ != other.width_ || height_ != other.height_)
      LOG(FATAL) << absl::StrFormat(
          "Width and height don't match (lhs = %dx%d, rhs = %dx%d)", width_,
          height_, other.width_, other.height_);

    Texture ret(width_, height_);
    ret.color_ = color_ - other.color_;
    return ret;
  }
  // Compute the geometric mean of the amplitude of the raster.
  float Length() const { return glm::length(color_); }

  // static Texture Allocate(gl::GlTextureManager& texture_manager) {
  //   absl::StatusOr<int> texture_unit = texture_manager.Allocate();
  //   CHECK_OK(texture_unit);
  //   return Texture { .texture_unit = texture_unit.get(); };
  // }
  //
  int ActivateRenderContext() { return 0; }

  static Texture SolidColor(glm::vec4 color, size_t width, size_t height) {
    // std::vector<glm::vec4> solid(width * height * 4, 0);
    // glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_BGRA,
    // GL_UNSIGNED_BYTE, &emptyData[0]);
    Texture tex(width, height);
    tex.color_ = color;
    return tex;
  }

 private:
  glm::vec4 color_ = {};
  size_t width_, height_;
};

std::ostream& operator<<(std::ostream& os, const Texture& texture) {
  return os << "Texture(<no info>)";
}

template <typename T>
Type ToType();

// #define DECLARE_TYPE_CONVERSION(type) \
//   template <>                         \
//   Type ToType<type>() {               \
//     return Type::k##type;             \
//   }
//
// DECLARE_TYPE_CONVERSION(Unitary);
template <>
Type ToType<Unitary>() {
  return Type::kUnitary;
}
template <>
Type ToType<Monotonic>() {
  return Type::kMonotonic;
}
template <>
Type ToType<int>() {
  return Type::kInteger;
}
template <>
Type ToType<float>() {
  return Type::kFloatGeneric;
}
template <>
Type ToType<Texture>() {
  return Type::kTexture;
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
      // understanding is that we can use some flag to disable this
      // requirement.
      auto t_output = reinterpret_cast<OutputTuple*>(output);
      const auto t_input = reinterpret_cast<const InputTuple*>(input);
      *t_output = function(*t_input);
    };
  }

  // Determines whether or not this Conversion can produce an output
  // compatible with the input of `other`.
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
    LOG(INFO) << "Invoking function";
    if (input_types != ConstructTypes<InputTupleArgs...>())
      LOG(FATAL) << "Input types do not match!";

    convert(reinterpret_cast<const void*>(&input),
            reinterpret_cast<void*>(output_storage.get()));

    return *this;
  }

  Conversion& InvokeOpaque(const std::shared_ptr<uint8_t>& input) {
    LOG(INFO) << "Invoking function";
    convert(reinterpret_cast<const void*>(input.get()),
            reinterpret_cast<void*>(output_storage.get()));

    return *this;
  }

  std::shared_ptr<uint8_t> ResultOpaque() { return output_storage; }

  template <typename... OutputTypes>
  std::tuple<OutputTypes...> Result() {
    LOG(INFO) << "Fetching result";
    using OutputTuple = std::tuple<OutputTypes...>;
    if (output_types != ConstructTypes<OutputTypes...>())
      LOG(FATAL) << "Output types do not match!";
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

std::ostream& operator<<(std::ostream& os, const Conversion& conversion) {
  return os << "Conversion(input_types = " << conversion.input_types
            << ", output_types = " << conversion.output_types << ")";
}
std::ostream& operator<<(std::ostream& os,
                         const std::shared_ptr<Conversion>& conversion) {
  if (conversion == nullptr) {
    LOG(INFO) << "<nullptr>";
    return os;
  }
  LOG(INFO) << "Formatting shared_ptr<Conversion>";
  return os << *conversion.get();
}

class Graph {
 public:
  template <typename... InputTypes, typename... OutputTypes>
  void Evaluate(std::tuple<InputTypes...> input,
                std::tuple<OutputTypes...>& output) {
    CHECK_NULL(input_node);
    if (input_node->input_types != ConstructTypes<InputTypes...>())
      LOG(FATAL) << "Incorrect output type of evaluation";

    CHECK_NULL(output_node);
    if (output_node->output_types != ConstructTypes<OutputTypes...>())
      LOG(FATAL) << "Incorrect output type of evaluation";

    if (conversions.size() < 1) LOG(FATAL) << "No conversions in graph";

    if (input_node != conversions.front() || output_node != conversions.back())
      LOG(FATAL) << "Graph incorrectly constructed";

    input_node->Invoke(input);
    std::shared_ptr<uint8_t> last_result = conversions.front()->output_storage;
    for (int i = 1; i < conversions.size(); ++i) {
      auto& conversion = conversions[i];
      last_result = conversion->InvokeOpaque(last_result).ResultOpaque();
    }
    output = output_node->Result<OutputTypes...>();
  }

  template <typename OutputTuple, typename InputTuple>
  OutputTuple Evaluate(InputTuple input) {
    LOG(INFO) << "Evaluate(InputTuple)";
    OutputTuple output;
    Evaluate(input, output);
    return output;
  }

  std::shared_ptr<Conversion> input_node, output_node;
  std::vector<std::shared_ptr<Conversion>> conversions = {};
};

using ConversionPtr = std::shared_ptr<Conversion>;
using ConversionSearchRecord =
    std::pair<ConversionPtr, std::list<ConversionPtr>>;

template <typename T>
std::ostream& operator<<(std::ostream& os, std::shared_ptr<T> value) {
  return os << *value.get();
}

std::ostream& operator<<(std::ostream& os,
                         const std::list<Conversion>& conversions) {
  for (auto iter = conversions.begin(); iter != conversions.end(); ++iter) {
    os << *iter << ", ";
  }
  return os;
}

std::ostream& operator<<(
    std::ostream& os,
    const std::list<std::shared_ptr<Conversion>>& conversions) {
  for (auto iter = conversions.begin(); iter != conversions.end(); ++iter) {
    os << *iter->get() << ", ";
  }
  return os;
}

std::ostream& operator<<(
    std::ostream& os,
    const std::vector<std::shared_ptr<Conversion>>& conversions) {
  for (auto iter = conversions.begin(); iter != conversions.end(); ++iter) {
    os << *iter->get() << ", ";
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
  return os << "Graph(input_node = " << graph.input_node
            << ", output_node = " << graph.output_node
            << ", conversions = " << graph.conversions << ")";
}

void PrintStack(const std::list<ConversionSearchRecord>& stack,
                std::string_view prefix = "") {
  LOG(INFO) << "Printing stack (" << prefix
            << "): stack.size() = " << stack.size();
  for (const auto entry : stack) {
    LOG(INFO) << prefix << entry.first << " -> " << entry.second;
  }
}

class ComputeGraph {
 public:
  template <typename InputTuple, typename OutputTuple>
  void DeclareConversion(
      std::string name,
      std::function<OutputTuple(InputTuple)> conversion_function) {
    auto conversion = std::make_shared<Conversion>(name, conversion_function);

    InsertConversion(conversion);
  }

  Graph Construct(absl::string_view graph_spec) {
    Graph graph;
    graph.input_node = graph.output_node =
        conversions_by_name_[std::string(graph_spec)];
    graph.conversions.push_back(graph.input_node);
    return graph;
  }

  Graph BridgeHelper(const std::vector<Type>& input_types,
                     const std::vector<Type>& output_types) {
    LOG(INFO) << "BridgeHelper(input_types = " << input_types
              << ", output_types = " << output_types << ")";
    // Very slow brute-force algorithm:
    //
    // 0. Assign S = input_types
    // 1. Iterate over all conversions from S -> B
    //     1'. If conversion from S -> output_types exists, return
    // 2. Stack each candidate S -> B, along with all conversions B -> C.
    // 3. Pop conversion from list in top element, assign to S
    //     3'. If no element in list, pop entire entry and pop element in next
    // 4. GOTO 1
    std::list<ConversionSearchRecord> stack{};

    PrintStack(stack, "Before init");
    PrintStack(stack);

    auto first_layer = conversions_by_input_[input_types];
    stack.emplace_back(nullptr,
                       std::list(first_layer.begin(), first_layer.end()));
    PrintStack(stack, "Before loop");

    do {
      PrintStack(stack, "Loop entry: ");
      if (stack.empty()) return {};

      // If the topmost conversion at the top of the stack converts to the
      // target types, then construct the graph and return it.
      if (stack.back().second.back()->output_types == output_types) {
        Graph graph;
        for (auto entry : stack) {
          if (entry.first == nullptr) continue;
          graph.conversions.push_back(entry.first);
        }
        graph.conversions.push_back(stack.back().second.back());
        graph.input_node = graph.conversions.front();
        graph.output_node = graph.conversions.back();

        LOG(INFO) << "SUCCESS! Graph = " << graph;
        return graph;
      }

      // When the stack top has no remaining conversions to try, pop it.
      if (stack.back().second.empty()) {
        stack.pop_back();
        continue;
      }

      auto conversion = stack.back().second.back();
      stack.back().second.pop_back();
      auto next_conversions = conversions_by_input_[conversion->output_types];
      stack.push_back(std::make_tuple(
          conversion,
          std::list(next_conversions.begin(), next_conversions.end())));

    } while (stack.back().first == nullptr ||
             stack.back().first->output_types != output_types);

    return {};
  }

  // Constructs a graph that transforms `InputTypes` into `OutputTypes`. If no
  // such transformation is possible, returns `false`.
  template <typename... InputTypes, typename... OutputTypes>
  Graph Bridge(const std::tuple<InputTypes...>&, std::tuple<OutputTypes...>&) {
    const auto input_types = ConstructTypes<InputTypes...>();
    const auto output_types = ConstructTypes<OutputTypes...>();

    return BridgeHelper(input_types, output_types);
  }

  template <typename OutputTuple, typename InputTuple>
  void OrganizeAndEvaluate(InputTuple input, OutputTuple& output) {
    Graph graph = Bridge(input, output);
    graph.Evaluate(input, output);
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
