#ifndef UTIL_CONTAINER_ALGORITHMS_H_
#define UTIL_CONTAINER_ALGORITHMS_H_

#include <cstdlib>
#include <numeric>
#include <unordered_set>
#include <vector>

#include "absl/types/span.h"
#include "util/logging/logging.h"

namespace opendrop {

template <typename T>
std::vector<T> Unique(const absl::Span<const T> elements) {
  std::unordered_set<T> seen{};
  std::vector<T> unique{};

  for (const auto& element : elements) {
    if (seen.find(element) != seen.end()) continue;
    unique.push_back(element);
    seen.insert(element);
  }

  return unique;
}

// Tests if a `container` contains `value`.
template <typename C, typename T>
bool Contains(const C& container, const T& value) {
  return container.find(value) != container.end();
}

// Returns the value from `elements` at `index % size()`.
template <typename T>
const T& ModPick(const absl::Span<const T> elements, int64_t index) {
  return elements[index % elements.size()];
}

// Returns a value from `elements` (uniformly drawn).
template <typename T>
const T& RandomPick(const absl::Span<const T> elements) {
  return ModPick<T>(elements, std::rand());
}

// Returns a value from `elements` where the probability of drawing a particular
// value is `weights`[index] / sum(`weights`).
template <typename T>
const T& WeightedPick(const absl::Span<const T> elements,
                      const absl::Span<const float> weights, float r) {
  CHECK(elements.size() == weights.size())
      << "Provided `weights` is not the same size as `elements`.";
  std::vector<float> incremental_rs{};

  incremental_rs.reserve(elements.size());

  const float total_weight =
      std::accumulate(weights.begin(), weights.end(), 0.0f);

  CHECK(total_weight > 0) << "Provided weights sum to zero.";

  {
    float incremental_r = 0.0f;
    for (const float weight : weights) {
      CHECK(weight >= 0) << "Provided weights contain negative values.";
      incremental_r += weight / total_weight;

      incremental_rs.push_back(incremental_r);
    }
  }

  for (int i = 0; i < elements.size(); ++i) {
    if (r <= incremental_rs[i]) return elements[i];
  }

  return elements.back();
}

template <typename T>
size_t RandomIndexOf(const absl::Span<const T> elements,
                     const T& element_to_find) {
  std::vector<size_t> indices{};
  for (int i = 0; i < elements.size(); ++i) {
    if (elements[i] == element_to_find) indices.push_back(i);
  }

  CHECK(indices.size() > 0) << "RandomIndexOf(): Element not found.";

  return RandomPick<size_t>(indices);
}

template <typename T>
size_t IndexOf(const absl::Span<const T> elements, const T& element_to_find) {
  const auto iter =
      std::find(elements.begin(), elements.end(), element_to_find);

  CHECK(iter != elements.end()) << "IndexOf(): Element not found.";

  return iter - elements.begin();
}

}  // namespace opendrop

#endif  // UTIL_CONTAINER_ALGORITHMS_H_
