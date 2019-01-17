#include <gtest/gtest.h>

#include "nih/range.hh"

namespace nih {

TEST(Basic, Range) {
  NihInt j = 0;
  for (auto i : Range(0, 10)) {
    ASSERT_EQ(i, j);
    j++;
  }
  ASSERT_EQ(j, 10);

  j = 0;
  for (auto i : Range(10)) {
    ASSERT_EQ(i, j);
    j++;
  }
  ASSERT_EQ(j, 10);

  j = 0;
  for (auto i : Range(0, 10, 2)) {
    ASSERT_EQ(i, j);
    j += 2;
  }
  ASSERT_EQ(j, 10);
}

}  // namespace nih