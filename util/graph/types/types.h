#ifndef UTIL_GRAPH_TYPES_TYPES_H_
#define UTIL_GRAPH_TYPES_TYPES_H_

#include <list>
#include <ostream>
#include <vector>

namespace opendrop {

enum class Type {
  kFloatGeneric,
  kUnitary,
  kMonotonic,
  kTexture,
  kInteger,
  kColor,
};

// Returns the `Type` value that corresponds to a given actual type.
//
// e.g., `ToType<int>() -> Type::kInteger`.
//
// Additional specializations are defined in per-type headers.
template <typename T>
Type ToType();

//
// Specializations for native types.
//

// Returns `Type::kInteger`.
template <>
Type ToType<int>();
template <>
Type ToType<int&>();

// Returns `Type::kFloat`.
template <>
Type ToType<float>();
template <>
Type ToType<float&>();

namespace types_internal {
template <typename First>
void ConstructTypesHelper(std::vector<Type>& v) {
  v.push_back(ToType<First>());
}

template <typename First, typename Second, typename... Remaining>
void ConstructTypesHelper(std::vector<Type>& v) {
  v.push_back(ToType<First>());
  ConstructTypesHelper<Second, Remaining...>(v);
}
}  // namespace types_internal

//
// ConstructTypes() and ConstructTypesFromTuple() are utility functions for
// getting type specifications for conversions.
//
// ConstructTypes() is invoked with the types (as typename) provided in the
// template arguments, and returns a std::vector containing the corresponding
// Type values in the same order.
//
//   auto types = ConstructTypes<Monotonic, Unitary>();
//   // After the above, types = {Type::kMonotonic, Type::kUnitary}
//
// ConstructTypesFromTuple() is invoked with  a std::tuple of type instances and
// returns a std::vector containing the corresponding Type values in the same
// order.
//
//   auto types = ConstructTypesFromTuple(std::tuple<Texture, Monotonic>());
//   // After the above, types = {Type::kTexture, Type::kMonotonic}
//

// Constructs a vector from the `Type` values provided in the template args.
template <typename Arg, typename... Args>
std::vector<Type> ConstructTypes();

// Returns an empty vector (base case).
std::vector<Type> ConstructTypes();

// Same as ConstructTypes(), but takes a tuple of type instances.
template <typename Arg, typename... Args>
std::vector<Type> ConstructTypesFromTuple(const std::tuple<Arg, Args...>&);

// Returns an empty vector (base case).
std::vector<Type> ConstructTypesFromTuple(const std::tuple<>&);


// std::ostream::operator<<() overloads for `Type` et al.
std::ostream& operator<<(std::ostream& os, Type type);
std::ostream& operator<<(std::ostream& os, const std::vector<Type>& types);
std::ostream& operator<<(std::ostream& os, const std::list<Type>& types);

//
// Implementation.
//

template <typename T>
Type ToType() {
  return std::remove_reference_t<T>::kType;
}

template <typename Arg, typename... Args>
std::vector<Type> ConstructTypes() {
  std::vector<Type> v;
  v.reserve(sizeof...(Args) + 1);
  types_internal::ConstructTypesHelper<Arg, Args...>(v);
  return v;
}

template <typename Arg, typename... Args>
std::vector<Type> ConstructTypesFromTuple(const std::tuple<Arg, Args...>&) {
  return ConstructTypes<Arg, Args...>();
}

}  // namespace opendrop

#endif  // UTIL_GRAPH_TYPES_TYPES_H_
