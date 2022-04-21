/*!
 * Copyright 2018 XGBoost contributors
 */
#include <gtest/gtest.h>
#include <vector>

#include <nih/span.h>

namespace nih {
template <typename Iter>
void InitializeRange(Iter _begin, Iter _end) {
  float j = 0;
  for (Iter i = _begin; i != _end; ++i, ++j) {
    *i = j;
  }
}

#define SPAN_ASSERT_TRUE(cond, status)          \
  if (!(cond)) {                                \
    *(status) = -1;                             \
  }

#define SPAN_ASSERT_FALSE(cond, status)         \
  if ((cond)) {                                 \
    *(status) = -1;                             \
  }

struct TestTestStatus {
  int * status_;

  TestTestStatus(int* _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    SPAN_ASSERT_TRUE(false, status_);
  }
};

struct TestAssignment {
  int* status_;

  TestAssignment(int* _status) : status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    Span<float> s1;

    float arr[] = {3, 4, 5};

    Span<const float> s2 = arr;
    SPAN_ASSERT_TRUE(s2.size() == 3, status_);
    SPAN_ASSERT_TRUE(s2.data() == &arr[0], status_);

    s2 = s1;
    SPAN_ASSERT_TRUE(s2.empty(), status_);
  }
};

struct TestBeginEnd {
  int* status_;

  TestBeginEnd(int* _status) : status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);

    Span<float> s (arr);
    Span<float>::iterator beg { s.begin() };
    Span<float>::iterator end { s.end() };

    SPAN_ASSERT_TRUE(end ==  beg + 16, status_);
    SPAN_ASSERT_TRUE(*beg == arr[0], status_);
    SPAN_ASSERT_TRUE(*(end - 1) == arr[15], status_);
  }
};

struct TestRBeginREnd {
  int * status_;

  TestRBeginREnd(int* _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);

    Span<float> s (arr);
    Span<float>::iterator rbeg { s.rbegin() };
    Span<float>::iterator rend { s.rend() };

    SPAN_ASSERT_TRUE(rbeg == rend + 16, status_);
    SPAN_ASSERT_TRUE(*(rbeg - 1) == arr[15], status_);
    SPAN_ASSERT_TRUE(*rend == arr[0], status_);
  }
};

struct TestObservers {
  int * status_;

  TestObservers(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    // empty
    {
      float *arr = nullptr;
      Span<float> s(arr, static_cast<Span<float>::index_type>(0));
      SPAN_ASSERT_TRUE(s.empty(), status_);
    }

    // size, size_types
    {
      float* arr = new float[16];
      Span<float> s (arr, 16);
      SPAN_ASSERT_TRUE(s.size() == 16, status_);
      SPAN_ASSERT_TRUE(s.size_bytes() == 16 * sizeof(float), status_);
      delete [] arr;
    }
  }
};

struct TestCompare {
  int * status_;

  TestCompare(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float lhs_arr[16], rhs_arr[16];
    InitializeRange(lhs_arr, lhs_arr + 16);
    InitializeRange(rhs_arr, rhs_arr + 16);

    Span<float> lhs(lhs_arr);
    Span<float> rhs(rhs_arr);

    SPAN_ASSERT_TRUE(lhs == rhs, status_);
    SPAN_ASSERT_FALSE(lhs != rhs, status_);

    SPAN_ASSERT_TRUE(lhs <= rhs, status_);
    SPAN_ASSERT_TRUE(lhs >= rhs, status_);

    lhs[2] -= 1;

    SPAN_ASSERT_FALSE(lhs == rhs, status_);
    SPAN_ASSERT_TRUE(lhs < rhs, status_);
    SPAN_ASSERT_FALSE(lhs > rhs, status_);
  }
};

struct TestIterConstruct {
  int * status_;

  TestIterConstruct(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    Span<float>::iterator it1;
    Span<float>::iterator it2;
    SPAN_ASSERT_TRUE(it1 == it2, status_);

    Span<float>::const_iterator cit1;
    Span<float>::const_iterator cit2;
    SPAN_ASSERT_TRUE(cit1 == cit2, status_);
  }
};

