#include <gtest/gtest.h>
#include "nih/memory.hh"

namespace nih {
TEST(MakeUnique, Basic) {
  // no crash
  auto p_vec = makeUnique<std::vector<int>>(3, 0);
  p_vec->at(2) = 1;
  auto int_arr = makeUnique<int[]>(4);  // NOLINT
  int_arr[3] = 0;
}

TEST(MemInfo, Basic) {
  MemInfo meminfo(false);
  ASSERT_NE(meminfo.memAvailable(), 0);
  ASSERT_NE(meminfo.memTotal(), 0);
}

}  // namespace nih