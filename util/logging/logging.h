#ifndef UTIL_LOGGING_LOGGING_H_
#define UTIL_LOGGING_LOGGING_H_

#include <cstdlib>
#include <iostream>
#include <ostream>
#include <sstream>

#include "absl/strings/str_format.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "util/logging/logging_glm_helpers.h"
#include "util/logging/logging_helpers.h"
#include "util/logging/logging_macros.h"

#if defined(ENABLE_DEBUG_LOGGING)
constexpr static bool kDebugLoggingEnabled = true;
#else
constexpr static bool kDebugLoggingEnabled = false;
#endif

enum _LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3, FATAL = 4 };

class _Logger {
 public:
  _Logger(_LogLevel level, const char* file, int line)
      : level_(level),
        file_(file),
        line_(line),
        condition_(true),
        sink_(&std::cout),
        error_sink_(&std::cerr) {}
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

  // Sets the sink for this logger instance.
  _Logger& SetSink(std::ostream* sink) {
    sink_ = sink;
    return *this;
  }

  // Sets the error/warning level sink for this logger instance.
  _Logger& SetErrorSink(std::ostream* error_sink) {
    error_sink_ = error_sink;
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
    auto time_since_epoch = absl::Now() - absl::UnixEpoch();

    ostream << "["
            << absl::StrFormat(
                   "%d.%06d", absl::ToInt64Seconds(time_since_epoch),
                   absl::ToInt64Microseconds(time_since_epoch) % 1000000ul)
            << "]";
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
  std::ostream* sink_;
  std::ostream* error_sink_;
};

class _DummyLogger {
 public:
  template <typename T>
  _DummyLogger& operator<<(const T& value) {
    return *this;
  }
};

template<typename T>
std::string ToString(const T& t) {
  std::stringstream ss;
  ss << t;
  return ss.str();
}

#endif  // UTIL_LOGGING_LOGGING_H_
