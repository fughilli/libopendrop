#ifndef UTIL_GRAPH_GRAPH_H_
#define UTIL_GRAPH_GRAPH_H_

#include <cstdlib>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <ostream>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
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

struct Node;

using NodePortIndex = std::pair<std::shared_ptr<Node>, int>;

enum NodePortIndexDirection { kOutput, kInput };

struct Edge {
  // The node and `output_tuple` index of the value on this edge.
  NodePortIndex value_in;
  // The node and `input_tuple` index of the alias to `value_in`.
  NodePortIndex alias;
};

struct Node : public std::enable_shared_from_this<Node> {
  Node(std::shared_ptr<Conversion> conversion)
      : conversion(conversion),
        // To save on construction/destruction cost for inputs that will
        // eventually be aliased, we make the input tuple empty.
        //
        // If the input needs to be aliased to an empty source (such as the
        // `io_node.output` that is used as the input tuple for `Graph`), then
        // we can construct and fill any remaining empty inputs in the node with
        // ConstructEmptyInputs().
        input_tuple(conversion->input_factory.Empty()),
        output_tuple(conversion->output_factory.Construct()) {}

  Node(OpaqueTuple input_tuple, OpaqueTuple output_tuple)
      : conversion(nullptr),
        input_tuple(input_tuple),
        output_tuple(output_tuple) {}

  // Invokes the conversion in this node, passing `input_tuple` and storing the
  // result to `output_tuple`.
  void Invoke() { conversion->InvokeOpaque(input_tuple, output_tuple); }

  // Validates the input configuration of this node.
  bool ValidateInputEdges() const {
    if (input_edges.size() > input_tuple.Types().size()) {
      LOG(DEBUG) << absl::StrFormat(
          "ValidateInputEdges(): Too many input edges (%d > %d)",
          input_edges.size(), input_tuple.Types().size());
    }
    if (input_edges.size() != input_tuple.Types().size()) {
      LOG(DEBUG) << absl::StrFormat(
          "ValidateInputEdges(): Not all inputs are satisfied (%d of %d "
          "satisfied)",
          input_edges.size(), input_tuple.Types().size());
    }
    for (const Edge& input_edge : input_edges) {
      const auto& [value_in_node, value_in_index] = input_edge.value_in;
      const auto& [alias_node, alias_index] = input_edge.alias;

      const Type value_in_type =
          value_in_node->output_tuple.Types()[value_in_index];
      const Type alias_type = alias_node->input_tuple.Types()[alias_index];
      if (value_in_type != alias_type) {
        LOG(DEBUG) << absl::StrFormat(
            "ValidateInputEdges(): Types don't match: %s != %s",
            ToString(value_in_type), ToString(alias_type));
        return false;
      }

      if (!alias_node->input_tuple.IsAliasOf(
              alias_index, value_in_node->output_tuple, value_in_index)) {
        LOG(DEBUG) << absl::StrFormat(
            "ValidateInputEdges(): Input cell %d of type %s is not aliasing "
            "expected cell (expected alias of cell %d of type %s)",
            alias_index, ToString(alias_type), value_in_index,
            ToString(value_in_type));
        return false;
      }
    }
    return true;
  }

  // Constructs a `NodePortIndex` from this `Node` and an `index`.
  NodePortIndex PortIndex(int index) {
    return std::make_pair(shared_from_this(), index);
  }

  // Returns the size of the input tuple of this `Node`.
  size_t InputSize() const { return input_tuple.size(); }

  // Returns the size of the output tuple of this `Node`.
  size_t OutputSize() const { return output_tuple.size(); }

  void ConstructEmptyInputs() {
    CHECK(conversion != nullptr) << "ConstructEmptyInputs(): No factory to "
                                    "produce constructed inputs with";
    OpaqueTuple constructed_inputs = conversion->input_factory.Construct();

    for (int i = 0; i < InputSize(); ++i) {
      if (input_tuple.CellIsEmpty(i))
        input_tuple.Alias(i, constructed_inputs, i);
    }

    // Any inputs not aliased are discarded when `constructed_inputs` goes out
    // of scope here.
  }

  // The conversion performed by this node. The conversion is potentially shared
  // between several nodes.
  std::shared_ptr<Conversion> conversion{};

  // The storage for this node. The input OpaqueTuple will usually be entirely
  // aliases (unless it is the input for the graph).
  OpaqueTuple input_tuple, output_tuple;

  std::vector<Edge> output_edges{};
  std::vector<Edge> input_edges{};

  bool alive = false;

  void DestructEdges() {
    input_edges.clear();
    output_edges.clear();
  }
};

Type NodePortIndexType(const NodePortIndex& port_index,
                       NodePortIndexDirection dir);

