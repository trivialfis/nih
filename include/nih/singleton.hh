#ifndef _SINGLETON_HH_
#define _SINGLETON_HH_

namespace nih {

template <typename Type>
class Singleton {
 public:
  static Type& ins() {
    static Type instance;
    return instance;
  }
};

}      // namespace nih

#endif  // _SINGLETON_HH_