#ifndef CB_TIME_H_
#define CB_TIME_H_

#if defined(_MSC_VER)
#include <chrono>  // NOLINT
#else
#include <sys/time.h>
#endif

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>

namespace cb {

class Timer {
 public:
  void Reset() {
    start_ = 0;
    stop_ = 0;
    total_ = 0;
    count_ = 0;
    min_ = UINT64_MAX;
    max_ = 0;
  }

  void Start() { start_ = Now(); }

  void Stop() {
    stop_ = Now();
    uint64_t d = stop_ - start_;
    total_ += d;
    ++count_;
    min_ = std::min(min_, d);
    max_ = std::max(max_, d);
  }

  inline uint64_t get() const { return stop_ - start_; }

  inline uint64_t average() const { return count_ == 0 ? 0 : total_ / count_; }

  inline uint64_t total() const { return total_; }

  inline uint64_t count() const { return count_; }

  inline uint64_t min() const { return count_ == 0 ? 0 : min_; }

  inline uint64_t max() const { return max_; }

  static uint64_t Now() {
#if defined(_MSC_VER)
    return std::chrono::duration_cast<std::chrono::microseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
#else
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
#endif
  }

  std::string str() const {
    std::stringstream ss;
    ss << "[time(ms)" << std::setprecision(4) << " last:" << get() / 1e3f
       << " ave:" << average() / 1e3f << " min:" << min() / 1e3f
       << " max:" << max() / 1e3f << " cnt:" << count() << "]";
    return ss.str();
  }

 private:
  uint64_t start_;
  uint64_t stop_;
  uint64_t total_;
  uint64_t count_;
  uint64_t min_;
  uint64_t max_;
};

class TimerFactory {
 public:
  static TimerFactory* instance() {
    static TimerFactory _instance;
    return &_instance;
  }

  Timer* get(const std::string& name) const {
    const auto timers = &instance()->timers_;
    auto iter = timers->find(name);
    if (iter == timers->end()) return nullptr;
    return &iter->second;
  }

  void Start(const std::string& name) { instance()->timers_[name].Start(); }

  void Stop(const std::string& name) { instance()->timers_[name].Stop(); }

 private:
  std::map<std::string, Timer> timers_;
};

}  // namespace cb

#ifdef ENABLE_TIMER
#define TIMER_NAME(name) const char* name = #name
#define TIMER_START(name) cb::TimerFactory::instance()->Start(name)
#define TIMER_STOP(name) cb::TimerFactory::instance()->Stop(name)
#define TIMER_PRINT(name) \
  LOG(INFO) << name << " " << cb::TimerFactory::instance()->get(name)->str()
#else
#define TIMER_NAME(name)
#define TIMER_START(name)
#define TIMER_STOP(name)
#define TIMER_PRINT(name)
#endif

#endif  // CB_TIME_H_