bool NodePortIndexIsNullptr(const NodePortIndex& port_index,
                            NodePortIndexDirection dir);

std::ostream& operator<<(std::ostream& os, const Node& node);
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node);
std::ostream& operator<<(
    std::ostream& os, const std::unordered_set<std::shared_ptr<Node>>& nodes);
std::ostream& operator<<(std::ostream& os,
                         const std::vector<std::shared_ptr<Node>>& nodes);
std::ostream& operator<<(std::ostream& os, NodePortIndex port_index);

// Class used to store a particular graph configuration (and its results).
class InnerGraph {
 public:
  // Constructs a graph from a single conversion.
  InnerGraph(std::shared_ptr<Conversion> conversion)
      : io_node(std::make_shared<Node>(
            /*input_tuple=*/conversion->output_factory.Construct(),
            /*output_tuple=*/conversion->input_factory.Construct())) {
    nodes.push_back(std::make_shared<Node>(conversion));

    // Alias, including assigning the types.
    nodes.front()->input_tuple = io_node->output_tuple;
    io_node->input_tuple = nodes.front()->output_tuple;

    // Mark all inputs as satisfied.
    for (int i = 0; i < InputTypes().size(); ++i) {
      nodes.front()->input_edges.push_back(
          Edge{.value_in = std::make_pair(io_node, i),
               .alias = std::make_pair(nodes.front(), i)});
    }
  }

  // Constructs an empty graph.
  InnerGraph()
      : io_node(std::make_shared<Node>(OpaqueTuple{}, OpaqueTuple{})) {}

  // Constructs an empty graph with the given input/output types.
  InnerGraph(const std::vector<Type>& input_types,
             const std::vector<Type>& output_types)
      : io_node(std::make_shared<Node>(
            /*input_tuple=*/OpaqueTuple::EmptyFromTypes(output_types),
            /*output_tuple=*/OpaqueTuple::EmptyFromTypes(input_types))) {}

  template <typename... InputTypes>
  void Evaluate(std::tuple<InputTypes...> input) {
    auto input_types = ConstructTypes<InputTypes...>();
    if (this->InputTypes() != input_types)
      LOG(FATAL) << absl::StrFormat(
          "InnerGraph::Evaluate(): Incorrect input type to Evaluate: passed "
          "%s, "
          "expected %s",
          ToString(input_types), ToString(this->InputTypes()));

    if (nodes.size() < 1) LOG(FATAL) << "No nodes in graph";

    io_node->output_tuple.AssignFrom(input);

    InvokeHelper();
  }

  template <typename... OutputTypes>
  std::tuple<OutputTypes...> Result() const {
    return io_node->input_tuple.ToTuple<OutputTypes...>();
  }

  const std::vector<Type>& InputTypes() const {
    return io_node->output_tuple.Types();
  }
  const std::vector<Type>& OutputTypes() const {
    return io_node->input_tuple.Types();
  }

  // Input and output tuples of the graph. We store these in a `Node` such that
  // the validator implementation can be simplified.
  std::shared_ptr<Node> io_node{};
  // List of all nodes in the graph.
  std::vector<std::shared_ptr<Node>> nodes = {};

  void ResetEvaluationOrder() { evaluation_ordered_nodes = {}; }

  std::vector<std::shared_ptr<Node>> NodesInEvaluationOrder() const {
    return evaluation_ordered_nodes;
  }

  bool HasEmptyInputCells() const {
    for (int i = 0; i < io_node->output_tuple.size(); ++i)
      if (io_node->output_tuple.CellIsEmpty(i)) return true;
    return false;
  }

  void MarkNodesAlive() {
    LOG(INFO) << absl::StrFormat("Mark nodes alive for graph at %d",
                                 reinterpret_cast<intptr_t>(this));
    std::queue<Node*> nodes_to_visit{};
    nodes_to_visit.push(io_node.get());

    while (!nodes_to_visit.empty()) {
      Node* current = nodes_to_visit.front();
      nodes_to_visit.pop();

      current->alive = true;
      LOG(INFO) << absl::StrFormat("Node at %X is alive",
                                   reinterpret_cast<intptr_t>(current));
      for (auto& edge : current->output_edges) {
        auto& node = edge.alias.first;
        if (!node->alive) {
          nodes_to_visit.push(node.get());
        }
      }
    }
  }

  ~InnerGraph() {
    evaluation_ordered_nodes.clear();
    nodes.clear();
    LOG(INFO) << "Destructing InnerGraph";
    LOG(INFO) << "Input tuple state: " << io_node->output_tuple.StateAsString();
    LOG(INFO) << "Output tuple state: " << io_node->input_tuple.StateAsString();
  }

