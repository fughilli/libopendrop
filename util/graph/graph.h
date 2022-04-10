#ifndef UTIL_GRAPH_GRAPH_H_
#define UTIL_GRAPH_GRAPH_H_

#include <memory>
#include <vector>

namespace opendrop {

struct Node {
  std::vector<shared_ptr<Node>> incoming_edges = {};
};

class Graph {
 public:
 private:
  std::vector<shared_ptr<Node>> nodes_ = {};
};

}  // namespace opendrop

#endif  // UTIL_GRAPH_GRAPH_H_
