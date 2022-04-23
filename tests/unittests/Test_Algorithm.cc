#include <gtest/gtest.h>
#include <nih/Algorithm.h>

namespace nih {
TEST(Algorithm, NIH) {
  std::vector<float> values{3, 2, 1};
  auto indices = argSort<size_t>(values);
  for (size_t i = 0; i < indices.size(); ++i) {
    ASSERT_EQ(i, indices[indices.size() - i - 1]);
  }
}
}  // namespace nih