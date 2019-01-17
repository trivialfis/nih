/* This file is part of NIH.
 *
 * Copyright (c) 2019 Jiaming Yuan <jm.yuan@outlook.com>
 *
 * NIH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NIH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NIH.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _MATH_HH_
#define _MATH_HH_

#include <cmath>
#include <type_traits>
#include <utility>

#include "nih/primitives.hh"

namespace nih {
namespace math {

inline NihFloat beta(NihFloat x, NihFloat y) {
  return std::tgamma(x) * std::tgamma(y) / std::tgamma(x + y);
}

inline NihFloat lbeta(NihFloat x, NihFloat y) {
  return std::lgamma(x) + std::lgamma(y) - std::lgamma(x + y);
}

}  // namespace math
}  // namespace nih
#endif