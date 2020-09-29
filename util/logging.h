#ifndef UTIL_LOGGING_H_
#define UTIL_LOGGING_H_

#include <cstdlib>
#include <iostream>
#include <sstream>

#if defined(ENABLE_DEBUG_LOGGING)
constexpr static bool kDebugLoggingEnabled = true;
#else
constexpr static bool kDebugLoggingEnabled = false;
#endif

enum _LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3, FATAL = 4 };

class _Logger {
 public:
  _Logger(_LogLevel level, const char* file, int line)
      : level_(level), file_(file), line_(line), condition_(true) {}
  template <typename T>
  _Logger& operator<<(const T& value) {
    if (!Enabled()) {
      return *this;
    }
    logline_stream_ << value;
    return *this;
  }

  // Sets the value of the condition for this logger instance.
  _Logger& SetCondition(bool condition) {
    condition_ = condition;
    return *this;
  }

  ~_Logger() {
    if (!Enabled()) {
      return;
    }

    switch (level_) {
      case DEBUG:
      case INFO:
        WriteHeader(std::cout);
        std::cout << logline_stream_.str() << std::endl;
        break;
      case WARNING:
      case ERROR:
      case FATAL:
        WriteHeader(std::cerr);
        std::cerr << logline_stream_.str() << std::endl;
    }

    if (level_ == FATAL) {
      abort();
    }
  }

 private:
  bool Enabled() const {
    if (!kDebugLoggingEnabled && level_ == DEBUG) {
      // If this is a debug log, and debug logging is disabled, don't log.
      return false;
    }

    // Otherwise return the condition.
    return condition_;
  }

  void WriteHeader(std::ostream& ostream) {
    switch (level_) {
      case DEBUG:
        ostream << "[DEBUG]";
        break;
      case INFO:
        ostream << "[INFO ]";
        break;
      case WARNING:
        ostream << "[WARN ]";
        break;
      case ERROR:
        ostream << "[ERROR]";
        break;
      case FATAL:
        ostream << "[FATAL]";
        break;
    }
    ostream << "[" << file_ << ":" << line_ << "]  ";
  }

  _LogLevel level_;
  const char* file_;
  int line_;
  std::stringstream logline_stream_;
  bool condition_;
};

class _DummyLogger {
 public:
  template <typename T>
  _DummyLogger& operator<<(const T& value) {
    return *this;
  }
};

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
#endif

#define CHECK_NULL(pointer)                      \
  (LOG(FATAL).SetCondition((pointer) == nullptr) \
   << "CHECK_NULL FAIL ((" #pointer ") == nullptr): ")
#define CHECK(expression) \
  (LOG(FATAL).SetCondition(!(expression)) << "CHECK FAIL (" #expression "): ")

#endif  // UTIL_LOGGING_H_
