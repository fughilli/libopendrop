#include "util/graph/graph.h"

namespace opendrop {

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
                std::string_view prefix) {
  LOG(DEBUG) << "Printing stack (" << prefix
             << "): stack.size() = " << stack.size();
  for (const auto entry : stack) {
    LOG(DEBUG) << prefix << entry.first << " -> " << entry.second;
  }
}

void GraphBuilder::InsertConversion(std::shared_ptr<Conversion> conversion) {
  if (conversions_by_name_.find(conversion->name) != conversions_by_name_.end())
    LOG(FATAL) << "Duplicate conversion found for name " << conversion->name;

  conversions_by_name_[conversion->name] = conversion;

  if (conversions_by_input_.find(conversion->InputTypes()) ==
      conversions_by_input_.end())
    conversions_by_input_[conversion->InputTypes()] = {};
  if (conversions_by_output_.find(conversion->OutputTypes()) ==
      conversions_by_output_.end())
    conversions_by_output_[conversion->OutputTypes()] = {};

  conversions_by_input_[conversion->InputTypes()].push_back(conversion);
  conversions_by_output_[conversion->OutputTypes()].push_back(conversion);
}

absl::StatusOr<Graph> GraphBuilder::Bridge(
    const std::vector<Type>& input_types,
    const std::vector<Type>& output_types) {
  LOG(DEBUG) << "Bridge(input_types = " << input_types
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

  PrintStack(stack, "Before init: ");
  PrintStack(stack);

  auto first_layer = conversions_by_input_[input_types];
  stack.emplace_back(nullptr,
                     std::list(first_layer.begin(), first_layer.end()));
  PrintStack(stack, "Before loop: ");

  do {
    PrintStack(stack, "Loop entry: ");
    if (stack.empty()) break;

    // When the stack top has no remaining conversions to try, pop it.
    if (stack.back().second.empty()) {
      stack.pop_back();
      continue;
    }

    // If the topmost conversion at the top of the stack converts to the
    // target types, then construct the graph and return it.
    if (stack.back().second.back()->OutputTypes() == output_types) {
      Graph graph;
      for (auto entry : stack) {
        if (entry.first == nullptr) continue;
        graph.conversions.push_back(entry.first);
      }
      graph.conversions.push_back(stack.back().second.back());
      graph.input_node = graph.conversions.front();
      graph.output_node = graph.conversions.back();

      LOG(DEBUG) << "SUCCESS! Graph = " << graph;
      return graph;
    }

    auto conversion = stack.back().second.back();
    stack.back().second.pop_back();
    auto next_conversions = conversions_by_input_[conversion->OutputTypes()];
    stack.push_back(std::make_tuple(
        conversion,
        std::list(next_conversions.begin(), next_conversions.end())));

  } while (stack.back().first == nullptr ||
           stack.back().first->OutputTypes() != output_types);

  return absl::NotFoundError(absl::StrFormat("No path from %s to %s found",
                                             ToString(input_types),
                                             ToString(output_types)));
}

}  // namespace opendrop
