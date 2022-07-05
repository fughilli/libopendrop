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

struct NodePortIndex {
  std::weak_ptr<Node> node;
  int index;
};

enum NodePortIndexDirection { kOutput, kInput };

struct Edge {
  // The node and `output_tuple` index that mutates the value on this edge.
  NodePortIndex in;
  // The node and `input_tuple` index that accesses the value on this edge..
  NodePortIndex out;
};

struct Node : public std::enable_shared_from_this<Node> {
  Node(std::shared_ptr<Conversion> conversion, OpaqueTuple input_tuple,
       OpaqueTuple output_tuple)
      : conversion(conversion),
        input_tuple(input_tuple),
        output_tuple(output_tuple) {}

  // Invokes the conversion in this node, passing `input_tuple` and storing the
  // result to `output_tuple`.
  void Invoke() {
    if (conversion == nullptr) return;
    conversion->InvokeOpaque(input_tuple, output_tuple);
  }

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
      const auto& [in_node, in_index] = input_edge.in;
      const auto& [out_node, out_index] = input_edge.out;

      const Type in_type = in_node.lock()->output_tuple.Types()[in_index];
      const Type out_type = out_node.lock()->input_tuple.Types()[out_index];
      if (in_type != out_type) {
        LOG(DEBUG) << absl::StrFormat(
            "ValidateInputEdges(): Types don't match: %s != %s",
            ToString(in_type), ToString(out_type));
        return false;
      }

