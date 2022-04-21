/*!
 * Copyright (c) by Contributors 2020
 * \brief Implementation of Intrusive Ptr.
 */
#ifndef NIH_INTRUSIVE_PTR_H_
#define NIH_INTRUSIVE_PTR_H_

#include <atomic>
#include <cinttypes>
#include <functional>
#include <ostream>

namespace nih {
/*!
 * \brief Helper class for embedding reference counting into client objects.  See
 *        https://www.boost.org/doc/libs/1_74_0/doc/html/atomic/usage_examples.html for
 *        discussions of memory order.
 */
class IntrusivePtrCell {
 private:
  std::atomic<int32_t> _count;
  template <typename T> friend class IntrusivePtr;

  std::int32_t IncRef() noexcept {
    return _count.fetch_add(1, std::memory_order_relaxed);
  }
  std::int32_t DecRef() noexcept {
    return _count.fetch_sub(1, std::memory_order_release);
  }
  [[nodiscard]] bool IsZero() const { return Count() == 0; }

 public:
  IntrusivePtrCell() noexcept : _count{0} {}
  [[nodiscard]] int32_t Count() const { return _count.load(std::memory_order_relaxed); }
};

/*!
 * \brief User defined function for returing embedded reference count.
 */
template <typename T> IntrusivePtrCell &IntrusivePtrRefCount(T const *ptr) noexcept;

/*!
 * \brief Implementation of Intrusive Pointer.  A smart pointer that points to an object
 *        with an embedded reference counter. The underlying object must implement a
 *        friend function IntrusivePtrRefCount() that returns the ref counter (of type
 *        IntrusivePtrCell). The intrusive pointer is faster than std::shared_ptr<>:
 *        std::shared_ptr<> makes an extra memory allocation for the ref counter whereas
 *        the intrusive pointer does not.
 *
 * \code
 *
 *   class ForIntrusivePtrTest {
 *    public:
 *     mutable class IntrusivePtrCell ref;
 *     float data { 0 };
 *
 *     friend IntrusivePtrCell &
 *     IntrusivePtrRefCount(ForIntrusivePtrTest const *t) noexcept {  // NOLINT
 *       return t->ref;
 *     }
 *
 *     ForIntrusivePtrTest() = default;
 *     ForIntrusivePtrTest(float a, int32_t b) : data{a + static_cast<float>(b)} {}
 *
 *     explicit ForIntrusivePtrTest(NotCopyConstructible a) : data{a.data} {}
 *   };
 *
 *   IntrusivePtr<ForIntrusivePtrTest> ptr {new ForIntrusivePtrTest};
 *
 * \endcode
 */
template <typename T> class IntrusivePtr {
 private:
  void IncRef(T *ptr) {
    if (ptr) {
      IntrusivePtrRefCount(ptr).IncRef();
    }
  }
  void DecRef(T *ptr) {
    if (ptr) {
      if (IntrusivePtrRefCount(ptr).DecRef() == 1) {
        std::atomic_thread_fence(std::memory_order_acquire);
        delete ptr;
      }
    }
  }

 protected:
  T *_ptr{nullptr};

 public:
  using element_type = T;  // NOLINT
  struct Hash {
    std::size_t operator()(IntrusivePtr<element_type> const &ptr) const noexcept {
      return std::hash<element_type *>()(ptr.get());
    }
  };
  /*!
   * \brief Contruct an IntrusivePtr from raw pointer. IntrusivePtr takes the ownership.
   *
   * \param p Raw pointer to object
   */
  explicit IntrusivePtr(T *p) : _ptr{p} {
    if (_ptr) {
      IncRef(_ptr);
    }
  }

  IntrusivePtr() noexcept = default;
  IntrusivePtr(IntrusivePtr const &that) : _ptr{that._ptr} { IncRef(_ptr); }
  IntrusivePtr(IntrusivePtr &&that) noexcept : _ptr{that._ptr} { that._ptr = nullptr; }

  ~IntrusivePtr() { DecRef(_ptr); }

  IntrusivePtr<T> &operator=(IntrusivePtr<T> const &that) {
    IntrusivePtr<T>{that}.swap(*this);
    return *this;
  }
  IntrusivePtr<T> &operator=(IntrusivePtr<T> &&that) noexcept {
    std::swap(_ptr, that._ptr);
    return *this;
  }

  void reset() {  // NOLINT
    DecRef(_ptr);
    _ptr = nullptr;
  }
  void reset(element_type *that) { IntrusivePtr{that}.swap(*this); }  // NOLINT

  element_type &operator*() const noexcept { return *_ptr; }
  element_type *operator->() const noexcept { return _ptr; }
  element_type *get() const noexcept { return _ptr; }  // NOLINT

  explicit operator bool() const noexcept { return static_cast<bool>(_ptr); }

  int32_t use_count() noexcept {  // NOLINT
    return _ptr ? IntrusivePtrRefCount(_ptr).Count() : 0;
  }

  /*
   * \brief Helper function for swapping 2 pointers.
   */
  void swap(IntrusivePtr<T> &that) noexcept {  // NOLINT
    std::swap(_ptr, that._ptr);
  }
};

template <class T, class U>
bool operator==(IntrusivePtr<T> const &x, IntrusivePtr<U> const &y) noexcept {
  return x.get() == y.get();
}

template <class T, class U>
bool operator!=(IntrusivePtr<T> const &x, IntrusivePtr<U> const &y) noexcept {
  return x.get() != y.get();
}

template <class T, class U>
bool operator==(IntrusivePtr<T> const &x, U *y) noexcept {
  return x.get() == y;
}

template <class T, class U>
bool operator!=(IntrusivePtr<T> const &x, U *y) noexcept {
  return x.get() != y;
}

template <class T, class U>
bool operator==(T *x, IntrusivePtr<U> const &y) noexcept {
  return y == x;
}

template <class T, class U>
bool operator!=(T *x, IntrusivePtr<U> const &y) noexcept {
  return y != x;
}

template <class T>
bool operator<(IntrusivePtr<T> const &x, IntrusivePtr<T> const &y) noexcept {
  return std::less<T*>{}(x.get(), y.get());
}

template <class T>
bool operator<=(IntrusivePtr<T> const &x, IntrusivePtr<T> const &y) noexcept {
  return std::less_equal<T*>{}(x.get(), y.get());
}

template <class T>
bool operator>(IntrusivePtr<T> const &x, IntrusivePtr<T> const &y) noexcept {
  return !(x <= y);
}

template <class T>
bool operator>=(IntrusivePtr<T> const &x, IntrusivePtr<T> const &y) noexcept {
  return !(x < y);
}

template <class E, class T, class Y>
std::basic_ostream<E, T> &operator<<(std::basic_ostream<E, T> &os,
                                     IntrusivePtr<Y> const &p) {
  os << p.get();
  return os;
}
}  // namespace nih

namespace std {
template <class T>
void swap(nih::IntrusivePtr<T> &x,  // NOLINT
          nih::IntrusivePtr<T> &y) noexcept {
  x.swap(y);
}

template <typename T>
struct hash<nih::IntrusivePtr<T>> : public nih::IntrusivePtr<T>::Hash {};
}      // namespace std
#endif  // NIH_INTRUSIVE_PTR_H_