struct TestIterRef {
  int * status_;

  TestIterRef(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);

    Span<float> s (arr);
    SPAN_ASSERT_TRUE(*(s.begin()) == s[0], status_);
    SPAN_ASSERT_TRUE(*(s.end() - 1) == s[15], status_);
  }
};

struct TestIterCalculate {
  int * status_;

  TestIterCalculate(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);

    Span<float> s (arr);
    Span<float>::iterator beg { s.begin() };

    beg += 4;
    SPAN_ASSERT_TRUE(*beg == 4, status_);

    beg -= 2;
    SPAN_ASSERT_TRUE(*beg == 2, status_);

    ++beg;
    SPAN_ASSERT_TRUE(*beg == 3, status_);

    --beg;
    SPAN_ASSERT_TRUE(*beg == 2, status_);

    beg++;
    beg--;
    SPAN_ASSERT_TRUE(*beg == 2, status_);
  }
};

struct TestIterCompare {
  int * status_;

  TestIterCompare(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);
    Span<float> s (arr);
    Span<float>::iterator left { s.begin() };
    Span<float>::iterator right { s.end() };

    left += 1;
    right -= 15;

    SPAN_ASSERT_TRUE(left == right, status_);

    SPAN_ASSERT_TRUE(left >= right, status_);
    SPAN_ASSERT_TRUE(left <= right, status_);

    ++right;
    SPAN_ASSERT_TRUE(right > left, status_);
    SPAN_ASSERT_TRUE(left < right, status_);
    SPAN_ASSERT_TRUE(left <= right, status_);
  }
};

struct TestAsBytes {
  int * status_;

  TestAsBytes(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);

    {
      const Span<const float> s {arr};
      const Span<const byte> bs = as_bytes(s);
      SPAN_ASSERT_TRUE(bs.size() == s.size_bytes(), status_);
      SPAN_ASSERT_TRUE(static_cast<const void*>(bs.data()) ==
                       static_cast<const void*>(s.data()),
                       status_);
    }

    {
      Span<float> s;
      const Span<const byte> bs = as_bytes(s);
      SPAN_ASSERT_TRUE(bs.size() == s.size(), status_);
      SPAN_ASSERT_TRUE(bs.size() == 0, status_);
      SPAN_ASSERT_TRUE(bs.size_bytes() == 0, status_);
      SPAN_ASSERT_TRUE(static_cast<const void*>(bs.data()) ==
                       static_cast<const void*>(s.data()),
                       status_);
      SPAN_ASSERT_TRUE(bs.data() == nullptr, status_);
    }
  }
};

struct TestAsWritableBytes {
  int * status_;

  TestAsWritableBytes(int * _status): status_(_status) {}

  void operator()() {
    this->operator()(0);
  }
  void operator()(int _idx) {
    float arr[16];
    InitializeRange(arr, arr + 16);

    {
      Span<float> s;
      Span<byte> bs = as_writable_bytes(s);
      SPAN_ASSERT_TRUE(bs.size() == s.size(), status_);
      SPAN_ASSERT_TRUE(bs.size_bytes() == s.size_bytes(), status_);
      SPAN_ASSERT_TRUE(bs.size() == 0, status_);
      SPAN_ASSERT_TRUE(bs.size_bytes() == 0, status_);
      SPAN_ASSERT_TRUE(bs.data() == nullptr, status_);
      SPAN_ASSERT_TRUE(static_cast<void*>(bs.data()) ==
                       static_cast<void*>(s.data()), status_);
    }

    {
      Span<float> s { arr };
      Span<byte> bs { as_writable_bytes(s) };
      SPAN_ASSERT_TRUE(s.size_bytes() == bs.size_bytes(), status_);
      SPAN_ASSERT_TRUE(static_cast<void*>(bs.data()) ==
                       static_cast<void*>(s.data()), status_);
    }
  }
};

