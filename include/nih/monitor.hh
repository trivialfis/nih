#ifndef _MONITOR_HH_
#define _MONITOR_HH_

#include <chrono>
#include <map>
#include <string>
#include <iostream>
#include <sstream>

namespace nih {
class Timer {
 public:
  using TimePoint = std::chrono::high_resolution_clock::time_point;
  using Duration = std::chrono::high_resolution_clock::duration;

 private:
  TimePoint start_;
  TimePoint end_;

 public:
  Timer() = default;
  ~Timer() = default;

  Timer tick() {
    start_ = std::chrono::high_resolution_clock::now();
    return *this;
  }
  void tock() {
    end_ = std::chrono::high_resolution_clock::now();
  }

  Duration duration() const {
    return end_ - start_;
  }

  Timer& operator+=(Timer const& that) {
    end_ += that.duration();
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, Timer t) {
    os << "Elapsed: ";
    os << std::to_string(std::chrono::duration_cast<std::chrono::microseconds>(
        t.duration()).count());
    return os;
  }
};

class GlobalMonitor {
  std::map<std::string, Timer> store_;

 public:
  void add(std::string const& name, Timer const& timer) {
    auto iter = store_.find(name);
    if (iter == store_.cend()) {
      store_[name] = timer;
    } else {
      (*iter).second += timer;
    }
  }

  std::map<std::string, Timer> getTimers() {
    return store_;
  }
  std::string toString() {
    std::stringstream stream;
    for (auto kv : store_) {
      stream << kv.first << ": " << kv.second << "\n";
    }
    return stream.str();
  }

  static GlobalMonitor& ins() {
    static GlobalMonitor instance;
    return instance;
  }
};

class TimerContext {
  Timer timer_;
  std::string name_;
  static std::map<std::string, Timer> store_;

 public:
  TimerContext(std::string name) : name_{std::move(name)} {
    timer_.tick();
  }
  ~TimerContext() {
    timer_.tock();
    GlobalMonitor::ins().add(name_, timer_);
  }
};

#define FUNC_TIMER(name)                                        \
  TimerContext __ ## name ## _timer__                           \
  (# name + std::string(" in ") + std::string{__func__});
}      // namespace nih

#endif  // _MONITOR_HH_