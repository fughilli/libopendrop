#ifndef UTIL_STATUS_MACROS_H_
#define UTIL_STATUS_MACROS_H_

#include "absl/status/statusor.h"

namespace absl {

#define RETURN_IF_ERROR(expr)              \
  do {                                     \
    const ::absl::Status _status = (expr); \
    if (!_status.ok()) {                   \
      return _status;                      \
    }                                      \
  } while (0)

template <typename T>
::absl::Status AssignOrReturnStatus(T& value_to_assign,
                                    ::absl::StatusOr<T> status_or_value) {
  if (!status_or_value.ok()) {
    return status_or_value.status();
  }
  value_to_assign = *status_or_value;
  return ::absl::OkStatus();
}

#define ASSIGN_OR_RETURN(lhs, rhs)                                      \
  do {                                                                  \
    ::absl::Status status = ::absl::AssignOrReturnStatus((lhs), (rhs)); \
    if (!status.ok()) {                                                 \
      return status;                                                    \
    }                                                                   \
  } while (0)

}  // namespace absl

#endif  // UTIL_STATUS_MACROS_H_
