#include <gtest/gtest.h>

#include <cinttypes>
#include <nih/concept.h>

namespace nih {

namespace {
struct Foo {};

struct Bar {
  bool operator==(Foo const& foo) {
    return true;
  }
};

}  // anonymous namespace

TEST(Concept, Comparable) {
  static_assert(EqualityComparable<float, float>::value, "EqualityComparable");
  static_assert(EqualityComparable<float>::value,        "EqualityComparable");
  static_assert(EqualityComparable<float, int32_t>::value, "EqualityComparable");

  static_assert(!EqualityComparable<Foo, int32_t>::value, "EqualityComparable");
  static_assert(!EqualityComparable<Foo, Foo>::value,     "EqualityComparable");

  static_assert(EqualityComparable<Bar, Foo>::value, "EqualityComparable");

  static_assert(GreaterEqualComparable<float, int32_t>::value, "GreaterEqualComparable");
  static_assert(LessEqualComparable<float, int32_t>::value, "LessEqualComparable");
}
}  // namespace nih