TEST(Span, TestStatus) {
  int status = 1;
  TestTestStatus {&status}();
  ASSERT_EQ(status, -1);
}

TEST(Span, DlfConstructors) {
  // Dynamic extent
  {
    Span<int> s;
    ASSERT_EQ(s.size(), 0);
    ASSERT_EQ(s.data(), nullptr);

    Span<int const> cs;
    ASSERT_EQ(cs.size(), 0);
    ASSERT_EQ(cs.data(), nullptr);
  }

  // Static extent
  {
    Span<int, 0> s;
    ASSERT_EQ(s.size(), 0);
    ASSERT_EQ(s.data(), nullptr);

    Span<int const, 0> cs;
    ASSERT_EQ(cs.size(), 0);
    ASSERT_EQ(cs.data(), nullptr);
  }

  // Init list.
  {
    Span<float> s {};
    ASSERT_EQ(s.size(), 0);
    ASSERT_EQ(s.data(), nullptr);

    Span<int const> cs {};
    ASSERT_EQ(cs.size(), 0);
    ASSERT_EQ(cs.data(), nullptr);
  }
}

TEST(Span, FromNullPtr) {
  // dynamic extent
  {
    Span<float> s {nullptr, static_cast<Span<float>::index_type>(0)};
    ASSERT_EQ(s.size(), 0);
    ASSERT_EQ(s.data(), nullptr);

    Span<float const> cs {nullptr, static_cast<Span<float>::index_type>(0)};
    ASSERT_EQ(cs.size(), 0);
    ASSERT_EQ(cs.data(), nullptr);
  }
  // static extent
  {
    Span<float, 0> s {nullptr, static_cast<Span<float>::index_type>(0)};
    ASSERT_EQ(s.size(), 0);
    ASSERT_EQ(s.data(), nullptr);

    Span<float const, 0> cs {nullptr, static_cast<Span<float>::index_type>(0)};
    ASSERT_EQ(cs.size(), 0);
    ASSERT_EQ(cs.data(), nullptr);
  }
}

TEST(Span, FromPtrLen) {
  float arr[16];
  InitializeRange(arr, arr+16);

  // static extent
  {
    Span<float> s (arr, 16);
    ASSERT_EQ (s.size(), 16);
    ASSERT_EQ (s.data(), arr);

    for (Span<float>::index_type i = 0; i < 16; ++i) {
      ASSERT_EQ (s[i], arr[i]);
    }

    Span<float const> cs (arr, 16);
    ASSERT_EQ (cs.size(), 16);
    ASSERT_EQ (cs.data(), arr);

    for (Span<float const>::index_type i = 0; i < 16; ++i) {
      ASSERT_EQ (cs[i], arr[i]);
    }
  }

  {
    auto lazy = [=]() {Span<float const, 16> tmp (arr, 5);};
    EXPECT_ANY_THROW(lazy());
  }

  // dynamic extent
  {
    Span<float, 16> s (arr, 16);
    ASSERT_EQ (s.size(), 16);
    ASSERT_EQ (s.data(), arr);

    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ (s[i], arr[i]);
    }

    Span<float const, 16> cs (arr, 16);
    ASSERT_EQ (cs.size(), 16);
    ASSERT_EQ (cs.data(), arr);

    for (Span<float const>::index_type i = 0; i < 16; ++i) {
      ASSERT_EQ (cs[i], arr[i]);
    }
  }
}

TEST(Span, FromFirstLast) {
  float arr[16];
  InitializeRange(arr, arr+16);

  // dynamic extent
  {
    Span<float> s (arr, arr + 16);
    ASSERT_EQ (s.size(), 16);
    ASSERT_EQ (s.data(), arr);
    ASSERT_EQ (s.data() + s.size(), arr + 16);

    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ (s[i], arr[i]);
    }

    Span<float const> cs (arr, arr + 16);
    ASSERT_EQ (cs.size(), 16);
    ASSERT_EQ (cs.data(), arr);
    ASSERT_EQ (cs.data() + cs.size(), arr + 16);

    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ (cs[i], arr[i]);
    }
  }

  // static extent
  {
    Span<float, 16> s (arr, arr + 16);
    ASSERT_EQ (s.size(), 16);
    ASSERT_EQ (s.data(), arr);
    ASSERT_EQ (s.data() + s.size(), arr + 16);

    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ (s[i], arr[i]);
    }

    Span<float const> cs (arr, arr + 16);
    ASSERT_EQ (cs.size(), 16);
    ASSERT_EQ (cs.data(), arr);
    ASSERT_EQ (cs.data() + cs.size(), arr + 16);

    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ (cs[i], arr[i]);
    }
  }
}

