/*!
 * Copyright (c) by Contributors 2020
 * \file intrusive_ptr.h
 * \brief Implementation of Intrusive Ptr.
 */
#ifndef NIH_INTRUSIVE_PTR_H_
#define NIH_INTRUSIVE_PTR_H_

#include <atomic>
#include <cinttypes>
#include <functional>

namespace nih {
/*!
 * \brief Helper class for embedding reference counting into client objects.
 */
class RefCount {
  std::atomic<int32_t> count_;

 public:
  RefCount() noexcept : count_{0} {}
  std::int32_t Inc() noexcept { return ++count_; }
  std::int32_t Dec() noexcept { return --count_; }
  bool IsZero() const { return count_ == 0; }
  int32_t Count() const { return count_; }
};

/*!
 * \brief User defined function for returing embedded reference count.
 */
template <typename T> RefCount &IntrusivePtrRefCount(T const *ptr) noexcept;
/*!
 * \brief User defined function for destroying object.
 */
template <typename T> void IntrusivePtrDestroy(T const *ptr) noexcept;


/*!
 * \brief Implementation of Intrusive Pointer.
 *
 * \code
 *   struct RefCountedData {
 *     mutable class RefCount ref;
 *     float data { 0 };
 *
 *     friend RefCount &
 *     IntrusivePtrRefCountRefCount(RefCountedData const *t) noexcept {
 *       return t->ref;
 *     }
 *
 *     friend void
 *     IntrusivePtrDestroy(RefCountedData const *t) noexcept {
 *       delete t;
 *     }
 *   };
 *
 *   IntrusivePtr p { new RefCountedData };
 *
 * \endcode
 */
template <typename T> class IntrusivePtr {
 private:
  void Incref(T *ptr) {
    if (ptr) {
      IntrusivePtrRefCount(ptr).Inc();
    }
  }
  void Decref(T *ptr) {
    if (ptr) {
      if (IntrusivePtrRefCount(ptr).Dec() == 0) {
        IntrusivePtrDestroy(ptr);
      }
    }
  }

 protected:
  T *ptr_{nullptr};

 public:
  using element_type = T;  // NOLINT
  struct Hash {
    std::size_t operator()(IntrusivePtr<element_type> const &ptr) const noexcept {
      return std::hash<element_type *>()(ptr.Get());
    }
  };

  IntrusivePtr() noexcept = default;

  /*!
   * \brief Contruct an IntrusivePtr.
   *
   * \param p Raw pointer to object
   * \param add_ref Whether contructing the IntrusivePtr should increase reference count.
   */
  explicit IntrusivePtr(element_type *p, bool add_ref = true) : ptr_{p} {
    if (ptr_ && add_ref) {
      Incref(ptr_);
    }
  }

  IntrusivePtr(IntrusivePtr const &that) : ptr_{that.ptr_} { Incref(ptr_); }

  template <typename U>
  explicit IntrusivePtr(IntrusivePtr<U> const &that) : ptr_{that.ptr_} {
    Incref(ptr_);
  }

  IntrusivePtr(IntrusivePtr &&that) : ptr_{that.ptr_} { that.ptr_ = nullptr; }

  template <typename U>
  explicit IntrusivePtr(IntrusivePtr<U> &&that) : ptr_{that.ptr_} {
    that.ptr_ = nullptr;
  }

  ~IntrusivePtr() { Decref(ptr_); }

  IntrusivePtr &operator=(element_type *that) {
    *this = IntrusivePtr(that);
    return *this;
  }
  IntrusivePtr &operator=(IntrusivePtr const &that) {
    if (ptr_ == that.ptr_) {
      return *this;
    }
    // that can be something owned by this.
    element_type *t = that.ptr_;
    Incref(t);
    Decref(ptr_);
    ptr_ = t;
    return *this;
  }
  template <typename U> IntrusivePtr &operator=(IntrusivePtr<U> const &that) {
    IntrusivePtr<element_type> copy{that.get()};
    *this = copy;
    return *this;
  }

  IntrusivePtr &operator=(IntrusivePtr &&that) {
    std::swap(ptr_, that.ptr_);
    return *this;
  }
  template <typename U> IntrusivePtr &operator=(IntrusivePtr<U> &&that) {
    std::swap(ptr_, that.ptr_);
    return *this;
  }

  void Reset() { Decref(ptr_); }
  void Reset(element_type *that) { IntrusivePtr{that}.swap(*this); }
  void Reset(element_type *that, bool add_ref) { IntrusivePtr{that, add_ref}.swap(*this); }

  element_type &operator*() const noexcept { return *ptr_; }
  element_type *operator->() const noexcept { return ptr_; }
  element_type *Get() const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return static_cast<bool>(ptr_); }

  /*
   * \brief Helper function for swapping 2 pointers.
   */
  void swap(IntrusivePtr &that) noexcept {  // NOLINT
    element_type *t = ptr_;
    ptr_ = that.ptr_;
    that.ptr_ = t;
  }
};

template <class T, class U>
bool operator==(IntrusivePtr<T> const &x, IntrusivePtr<U> const &y) noexcept {
  return x.Get() == y.Get();
}

template <class T, class U>
bool operator!=(IntrusivePtr<T> const &x, IntrusivePtr<U> const &y) noexcept {
  return x.Get() != y.Get();
}

template <class T, class U>
bool operator==(IntrusivePtr<T> const &x, U *y) noexcept {
  return x.Get() == y;
}

template <class T, class U>
bool operator!=(IntrusivePtr<T> const &x, U *y) noexcept {
  return x.Get() != y;
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
  return std::less<T*>{}(x.Get(), y.Get());
}

template <class E, class T, class Y>
std::basic_ostream<E, T> &operator<<(std::basic_ostream<E, T> &os,
                                     IntrusivePtr<Y> const &p) {
  os << p.Get();
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
}
#endif  // NIH_INTRUSIVE_PTR_H_
