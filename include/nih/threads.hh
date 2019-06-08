#ifndef _THREADS_HH_
#define _THREADS_HH_

#include <thread>
#include <unordered_map>

namespace nih {

template <typename Type>
class ThreadStore {
  std::unordered_map<std::thread::id, Type> _store;

 public:
  Type& getCurrentThread() {
    return _store.at(std::this_thread::get_id());
  }
  Type const& getCurrentThread() const {
    return _store.at(std::this_thread::get_id());
  }
  bool hasValue(std::thread::id tid  = std::this_thread::get_id()) {
    return _store.find(tid) != _store.cend();
  }

  void setCurrentThread(Type&& value) {
    _store[std::this_thread::get_id()] = std::forward<Type>(value);
  }
};

}  // namespace nih

#endif  // _THREADS_HH_