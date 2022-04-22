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

std::ostream& operator<<(std::ostream& os, const Graph& graph) {
  return os << "\nGraph(\n    .input_tuple = " << graph.InputTypes()
            << ",\n    .output_tuple = " << graph.OutputTypes() << ")";
}

void PrintStack(const std::list<ConversionSearchRecord>& stack,
                std::string_view prefix) {
  LOG(DEBUG) << "Printing stack (" << prefix
             << "): stack.size() = " << stack.size();
  for (const auto entry : stack) {
    LOG(DEBUG) << prefix << entry.first << " -> " << entry.second;
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
  return os << "\nNodePortIndex(\n    .node = " << *node
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

void AliasHelper(NodePortIndex value_in, NodePortIndex alias) {
  CHECK(!(NodePortIndexIsNullptr(value_in, kOutput) &&
          NodePortIndexIsNullptr(alias, kInput)))
      << "Attempting to alias two null values";
  auto& [alias_node, alias_index] = alias;
  auto& [value_in_node, value_in_index] = value_in;

  if (NodePortIndexIsNullptr(value_in, kOutput)) {
    value_in_node->output_tuple.Alias(value_in_index, alias_node->input_tuple,
                                      alias_index);
  } else {
    alias_node->input_tuple.Alias(alias_index, value_in_node->output_tuple,
                                  value_in_index);
  }

  Edge edge{.value_in = value_in, .alias = alias};
  alias_node->input_edges.push_back(edge);
  value_in_node->output_edges.push_back(edge);
}

void PrintState(
    const std::vector<NodePortIndex>& unsatisfied,
    const std::map<Type, std::vector<NodePortIndex>>& available_by_type) {
  LOG(DEBUG) << "unsatisfied:";
  for (const auto& port_index : unsatisfied) {
    LOG(DEBUG) << "    " << port_index;
  }

  for (const auto& [type, available] : available_by_type) {
    LOG(DEBUG) << "available_by_type[" << type << "]:";
    for (const auto& port_index : available) LOG(DEBUG) << "    " << port_index;
  }
}

absl::StatusOr<Graph> GraphBuilder::Bridge(
    const std::vector<Type>& input_types,
    const std::vector<Type>& output_types) {
  LOG(DEBUG) << "Bridge(input_types = " << input_types
             << ", output_types = " << output_types << ")";

  // List of node port inputs which are unsatisfied in the graph.
  std::vector<NodePortIndex> unsatisfied{};

  // List of node port outputs which are present in the graph (available to be
  // linked up with).
  std::map<Type, std::vector<NodePortIndex>> available_by_type{};

  // TODO: Make it possible to construct storage for a type with just the Type::
  // value as input. This will alleviate the need for us to consider aliasing in
  // both directions when hooking up the graph input ports.
  Graph graph(input_types, output_types);

  // Populate `unsatisfied` and `available_by_type` with the node ports from the
  // empty graph.
  for (int i = 0; i < graph.io_node->InputSize(); ++i) {
    unsatisfied.push_back(graph.io_node->PortIndex(i));
  }
  for (int i = 0; i < graph.io_node->OutputSize(); ++i) {
    available_by_type[graph.io_node->output_tuple.Types()[i]].push_back(
        graph.io_node->PortIndex(i));
  }

  LOG(DEBUG) << "Before loop";
  PrintState(unsatisfied, available_by_type);

  do {
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

    LOG(DEBUG) << "Trying to satisfy type " << to_satisfy_type;
    if (Contains(available_by_type, to_satisfy_type)) {
      NodePortIndex matching_available =
          RandomPick<NodePortIndex>(available_by_type[to_satisfy_type]);
      LOG(DEBUG) << "Found existing available value: " << matching_available;
      if (NodePortIndexIsNullptr(matching_available, kOutput)) {
        // The available output does not have storage. Check to see if the input
        // has storage; if it does, alias it. If it doesn't, see if it can be
        // constructed and alias it. Otherwise, drop through to looking for a
        // conversion to produce it.
        //
        // Note: becase Graph currently uses a dummy `io_node` for the
        // input/output ports, this algorithm will prevent passthrough
        // connections from occurring (connections directly between graph
        // input/output that do not pass through a node).
        if (!NodePortIndexIsNullptr(to_satisfy, kInput)) {
          // The input has storage. Alias it.
          AliasHelper(matching_available, to_satisfy);
          continue;
        } else {
          // The input does not have storage. See if we can construct it.
          if (to_satisfy.first->conversion != nullptr) {
            to_satisfy.first->ConstructEmptyInputs();
            AliasHelper(matching_available, to_satisfy);
            continue;
          }
        }
        LOG(DEBUG) << "Both ports are null";
      } else {
        // The output has storage. Alias it.
        AliasHelper(matching_available, to_satisfy);
        continue;
      }
    }

    // Find a conversion that could satisfy the input, instantiate it, and
    // update the `unsatisfied` and `available_by_type` lists.
    if (Contains(conversions_by_individual_output_, to_satisfy_type)) {
      auto new_node =
          std::make_shared<Node>(RandomPick<std::shared_ptr<Conversion>>(
              conversions_by_individual_output_[to_satisfy_type]));
      graph.nodes.push_back(new_node);

      NodePortIndex matching_from_conversion = new_node->PortIndex(
          RandomIndexOf<Type>(new_node->output_tuple.Types(), to_satisfy_type));
      LOG(DEBUG) << "Satisfying " << to_satisfy_type << " with new node "
                 << *new_node << " port " << matching_from_conversion;
      AliasHelper(matching_from_conversion, to_satisfy);

      for (int i = 0; i < new_node->InputSize(); ++i) {
        unsatisfied.push_back(new_node->PortIndex(i));
      }
      for (int i = 0; i < new_node->OutputSize(); ++i) {
        available_by_type[new_node->output_tuple.Types()[i]].push_back(
            new_node->PortIndex(i));
      }
      continue;
    }

    return absl::NotFoundError(absl::StrFormat(
        "No path from %s to %s found; unable to satisfy type %s",
        ToString(input_types), ToString(output_types),
        ToString(to_satisfy_type)));
  } while (unsatisfied.size());

  return graph;
}

Type NodePortIndexType(const NodePortIndex& port_index,
                       NodePortIndexDirection dir) {
  const auto& [node, index] = port_index;

  if (dir == kOutput) {
    return node->output_tuple.Types()[index];
  }
  return node->input_tuple.Types()[index];
}

bool NodePortIndexIsNullptr(const NodePortIndex& port_index,
                            NodePortIndexDirection dir) {
  const auto& [node, index] = port_index;

  if (dir == kOutput) {
    return node->output_tuple.CellIsEmpty(index);
  }
  return node->input_tuple.CellIsEmpty(index);
}

}  // namespace opendrop
