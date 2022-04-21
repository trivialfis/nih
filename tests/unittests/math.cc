#include <gtest/gtest.h>

#include "nih/primitives.hh"
#include "nih/math.h"

namespace nih {
namespace math {

TEST(Math, Beta) {
  NihFloat constexpr kEps = 1e-6;
  ASSERT_NEAR(beta(1, 2), 0.5, kEps);
  ASSERT_NEAR(beta(3, 2), 0.083333333, kEps);

  ASSERT_NEAR(lbeta(1, 2), -0.69314718056, kEps);
}

}  // namespace math
}  // namespace nih