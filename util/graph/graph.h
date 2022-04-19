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

#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "util/graph/conversion.h"
#include "util/graph/tuple.h"
#include "util/graph/types/types.h"
#include "util/logging/logging.h"

namespace opendrop {

// Class used to store a particular graph configuration (and its results).
class Graph {
 public:
  template <typename... InputTypes>
  void Evaluate(std::tuple<InputTypes...> input) {
    CHECK_NULL(input_node);
    auto input_types = ConstructTypes<InputTypes...>();
    if (input_node->InputTypes() != input_types)
      LOG(FATAL) << absl::StrFormat(
          "Incorrect input type to Evaluate: passed %s, expected %s",
          ToString(input_types), ToString(input_node->InputTypes()));

    CHECK_NULL(output_node);

    if (conversions.size() < 1) LOG(FATAL) << "No conversions in graph";

    if (input_node != conversions.front() || output_node != conversions.back())
      LOG(FATAL) << "Graph incorrectly constructed";

    input_node->Invoke(input);
    OpaqueTuple last_result = conversions.front()->output_tuple;
    for (int i = 1; i < conversions.size(); ++i) {
      auto& conversion = conversions[i];
      last_result = conversion->InvokeOpaque(last_result).ResultOpaque();
    }
  }

  template <typename... OutputTypes>
  std::tuple<OutputTypes...> Result() const {
    return output_node->Result<OutputTypes...>();
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
                         const std::list<Conversion>& conversions);
std::ostream& operator<<(
    std::ostream& os,
    const std::list<std::shared_ptr<Conversion>>& conversions);
std::ostream& operator<<(
    std::ostream& os,
    const std::vector<std::shared_ptr<Conversion>>& conversions);
std::ostream& operator<<(std::ostream& os, const Graph& graph);
void PrintStack(const std::list<ConversionSearchRecord>& stack,
                std::string_view prefix = "");

class GraphBuilder {
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

  // Constructs a graph that transforms `InputTypes` into `OutputTypes`. If no
  // such transformation is possible, returns `false`.
  absl::StatusOr<Graph> Bridge(const std::vector<Type>& input_types,
                               const std::vector<Type>& output_types);

 private:
  using ConversionVector = std::vector<std::shared_ptr<Conversion>>;

  void InsertConversion(std::shared_ptr<Conversion> conversion);

  std::map<std::string, std::shared_ptr<Conversion>> conversions_by_name_;
  std::map<std::vector<Type>, ConversionVector> conversions_by_input_;
  std::map<std::vector<Type>, ConversionVector> conversions_by_output_;
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_GRAPH_H_
