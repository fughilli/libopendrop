#ifndef UTIL_STATUS_STATUS_MACROS_H_
#define UTIL_STATUS_STATUS_MACROS_H_

#include "absl/status/statusor.h"

namespace absl {

#define RETURN_IF_ERROR(expr)              \
  do {                                     \
    const ::absl::Status _status = (expr); \
    if (!_status.ok()) {                   \
      return _status;                      \
    }                                      \
  } while (0)

#define ASSIGN_OR_RETURN_IMPL(temp, lhs, rhs) \
  auto temp = (rhs);                          \
  RETURN_IF_ERROR(temp.status());             \
  lhs = std::move(temp).value()

#define ASSIGN_OR_RETURN_CONCAT(line, lhs, rhs) \
  ASSIGN_OR_RETURN_IMPL(__CONCAT(_status_or_, line), lhs, (rhs))

#define ASSIGN_OR_RETURN(lhs, rhs) ASSIGN_OR_RETURN_CONCAT(__LINE__, lhs, (rhs))

}  // namespace absl

#endif  // UTIL_STATUS_STATUS_MACROS_H_
