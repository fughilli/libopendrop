#include "util/graph/graph.h"

#include "util/container/algorithms.h"

namespace opendrop {

std::ostream& operator<<(std::ostream& os,
                         const std::list<Conversion>& conversions) {
  os << "\n";
  for (auto iter = conversions.begin(); iter != conversions.end(); ++iter) {
    os << *iter << ",\n";
  }
  return os;
}

std::ostream& operator<<(
    std::ostream& os,
    const std::list<std::shared_ptr<Conversion>>& conversions) {
  os << "\n";
  for (auto iter = conversions.begin(); iter != conversions.end(); ++iter) {
    os << *iter->get() << ",\n";
  }
  return os;
}

std::ostream& operator<<(
    std::ostream& os,
    const std::vector<std::shared_ptr<Conversion>>& conversions) {
  os << "\n";
  for (auto iter = conversions.begin(); iter != conversions.end(); ++iter) {
    os << *iter->get() << ",\n";
  }
  return os;
}

template <typename Iter>
std::ostream& OutputNodes(std::ostream& os, Iter begin, Iter end) {
  os << "\n";
  while (begin != end) {
    os << *begin << ",\n";
    ++begin;
  }
  return os;
}

std::ostream& operator<<(
    std::ostream& os, const std::unordered_set<std::shared_ptr<Node>>& nodes) {
  return OutputNodes(os, nodes.begin(), nodes.end());
}
std::ostream& operator<<(std::ostream& os,
                         const std::vector<std::shared_ptr<Node>>& nodes) {
  return OutputNodes(os, nodes.begin(), nodes.end());
}

std::ostream& operator<<(std::ostream& os, const InnerGraph& graph) {
  return os << "\nInnerGraph(\n    .input_tuple = " << graph.InputTypes()
            << ",\n    .output_tuple = " << graph.OutputTypes() << ")";
}

void PrintStack(const std::list<ConversionSearchRecord>& stack,
                std::string_view prefix) {
  LOG(INFO) << "Printing stack (" << prefix
             << "): stack.size() = " << stack.size();
  for (const auto& entry : stack) {
    LOG(INFO) << prefix << entry.first << " -> " << entry.second;
  }
}

std::ostream& operator<<(std::ostream& os, const Node& node) {
  return os << "\nNode(\n    .conversion = " << node.conversion
            << ",\n    .input_tuple = " << node.input_tuple
            << ", \n    .output_tuple = " << node.output_tuple << ")";
}
std::ostream& operator<<(std::ostream& os, const std::shared_ptr<Node>& node) {
  return os << *node;
}
std::ostream& operator<<(std::ostream& os, NodePortIndex port_index) {
  const auto& [node, index] = port_index;
  return os << "\nNodePortIndex(\n    .node = " << *node.lock()
            << ",\n    .index = " << index << ")";
}

void GraphBuilder::InsertConversion(std::shared_ptr<Conversion> conversion) {
  if (conversions_by_name_.find(conversion->name) != conversions_by_name_.end())
    LOG(FATAL) << "Duplicate conversion found for name " << conversion->name;

  conversions_by_name_[conversion->name] = conversion;

  // Register mappings from full input/output types to conversions.
  conversions_by_input_[conversion->InputTypes()].push_back(conversion);
  conversions_by_output_[conversion->OutputTypes()].push_back(conversion);

  // Register mappings from individual input/output types to conversions.
  for (const Type input_type :
       Unique(absl::Span<const Type>(conversion->InputTypes())))
    conversions_by_individual_input_[input_type].push_back(conversion);
  for (const Type output_type :
       Unique(absl::Span<const Type>(conversion->OutputTypes())))
    conversions_by_individual_output_[output_type].push_back(conversion);
}

bool InnerGraph::BuildEdgeHelper(NodePortIndex in, NodePortIndex out) {
  auto [out_node, out_index] = std::make_tuple(out.node.lock(), out.index);
  auto [in_node, in_index] = std::make_tuple(in.node.lock(), in.index);

  if (NodePortIndexIsNullptr(in, kOutput) &&
      NodePortIndexIsNullptr(out, kInput)) {
    LOG(INFO) << "Cannot build edge without storage";
    return false;
  }

  if (NodePortIndexIsNullptr(in, kOutput)) {
    in_node->output_tuple.Alias(in_index, out_node->input_tuple, out_index);
  } else {
    out_node->input_tuple.Alias(out_index, in_node->output_tuple, in_index);
  }

  Edge edge{.in = in, .out = out};
  out_node->input_edges.push_back(edge);
  in_node->output_edges.push_back(edge);
  return true;
}

void PrintSearchState(
    const std::vector<NodePortIndex>& unsatisfied,
    const std::map<Type, std::vector<NodePortIndex>>& available_by_type) {
  LOG(INFO) << "unsatisfied:";
  for (const auto& port_index : unsatisfied) {
    LOG(INFO) << "    " << port_index;
  }

  for (const auto& [type, available] : available_by_type) {
    LOG(INFO) << "available_by_type[" << type << "]:";
    for (const auto& port_index : available) LOG(INFO) << "    " << port_index;
  }
}

struct Choice {
  std::list<std::shared_ptr<uint8_t>> new_node_storage{};
  std::shared_ptr<Node> new_node = nullptr;
  NodePortIndex port_index;
};

bool Satisfy(InnerGraph& graph, NodePortIndex to_satisfy, Choice choice,
             std::vector<NodePortIndex>& unsatisfied,
             std::map<Type, std::vector<NodePortIndex>>& available_by_type) {
  if (!graph.BuildEdgeHelper(choice.port_index, to_satisfy)) return false;

  if (choice.new_node == nullptr) return true;

  auto& new_node = choice.new_node;

  graph.nodes.push_back(new_node);
  graph.opaque_storage.splice(graph.opaque_storage.end(),
                              choice.new_node_storage);

  for (int i = 0; i < new_node->InputSize(); ++i) {
    unsatisfied.push_back(new_node->PortIndex(i));
  }
  for (int i = 0; i < new_node->OutputSize(); ++i) {
    available_by_type[new_node->output_tuple.Types()[i]].push_back(
        new_node->PortIndex(i));
  }
  return true;
}

absl::StatusOr<Graph> GraphBuilder::Bridge(
    const std::vector<Type>& input_types,
    const std::vector<Type>& output_types) {
  LOG(INFO) << "=========================================================";
  LOG(INFO) << "Bridge(input_types = " << input_types
             << ", output_types = " << output_types << ")";
  LOG(INFO) << "=========================================================";

  // List of node port inputs which are unsatisfied in the graph.
  std::vector<NodePortIndex> unsatisfied{};

  // List of node port outputs which are present in the graph (available to be
  // linked up with).
  std::map<Type, std::vector<NodePortIndex>> available_by_type{};

  // TODO: Make it possible to construct storage for a type with just the Type::
  // value as input. This will alleviate the need for us to consider aliasing in
  // both directions when hooking up the graph input ports.
  std::shared_ptr<InnerGraph> graph =
      std::make_shared<InnerGraph>(input_types, output_types);

  // Populate `unsatisfied` and `available_by_type` with the node ports from the
  // empty graph.
  for (int i = 0; i < graph->output_node->InputSize(); ++i) {
    auto port_index = graph->output_node->PortIndex(i);
    unsatisfied.push_back(port_index);
  }
  for (int i = 0; i < graph->input_node->OutputSize(); ++i) {
    available_by_type[graph->input_node->output_tuple.Types()[i]].push_back(
        graph->input_node->PortIndex(i));
  }

  LOG(INFO) << "Before loop";
  PrintSearchState(unsatisfied, available_by_type);

  while (!unsatisfied.empty()) {
    // TODO: Fill entire tuples at once, if a matching conversion exists.
    //
    // if (Contains(conversions_by_output_, output_types)) {
    //   auto conversion =
    //       RandomPick(absl::Span<const std::shared_ptr<Conversion>>(
    //           conversions_by_output_[output_types]));
    //   auto node = std::make_shared<Node>(conversion);
    // }
    NodePortIndex to_satisfy = unsatisfied.back();
    unsatisfied.pop_back();

    const Type to_satisfy_type = NodePortIndexType(to_satisfy, kInput);

    LOG(INFO) << "Trying to satisfy type " << to_satisfy_type;

    std::vector<Choice> choices{};
    if (Contains(available_by_type, to_satisfy_type)) {
      NodePortIndex matching_available =
          RandomPick<NodePortIndex>(available_by_type[to_satisfy_type]);
      LOG(INFO) << "Found existing available value: " << matching_available;
      if (!graph->HasEmptyInputCells() ||
          matching_available.node.lock() == graph->input_node)
        choices.push_back({.port_index = matching_available});
    }

    // Find a conversion that could satisfy the input, instantiate it, and
    // update the `unsatisfied` and `available_by_type` lists.
    if (Contains(conversions_by_individual_output_, to_satisfy_type)) {
      auto conversion = RandomPick<std::shared_ptr<Conversion>>(
          conversions_by_individual_output_[to_satisfy_type]);
      auto [output_storage, output_tuple] =
          conversion->ConstructOutputStorageAndTuple();
      auto new_node = std::make_shared<Node>(
          /*conversion=*/conversion, /*input_tuple=*/
          OpaqueTuple::EmptyFromTypes(conversion->input_types),
          /*output_tuple=*/output_tuple);

      NodePortIndex matching_from_conversion = new_node->PortIndex(
          RandomIndexOf<Type>(new_node->output_tuple.Types(), to_satisfy_type));
      LOG(INFO) << "Satisfying " << to_satisfy_type << " with new node "
                 << *new_node << " port " << matching_from_conversion;
      choices.push_back({.new_node_storage = output_storage,
                         .new_node = new_node,
                         .port_index = matching_from_conversion});
    }

    if (!choices.empty()) {
      if (Satisfy(*graph.get(), to_satisfy, RandomPick<Choice>(choices),
                  unsatisfied, available_by_type))
        continue;
    }

    return absl::NotFoundError(absl::StrFormat(
        "No path from %s to %s found; unable to satisfy type %s",
        ToString(input_types), ToString(output_types),
        ToString(to_satisfy_type)));
  }

  graphs_.push_back(graph);
  for (auto& node : graph->nodes) {
    nodes_.insert(node);
  }
  MaybeGc();

  return Graph(graph);
}

Type NodePortIndexType(const NodePortIndex& port_index,
                       NodePortIndexDirection dir) {
  const auto [node, index] =
      std::make_tuple(port_index.node.lock(), port_index.index);

  if (dir == kOutput) {
    return node->output_tuple.Types()[index];
  }
  return node->input_tuple.Types()[index];
}

bool NodePortIndexIsNullptr(const NodePortIndex& port_index,
                            NodePortIndexDirection dir) {
  const auto [node, index] =
      std::make_tuple(port_index.node.lock(), port_index.index);

  if (dir == kOutput) {
    return node->output_tuple.CellIsEmpty(index);
  }
  return node->input_tuple.CellIsEmpty(index);
}

}  // namespace opendrop
