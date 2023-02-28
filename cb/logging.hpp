#ifndef CB_LOGGING_H_
#define CB_LOGGING_H_

#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>
#endif
#if defined(_MSC_VER)
#include <chrono>  // NOLINT
#else
#include <sys/time.h>
#endif

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>

namespace cb {
namespace logging {
// A wrapper that logs to stderr.
class LoggingWrapper {
 public:
  enum class LogSeverity : int {
    INFO = 0,
    WARN = 1,
    ERROR = 2,
    FATAL = 3,
  };

  LoggingWrapper(const char* filename, int line, LogSeverity severity)
      : severity_(severity), filename_(filename), line_(line) {}

  std::stringstream& Stream() { return stream_; }

  ~LoggingWrapper() {
    if (severity_ >= LogSeverity::INFO) {
      char tm_str[40] = {0};
      time_t now = time(nullptr);
      struct tm* tm_info = localtime(&now);
      auto len = strftime(tm_str, sizeof(tm_str), "%Y-%m-%d %H:%M:%S", tm_info);
      snprintf(tm_str + len, 8, ".%06d", GetUsec());

#if defined(_WIN32)
      char sep = '\\';
#else
      char sep = '/';
#endif
      const char* const partial_name = strrchr(filename_, sep);
      std::stringstream ss;
      ss << "IWEF"[static_cast<int>(severity_)] << ' ' << tm_str << ' '
         << (partial_name != nullptr ? partial_name + 1 : filename_) << ':'
         << line_ << "] " << stream_.str();
      std::cerr << ss.str() << std::endl;

#if defined(ANDROID) || defined(__ANDROID__)
      int android_log_level;
      switch (severity_) {
        case LogSeverity::INFO:
          android_log_level = ANDROID_LOG_INFO;
          break;
        case LogSeverity::WARN:
          android_log_level = ANDROID_LOG_WARN;
          break;
        case LogSeverity::ERROR:
          android_log_level = ANDROID_LOG_ERROR;
          break;
        case LogSeverity::FATAL:
          android_log_level = ANDROID_LOG_FATAL;
          break;
      }
      __android_log_write(android_log_level, "CB", ss.str().c_str() + 29);
#endif

      if (severity_ == LogSeverity::FATAL) {
        std::flush(std::cerr);
        std::abort();
      }
    }
  }

  static int GetUsec() {
#if defined(_MSC_VER)
    auto usec = std::chrono::duration_cast<std::chrono::microseconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count() %
                1000000;
    return static_cast<int>(usec);
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_usec;
#endif
  }

  static int VLogLevel() { return vlog_level(); }

  static void VLogSetLevel(int level) { vlog_level() = level; }

 private:
  static int& vlog_level() {
    static int _vlog_level = 0;
    return _vlog_level;
  }

  std::stringstream stream_;
  LogSeverity severity_;
  const char* filename_;
  int line_;
};

}  // namespace logging
}  // namespace cb

#define LOG(severity)                                                         \
  cb::logging::LoggingWrapper(                                                \
      __FILE__, __LINE__, cb::logging::LoggingWrapper::LogSeverity::severity) \
      .Stream()

#define LOG_IF(severity, condition) \
  if (condition) LOG(severity)

#define CHECK(condition) \
  LOG_IF(FATAL, !(condition)) << "Check failed: (" #condition ") "

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_LE(a, b) CHECK((a) <= (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_GE(a, b) CHECK((a) >= (b))
#define CHECK_GT(a, b) CHECK((a) > (b))

#define VLOG_IS_ON(verboselevel) \
  ((verboselevel) <= cb::logging::LoggingWrapper::VLogLevel())

#define VLOG_SET_LEVEL(verboselevel) \
  cb::logging::LoggingWrapper::VLogSetLevel(verboselevel)

#define VLOG(verboselevel) LOG_IF(INFO, VLOG_IS_ON(verboselevel))

#endif  // CB_LOGGING_H_
