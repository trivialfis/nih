/* This file is part of NIH.
 *
 * Copyright (c) 2019 Jiaming Yuan <jm.yuan@outlook.com>
 *
 * NIH is free software: you can redistribute it and/or modify it under the
 * terms of the Lesser GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * NIH is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Lesser GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with NIH.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _NIH_FUNCTIONAL_HH_
#define _NIH_FUNCTIONAL_HH_

namespace nih {

template <typename T, typename U>
struct Less {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return lhs < rhs;
  }
};

template <typename T, typename U>
struct Greater {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return lhs > rhs;
  }
};

template <typename T, typename U>
struct GreaterEqual {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return !Less<T, U>()(lhs, rhs);
  }
};

template <typename T, typename U>
struct LessEqual {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return !Greater<T, U>()(lhs, rhs);
  }
};

template <typename T, typename U>
struct Equal {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return lhs == rhs;
  }
};

}  // namespace nih

#endif  // _NIH_FUNCTIONAL_HH_