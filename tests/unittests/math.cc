/*
 * Copyright 2019-2022 The NIH Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied.  See the License for the specific language
 * governing permissions and limitations under the License.
 *
 */
#include "nih/math.h"

#include <gtest/gtest.h>

namespace nih {
namespace math {
TEST(Math, Beta) {
  float constexpr kEps = 1e-6;
  ASSERT_NEAR(beta(1, 2), 0.5, kEps);
  ASSERT_NEAR(beta(3, 2), 0.083333333, kEps);

  ASSERT_NEAR(lbeta(1, 2), -0.69314718056, kEps);
}
}  // namespace math
}  // namespace nih