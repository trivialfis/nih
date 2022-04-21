#include <mutex>
#include <exception>

namespace tungsten {
class OMPException {
 private:
  // exception_ptr member to store the exception
  std::exception_ptr _omp_exc;
  // mutex to be acquired during catch to set the exception_ptr
  std::mutex _lock;

 public:
  template <typename Function, typename... Parameters>
  void run(Function f, Parameters... params) {
    try {
      f(params...);
    } catch (std::runtime_error &ex) {
      std::lock_guard<std::mutex> lock(_lock);
      if (!_omp_exc) {
        _omp_exc = std::current_exception();
      }
    } catch (std::exception &ex) {
      std::lock_guard<std::mutex> lock(_lock);
      if (!_omp_exc) {
        _omp_exc = std::current_exception();
      }
    }
  }

  void rethrow() {
    if (this->_omp_exc) std::rethrow_exception(this->_omp_exc);
  }
};

enum class Schedule {
  kStatic,
  kDynamic,
  kGuided,
  kAuto
};

template <Schedule sched, typename Fn>
void parallelFor(size_t n, std::int32_t n_threads, Fn const &fn) {
  OMPException handler;
  switch (sched) {
  case Schedule::kAuto: {
#pragma omp parallel for num_threads(n_threads)
    for (size_t i = 0; i < n; ++i) {
      handler.run(fn, i);
    }
    break;
  }
  case Schedule::kStatic: {
#pragma omp parallel for num_threads(n_threads) schedule(static)
    for (size_t i = 0; i < n; ++i) {
      handler.run(fn, i);
    }
    break;
  }
  case Schedule::kDynamic: {
#pragma omp parallel for num_threads(n_threads) schedule(dynamic)
    for (size_t i = 0; i < n; ++i) {
      handler.run(fn, i);
    }
    break;
  }
  case Schedule::kGuided: {
#pragma omp parallel for num_threads(n_threads) schedule(guided)
    for (size_t i = 0; i < n; ++i) {
      handler.run(fn, i);
    }
    break;
  }
  };
  handler.rethrow();
}
}  // namespace tungsten