 private:
  std::vector<std::shared_ptr<Node>> evaluation_ordered_nodes = {};

  void MaybeDetermineEvaluationOrder() {
    if (evaluation_ordered_nodes.size() == nodes.size()) {
      LOG(DEBUG) << "Evaluation order already determined";
      return;
    }

    for (auto& node : nodes) {
      if (!node->ValidateInputEdges())
        LOG(FATAL) << "InnerGraph::Evaluate(): Graph incorrectly constructed";
    }

    std::unordered_set<std::shared_ptr<Node>> unevaluated{nodes.begin(),
                                                          nodes.end()};
    std::unordered_set<std::shared_ptr<Node>> evaluated{};
    evaluated.insert(io_node);

    auto count_unsatisfied_inputs =
        [&unevaluated](const std::shared_ptr<Node>& node) -> int {
      int unsatisfied = 0;
      for (Edge edge : node->input_edges) {
        if (unevaluated.count(edge.value_in.first) != 0) ++unsatisfied;
      }
      LOG(DEBUG) << "Node " << *node << " has " << unsatisfied
                 << " unsatisfied inputs";
      return unsatisfied;
    };
    auto count_satisfied_inputs =
        [&evaluated](const std::shared_ptr<Node>& node) -> int {
      int satisfied = 0;
      for (Edge edge : node->input_edges) {
        if (evaluated.count(edge.value_in.first) != 0) ++satisfied;
      }
      LOG(DEBUG) << "Node " << *node << " has " << satisfied
                 << " satisfied inputs";
      return satisfied;
    };

    while (!unevaluated.empty()) {
      LOG(DEBUG) << "Loop";
      LOG(DEBUG) << "unevaluated = " << unevaluated;
      LOG(DEBUG) << "evaluated = " << evaluated;
      LOG(DEBUG) << "evaluation_ordered_nodes = " << evaluation_ordered_nodes;

      // Whether or not any new nodes were evaluated in a cycle.
      bool new_nodes_evaluated = false;
      std::unordered_set<std::shared_ptr<Node>> nodes_to_check{};
      do {
        new_nodes_evaluated = false;
        nodes_to_check.clear();

        // Iterate over the evaluated nodes and push any unevaluated nodes which
        // use their outputs.
        for (auto& node : evaluated) {
          for (Edge edge : node->output_edges) {
            if (evaluated.count(edge.alias.first) == 0) {
              LOG(DEBUG) << "Found unevaluated node: " << edge.alias.first;
              nodes_to_check.insert(edge.alias.first);
            }
          }
        }

        // Find nodes in nodes_to_check which have all of their inputs ready.
        for (auto& node : nodes_to_check) {
          LOG(DEBUG) << "Checking readiness of node " << node;
          bool all_inputs_evaluated = true;
          for (Edge edge : node->input_edges) {
            LOG(DEBUG) << "Checking reverse edge to node "
                       << edge.value_in.first;
            if (evaluated.count(edge.value_in.first) == 0)
              all_inputs_evaluated = false;
          }

          LOG(DEBUG) << "Node " << node << " is "
                     << (all_inputs_evaluated ? "ready" : "not ready")
                     << " to be evaluated";

          if (all_inputs_evaluated) {
            unevaluated.erase(node);
            evaluated.insert(node);
            evaluation_ordered_nodes.push_back(node);
            new_nodes_evaluated = true;
          }
        }
        LOG(DEBUG) << "New nodes "
                   << (new_nodes_evaluated ? "were" : "were not")
                   << " evaluated this cycle.";

        LOG(DEBUG) << "unevaluated = " << unevaluated;
        LOG(DEBUG) << "evaluated = " << evaluated;
        LOG(DEBUG) << "evaluation_ordered_nodes = " << evaluation_ordered_nodes;
      } while (new_nodes_evaluated);
      LOG(DEBUG) << "Evaluated as many nodes as possible";

      if (unevaluated.empty()) break;

      // Evaluate the node with the least number of its inputs unsatisfied, and
      // at least one input satisfied.
      //
      // Conjecture: this will always be because of a cycle in the graph.
      std::vector<std::shared_ptr<Node>> unevaluated_sorted_by_satisfied{
          unevaluated.begin(), unevaluated.end()};
      std::sort(
          unevaluated_sorted_by_satisfied.begin(),
          unevaluated_sorted_by_satisfied.end(),
          [&count_unsatisfied_inputs](const std::shared_ptr<Node>& a,
                                      const std::shared_ptr<Node>& b) -> bool {
            return count_unsatisfied_inputs(a) < count_unsatisfied_inputs(b);
          });

      LOG(DEBUG) << "Unevaluated nodes sorted by satisfaciton: "
                 << unevaluated_sorted_by_satisfied;
      auto candidate_node = unevaluated_sorted_by_satisfied.front();
      LOG(DEBUG) << "Candidate node " << candidate_node << " has "
                 << count_satisfied_inputs(candidate_node)
                 << " inputs satisfied, "
                 << count_unsatisfied_inputs(candidate_node)
                 << " inputs unsatisfied.";
      if (count_satisfied_inputs(candidate_node) == 0) {
        LOG(DEBUG) << "Failed to determine evaluation order; maybe the graph "
                      "is ill-formed?";
        return;
      }

      evaluation_ordered_nodes.push_back(candidate_node);
      evaluated.insert(candidate_node);
      unevaluated.erase(candidate_node);
    }

    LOG(DEBUG) << "MaybeDetermineEvaluationOrder(): evaluation_ordered_nodes = "
               << evaluation_ordered_nodes;
  }