struct BaseClass {
  virtual void operator()() {}
};
struct DerivedClass : public BaseClass {
  void operator()() override {}
};

TEST(Span, FromOther) {

  // convert constructor
  {
    Span<DerivedClass> derived;
    Span<BaseClass> base { derived };
    ASSERT_EQ(base.size(), derived.size());
    ASSERT_EQ(base.data(), derived.data());
  }

  float arr[16];
  InitializeRange(arr, arr + 16);

  // default copy constructor
  {
    Span<float> s0 (arr);
    Span<float> s1 (s0);
    ASSERT_EQ(s0.size(), s1.size());
    ASSERT_EQ(s0.data(), s1.data());
  }
}

TEST(Span, FromArray) {
  float arr[16];
  InitializeRange(arr, arr + 16);

  {
    Span<float> s (arr);
    ASSERT_EQ(&arr[0], s.data());
    ASSERT_EQ(s.size(), 16);
    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ(arr[i], s[i]);
    }
  }

  {
    Span<float, 16> s (arr);
    ASSERT_EQ(&arr[0], s.data());
    ASSERT_EQ(s.size(), 16);
    for (size_t i = 0; i < 16; ++i) {
      ASSERT_EQ(arr[i], s[i]);
    }
  }
}

TEST(Span, FromContainer) {
  std::vector<float> vec (16);
  InitializeRange(vec.begin(), vec.end());

  Span<float> s(vec);
  ASSERT_EQ(s.size(), vec.size());
  ASSERT_EQ(s.data(), vec.data());

  bool res = std::equal(vec.begin(), vec.end(), s.begin());
  ASSERT_TRUE(res);
}

TEST(Span, Assignment) {
  int status = 1;
  TestAssignment{&status}();
  ASSERT_EQ(status, 1);
}

TEST(SpanIter, Construct) {
  int status = 1;
  TestIterConstruct{&status}();
  ASSERT_EQ(status, 1);
}

TEST(SpanIter, Ref) {
  int status = 1;
  TestIterRef{&status}();
  ASSERT_EQ(status, 1);
}

TEST(SpanIter, Calculate) {
  int status = 1;
  TestIterCalculate{&status}();
  ASSERT_EQ(status, 1);
}

