#include <gtest/gtest.h>
#include <nih/intrusive_ptr.hh>

namespace nih {
struct ForIntrusivePtrTest {
  mutable class RefCount ref;
  float data { 0 };

  friend RefCount &
  IntrusivePtrRefCount(ForIntrusivePtrTest const *t) noexcept {
    return t->ref;
  }

  friend void
  IntrusivePtrDestroy(ForIntrusivePtrTest const *t) noexcept {
    delete t;
  }
};


TEST(IntrusivePtr, Basic) {
  auto p = new ForIntrusivePtrTest;
  IntrusivePtr<ForIntrusivePtrTest> ptr { p };
  ASSERT_EQ(ptr.Get(), p);

  IntrusivePtr<ForIntrusivePtrTest> ptr_1 { ptr };
  ASSERT_EQ(ptr_1.Get(), p);

  ASSERT_EQ((*ptr_1).data, ptr_1->data);

  ASSERT_EQ(std::hash<IntrusivePtr<ForIntrusivePtrTest>>{}(ptr_1),
            std::hash<ForIntrusivePtrTest*>{}(ptr_1.Get()));

  ASSERT_EQ(ptr, p);
  ASSERT_EQ(ptr_1, ptr);

  ForIntrusivePtrTest* raw_ptr {nullptr};
  ASSERT_NE(ptr_1, raw_ptr);
  ASSERT_NE(raw_ptr, ptr_1);

  auto p_1 = new ForIntrusivePtrTest;
  ptr.Reset(p_1);

  ASSERT_EQ(IntrusivePtrRefCount(ptr_1.Get()).Count(), 1);
  ASSERT_EQ(IntrusivePtrRefCount(ptr.Get()).Count(), 1);

  IntrusivePtrRefCount(ptr_1.Get()).Inc();
  ASSERT_EQ(IntrusivePtrRefCount(ptr_1.Get()).Count(), 2);
  IntrusivePtr<ForIntrusivePtrTest> ptr_2 {ptr_1.Get(), false};
  ASSERT_EQ(IntrusivePtrRefCount(ptr_1.Get()).Count(), 2);

  ASSERT_TRUE(ptr);
  ASSERT_TRUE(ptr_1);
  ASSERT_TRUE(ptr_2);

  ptr_2.Reset(nullptr);
  ASSERT_FALSE(ptr_2);
}
} // namespace nih
