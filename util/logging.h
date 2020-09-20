#ifndef UTIL_LOGGING_H_
#define UTIL_LOGGING_H_

#include <iostream>
#include <sstream>

#if defined(ENABLE_DEBUG_LOGGING)
constexpr static bool kDebugLoggingEnabled = true;
#else
constexpr static bool kDebugLoggingEnabled = false;
#endif

enum _LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

class _Logger {
 public:
  _Logger(_LogLevel level, const char* file, int line)
      : level_(level), file_(file), line_(line) {}
  template <typename T>
  _Logger& operator<<(const T& value) {
    if (!kDebugLoggingEnabled && level_ == DEBUG) {
      return *this;
    }
    logline_stream_ << value;
    return *this;
  }

  ~_Logger() {
    if (!kDebugLoggingEnabled && level_ == DEBUG) {
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
        WriteHeader(std::cerr);
        std::cerr << logline_stream_.str() << std::endl;
    }
  }

 private:
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
    }
    ostream << "[" << file_ << ":" << line_ << "]  ";
  }

  _LogLevel level_;
  const char* file_;
  int line_;
  std::stringstream logline_stream_;
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

#define LOG(level) (__LOG_FUNCTION_##level(level))
#endif

#endif  // UTIL_LOGGING_H_