  void InvokeHelper() {
    MaybeDetermineEvaluationOrder();

    for (auto& node : evaluation_ordered_nodes) node->Invoke();
  }
};

class Graph {
 public:
  Graph() : inner_graph_(std::make_shared<InnerGraph>()) {}
  explicit Graph(std::shared_ptr<InnerGraph> inner_graph)
      : inner_graph_(inner_graph) {}

  template <typename... InputTypes>
  void Evaluate(std::tuple<InputTypes...> input) {
    inner_graph_->Evaluate<InputTypes...>(input);
  }

  template <typename... OutputTypes>
  std::tuple<OutputTypes...> Result() const {
    return inner_graph_->Result<OutputTypes...>();
  }

  std::vector<std::shared_ptr<Node>> NodesInEvaluationOrder() const {
    return inner_graph_->NodesInEvaluationOrder();
  }

  const std::shared_ptr<Node> io_node() const { return inner_graph_->io_node; }

  const std::vector<std::shared_ptr<Node>>& nodes() const {
    return inner_graph_->nodes;
  }

  bool valid() const { return inner_graph_ != nullptr; }

 private:
  std::shared_ptr<InnerGraph> inner_graph_;
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
std::ostream& operator<<(std::ostream& os, const InnerGraph& graph);
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
  template <typename OutputTuple>
  void DeclareProduction(std::string name,
                         std::function<OutputTuple()> conversion_function) {
    auto conversion = std::make_shared<Conversion>(name, conversion_function);

    InsertConversion(conversion);
  }

  Graph Construct(absl::string_view graph_spec) {
    return ConstructGraphHelper(conversions_by_name_[std::string(graph_spec)]);
  }

  // Constructs a graph that transforms `InputTypes` into `OutputTypes`. If no
  // such transformation is possible, returns `false`.
  absl::StatusOr<Graph> Bridge(const std::vector<Type>& input_types,
                               const std::vector<Type>& output_types);

  void MaybeGc() {
    for (auto& node : nodes_) {
      node->alive = false;
    }

    for (auto iter = graphs_.begin(); iter != graphs_.end();) {
      if (iter->use_count() == 1) {
        iter = graphs_.erase(iter);
        LOG(INFO) << "Destructed graph";
        continue;
      }

      (*iter)->MarkNodesAlive();
      ++iter;
    }

    for (auto iter = nodes_.begin(); iter != nodes_.end();) {
      if (!(*iter)->alive) {
        (*iter)->DestructEdges();
        iter = nodes_.erase(iter);
        LOG(INFO) << "Destructed node";
        continue;
      }

      ++iter;
    }
  }

  void PrintState() const {
    LOG_N_SEC(1.0, INFO) << "graphs_.size() = " << graphs_.size()
                         << ", nodes_.size() = " << nodes_.size();
  }

 private:
  using ConversionVector = std::vector<std::shared_ptr<Conversion>>;

  template <typename... Ts>
  Graph ConstructGraphHelper(Ts&&... ts) {
    std::shared_ptr<InnerGraph> inner_graph =
        std::make_shared<InnerGraph>(std::forward<Ts>(ts)...);
    graphs_.push_back(inner_graph);
    return Graph(inner_graph);
  }

  void InsertConversion(std::shared_ptr<Conversion> conversion);

  std::list<std::shared_ptr<InnerGraph>> graphs_;
  std::set<std::shared_ptr<Node>> nodes_;

  std::map<std::string, std::shared_ptr<Conversion>> conversions_by_name_;
  std::map<std::vector<Type>, ConversionVector> conversions_by_input_;
  std::map<Type, ConversionVector> conversions_by_individual_input_;
  std::map<std::vector<Type>, ConversionVector> conversions_by_output_;
  std::map<Type, ConversionVector> conversions_by_individual_output_;
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_GRAPH_H_