      if (!out_node.lock()->input_tuple.IsAliasOf(
              out_index, in_node.lock()->output_tuple, in_index)) {
        LOG(DEBUG) << absl::StrFormat(
            "ValidateInputEdges(): Input cell %d of type %s is not aliasing "
            "expected cell (expected alias of cell %d of type %s)",
            out_index, ToString(out_type), in_index, ToString(in_index));
        return false;
      }
    }
    return true;
  }

  // Constructs a `NodePortIndex` from this `Node` and an `index`.
  NodePortIndex PortIndex(int index) {
    return {.node = shared_from_this(), .index = index};
  }

  // Returns the size of the input tuple of this `Node`.
  size_t InputSize() const { return input_tuple.size(); }

  // Returns the size of the output tuple of this `Node`.
  size_t OutputSize() const { return output_tuple.size(); }

  // The conversion performed by this node. The conversion is potentially shared
  // between several nodes.
  std::shared_ptr<Conversion> conversion{};

  // The storage for this node. The input OpaqueTuple will usually be entirely
  // aliases (unless it is the input for the graph).
  OpaqueTuple input_tuple, output_tuple;

  std::vector<Edge> output_edges = {};
  std::vector<Edge> input_edges = {};

  bool alive = false;
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
  InnerGraph(std::shared_ptr<Conversion> conversion) {
    {
      auto [storage, tuple] = conversion->ConstructInputStorageAndTuple();
      input_node = std::make_shared<Node>(/*conversion=*/nullptr,
                                          /*input_tuple=*/OpaqueTuple{},
                                          /*output_tuple=*/tuple);
      opaque_storage.splice(opaque_storage.end(), storage);
    }
    {
      auto [storage, tuple] = conversion->ConstructOutputStorageAndTuple();
      output_node = std::make_shared<Node>(/*conversion=*/nullptr,
                                           /*input_tuple=*/tuple,
                                           /*output_tuple=*/OpaqueTuple{});
      opaque_storage.splice(opaque_storage.end(), storage);
    }

    nodes = {input_node,
             std::make_shared<Node>(/*conversion=*/conversion,
                                    /*input_tuple=*/input_node->output_tuple,
                                    /*output_tuple=*/output_node->input_tuple),
             output_node};

    // Build edges.
    for (int edge = 1; edge <= 2; ++edge) {
      for (int i = 0; i < nodes[edge]->input_tuple.size(); ++i) {
        nodes[edge - 1]->output_edges.push_back(
            Edge{.in = {input_node, i}, .out = {nodes[edge], i}});
        nodes[edge]->input_edges = nodes[edge - 1]->output_edges;
      }
    }
  }

  InnerGraph(absl::Span<const Type> input_types,
             absl::Span<const Type> output_types) {
    {
      auto [storage, tuple] =
          OpaqueTupleFactory::StorageAndTupleFromTypes(input_types);
      input_node = std::make_shared<Node>(/*conversion=*/nullptr,
                                          /*input_tuple=*/OpaqueTuple{},
                                          /*output_tuple=*/tuple);
      opaque_storage.splice(opaque_storage.end(), storage);
    }
    output_node = std::make_shared<Node>(
        /*conversion=*/nullptr,
        /*input_tuple=*/OpaqueTuple::EmptyFromTypes(output_types),
        /*output_tuple=*/OpaqueTuple{});

    nodes = {input_node, output_node};
  }

  // Constructs an empty graph.
  InnerGraph() {}

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

    input_node->output_tuple.AssignFrom(input);

    InvokeHelper();
  }

  template <typename... OutputTypes>
  std::tuple<OutputTypes...> Result() const {
    return output_node->input_tuple.ToTuple<OutputTypes...>();
  }

  const std::vector<Type>& InputTypes() const {
    return input_node->output_tuple.Types();
  }
  const std::vector<Type>& OutputTypes() const {
    return output_node->input_tuple.Types();
  }

  // Input and output tuples of the graph. We store these in `Node`s such that
  // the validator implementation can be simplified.
  std::shared_ptr<Node> input_node = nullptr;
  std::shared_ptr<Node> output_node = nullptr;
  // List of all nodes in the graph.
  std::vector<std::shared_ptr<Node>> nodes = {};

  std::list<std::shared_ptr<uint8_t>> opaque_storage{};

  void ResetEvaluationOrder() { evaluation_ordered_nodes = {}; }

  std::vector<std::shared_ptr<Node>> NodesInEvaluationOrder() const {
    return evaluation_ordered_nodes;
  }

  bool HasEmptyInputCells() const {
    for (int i = 0; i < input_node->output_tuple.size(); ++i)
      if (input_node->output_tuple.CellIsEmpty(i)) return true;
    return false;
  }

  void MarkNodesAlive() {
    LOG(INFO) << absl::StrFormat("Mark nodes alive for graph at %d",
                                 reinterpret_cast<intptr_t>(this));
    std::queue<Node*> nodes_to_visit{};
    nodes_to_visit.push(input_node.get());

    while (!nodes_to_visit.empty()) {
      Node* current = nodes_to_visit.front();
      nodes_to_visit.pop();

      current->alive = true;
      LOG(INFO) << absl::StrFormat("Node at %X is alive",
                                   reinterpret_cast<intptr_t>(current));
      for (auto& edge : current->output_edges) {
        auto node = edge.out.node.lock();
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
    if (input_node != nullptr)
      LOG(INFO) << "Input tuple state: "
                << input_node->output_tuple.StateAsString();
    if (output_node != nullptr)
      LOG(INFO) << "Output tuple state: "
                << output_node->input_tuple.StateAsString();
  }

  bool BuildEdgeHelper(NodePortIndex in, NodePortIndex out);

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
    evaluated.insert(input_node);

    auto count_unsatisfied_inputs =
        [&unevaluated](const std::shared_ptr<Node>& node) -> int {
      int unsatisfied = 0;
      for (Edge edge : node->input_edges) {
        if (unevaluated.count(edge.in.node.lock()) != 0) ++unsatisfied;
      }
      LOG(DEBUG) << "Node " << *node << " has " << unsatisfied
                 << " unsatisfied inputs";
      return unsatisfied;
    };
    auto count_satisfied_inputs =
        [&evaluated](const std::shared_ptr<Node>& node) -> int {
      int satisfied = 0;
      for (Edge edge : node->input_edges) {
        if (evaluated.count(edge.in.node.lock()) != 0) ++satisfied;
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
            auto out_node = edge.out.node.lock();
            if (evaluated.count(out_node) == 0) {
              LOG(DEBUG) << "Found unevaluated node: " << out_node;
              nodes_to_check.insert(out_node);
            }
          }
        }

        // Find nodes in nodes_to_check which have all of their inputs ready.
        for (auto& node : nodes_to_check) {
          LOG(DEBUG) << "Checking readiness of node " << node;
          bool all_inputs_evaluated = true;
          for (Edge edge : node->input_edges) {
            auto in_node = edge.in.node.lock();
            LOG(DEBUG) << "Checking reverse edge to node " << in_node;
            if (evaluated.count(in_node) == 0) all_inputs_evaluated = false;
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
      if (count_satisfied_inputs(candidate_node) == 0 &&
          count_unsatisfied_inputs(candidate_node) != 0) {
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

    if (evaluation_ordered_nodes.size() == nodes.size()) {
      for (auto& node : evaluation_ordered_nodes) node->Invoke();
      return;
    }

    for (auto& node : nodes) node->Invoke();
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

  const std::shared_ptr<Node> input_node() const {
    return inner_graph_->input_node;
  }
  const std::shared_ptr<Node> output_node() const {
    return inner_graph_->output_node;
  }

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
