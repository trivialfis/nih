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
#pragma once

#include <cmath>
#include <type_traits>
#include <utility>

namespace nih {
namespace math {
inline float beta(float x, float y) {
  return std::tgamma(x) * std::tgamma(y) / std::tgamma(x + y);
}

inline float lbeta(float x, float y) {
  return std::lgamma(x) + std::lgamma(y) - std::lgamma(x + y);
}
}  // namespace math
}  // namespace nih
