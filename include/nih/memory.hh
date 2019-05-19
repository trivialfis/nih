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
 *
 * std::make_unique implementation, translated from libstdc++
 */
#ifndef _MEMORY_HH_
#define _MEMORY_HH_
#include <memory>

namespace nih {

namespace detail {

template <typename Type>
struct MakeUnique {
  using SingleObject = std::unique_ptr<Type>;
};

template <typename Type>
struct MakeUnique<Type[]> {
  using Array = std::unique_ptr<Type[]>;
};

template <typename Type, size_t Bound>
struct MakeUnique<Type[Bound]> {
  using InvalidObject = std::unique_ptr<Type[Bound]>;
};

}  // namespace detail

template <typename Type, typename... Args>
typename detail::MakeUnique<Type>::SingleObject makeUnique(Args&&... args) {
  return std::unique_ptr<Type>(new Type(std::forward<Args>(args)...));
}

template <typename Type>
typename detail::MakeUnique<Type>::Array makeUnique(size_t num) {
  return std::unique_ptr<Type>(new typename std::remove_extent<Type>::type[num]());
}

template <typename Type, typename... Args>
typename detail::MakeUnique<Type>::InvalideObject makeUnique(Args&&...) = delete;

}      // namespace nih

#endif  // _MEMORY_HH_