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

}  // namespace opendrop