TEST(SpanIter, Compare) {
  int status = 1;
  TestIterCompare{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, BeginEnd) {
  int status = 1;
  TestBeginEnd{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, RBeginREnd) {
  int status = 1;
  TestRBeginREnd{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, ElementAccess) {
  float arr[16];
  InitializeRange(arr, arr + 16);

  Span<float> s (arr);
  size_t j = 0;
  for (auto i : s) {
    ASSERT_EQ(i, arr[j]);
    ++j;
  }

  EXPECT_ANY_THROW(s[16]);
  EXPECT_ANY_THROW(s[-1]);

  EXPECT_ANY_THROW(s(16));
  EXPECT_ANY_THROW(s(-1));
}

TEST(Span, Obversers) {
  int status = 1;
  TestObservers{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, FirstLast) {
  // static extent
  {
    float arr[16];
    InitializeRange(arr, arr + 16);

    Span<float> s (arr);
    Span<float, 4> first = s.first<4>();

    ASSERT_EQ(first.size(), 4);
    ASSERT_EQ(first.data(), arr);

    for (size_t i = 0; i < first.size(); ++i) {
      ASSERT_EQ(first[i], arr[i]);
    }

    EXPECT_ANY_THROW(s.first<static_cast<size_t>(-1)>());
    EXPECT_ANY_THROW(s.first<17>());
    EXPECT_ANY_THROW(s.first<32>());
  }

  {
    float arr[16];
    InitializeRange(arr, arr + 16);

    Span<float> s (arr);
    Span<float, 4> last = s.last<4>();

    ASSERT_EQ(last.size(), 4);
    ASSERT_EQ(last.data(), arr + 12);

    for (size_t i = 0; i < last.size(); ++i) {
      ASSERT_EQ(last[i], arr[i+12]);
    }

    EXPECT_ANY_THROW(s.last<static_cast<size_t>(-1)>());
    EXPECT_ANY_THROW(s.last<17>());
    EXPECT_ANY_THROW(s.last<32>());
  }

  // dynamic extent
  {
    float *arr = new float[16];
    InitializeRange(arr, arr + 16);
    Span<float> s (arr, 16);
    Span<float> first = s.first(4);

    ASSERT_EQ(first.size(), 4);
    ASSERT_EQ(first.data(), s.data());

    for (size_t i = 0; i < first.size(); ++i) {
      ASSERT_EQ(first[i], s[i]);
    }

    EXPECT_ANY_THROW(s.first(-1));
    EXPECT_ANY_THROW(s.first(17));
    EXPECT_ANY_THROW(s.first(32));

    delete [] arr;
  }

  {
    float *arr = new float[16];
    InitializeRange(arr, arr + 16);
    Span<float> s (arr, 16);
    Span<float> last = s.last(4);

    ASSERT_EQ(last.size(), 4);
    ASSERT_EQ(last.data(), s.data() + 12);

    for (size_t i = 0; i < last.size(); ++i) {
      ASSERT_EQ(s[12 + i], last[i]);
    }

    EXPECT_ANY_THROW(s.last(-1));
    EXPECT_ANY_THROW(s.last(17));
    EXPECT_ANY_THROW(s.last(32));

    delete [] arr;
  }
}

TEST(Span, Subspan) {
  int arr[16] {0};
  Span<int> s1 (arr);
  auto s2 = s1.subspan<4>();
  ASSERT_EQ(s1.size() - 4, s2.size());

  auto s3 = s1.subspan(2, 4);
  ASSERT_EQ(s1.data() + 2, s3.data());
  ASSERT_EQ(s3.size(), 4);

  auto s4 = s1.subspan(2, dynamic_extent);
  ASSERT_EQ(s1.data() + 2, s4.data());
  ASSERT_EQ(s4.size(), s1.size() - 2);

  EXPECT_ANY_THROW(s1.subspan(-1, 0));
  EXPECT_ANY_THROW(s1.subspan(16, 0));

  EXPECT_ANY_THROW(s1.subspan<static_cast<size_t>(-1)>());
  EXPECT_ANY_THROW(s1.subspan<16>());
}

TEST(Span, Compare) {
  int status = 1;
  TestCompare{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, AsBytes) {
  int status = 1;
  TestAsBytes{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, AsWritableBytes) {
  int status = 1;
  TestAsWritableBytes{&status}();
  ASSERT_EQ(status, 1);
}

TEST(Span, Empty) {
  {
    Span<float> s {nullptr, static_cast<Span<float>::index_type>(0)};
    auto res = s.subspan(0);
    ASSERT_EQ(res.data(), nullptr);
    ASSERT_EQ(res.size(), 0);

    res = s.subspan(0, 0);
    ASSERT_EQ(res.data(), nullptr);
    ASSERT_EQ(res.size(), 0);
  }

  {
    Span<float, 0> s {nullptr, static_cast<Span<float>::index_type>(0)};
    auto res = s.subspan(0);
    ASSERT_EQ(res.data(), nullptr);
    ASSERT_EQ(res.size(), 0);

    res = s.subspan(0, 0);
    ASSERT_EQ(res.data(), nullptr);
    ASSERT_EQ(res.size(), 0);
  }
}

}  // namespace nih
