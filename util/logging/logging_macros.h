#ifndef UTIL_LOGGING_LOGGING_MACROS_H_
#define UTIL_LOGGING_LOGGING_MACROS_H_

#include <chrono>
#include <ctime>

#include "util/time/periodic.h"

#if defined(DISABLE_LOGGING)
// Logging is disabled. Redirect all log calls to the dummy logger.
#define LOG(level) _DummyLogger()
#else
#define __LOG_FUNCTION_COMMON(level) _Logger(level, __FILE__, __LINE__)

#if defined(ENABLE_DEBUG_LOGGING)
#define __LOG_FUNCTION_DEBUG(level) __LOG_FUNCTION_COMMON(level)
#else
#define __LOG_FUNCTION_DEBUG(level) _DummyLogger()
#endif

#define __LOG_FUNCTION_INFO(level) __LOG_FUNCTION_COMMON(level)
#define __LOG_FUNCTION_WARNING(level) __LOG_FUNCTION_COMMON(level)
#define __LOG_FUNCTION_ERROR(level) __LOG_FUNCTION_COMMON(level)
#define __LOG_FUNCTION_FATAL(level) __LOG_FUNCTION_COMMON(level)

#define LOG(level) (__LOG_FUNCTION_##level(level))
#define LOG_IF(level, condition) \
  if ((condition)) LOG(level)
#endif

#define CHECK_NULL(pointer)                      \
  (LOG(FATAL).SetCondition((pointer) == nullptr) \
   << "CHECK_NULL FAIL ((" #pointer ") == nullptr): ")
#define CHECK(expression) \
  (LOG(FATAL).SetCondition(!(expression)) << "CHECK FAIL (" #expression "): ")

#define LOG_N_SEC_IMPL(interval, level, periodic_timer_name)       \
  static Periodic<int64_t> periodic_timer_name(                    \
      static_cast<int64_t>(interval * 1000));                      \
  if (periodic_timer_name.IsDue(                                   \
          std::chrono::duration_cast<std::chrono::milliseconds>(   \
              std::chrono::system_clock::now().time_since_epoch()) \
              .count()))                                           \
  LOG(level)

#define LOG_N_SEC_CONCAT(interval, level, line) \
  LOG_N_SEC_IMPL(interval, level, __CONCAT(_lognsec_ctrl_, line))

#define LOG_N_SEC(interval, level) LOG_N_SEC_CONCAT(interval, level, __LINE__)

#endif  // UTIL_LOGGING_LOGGING_MACROS_H_
