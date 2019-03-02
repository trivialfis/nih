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
#ifndef _ERRORS_HH_
#define _ERRORS_HH_

#include <exception>
#include <string>

namespace nih {
class NIHError : public std::exception {
  std::string error_;

 public:
  explicit NIHError(std::string const& what_arg) : error_{what_arg} {}
  explicit NIHError(char const* what_arg) : error_{what_arg} {}

  virtual const char* what() const noexcept {  // NOLINT
    return error_.c_str();
  }
  virtual ~NIHError() = default;
};
}  // namespace nih

#endif  // _ERRORS_HH_