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
#include <vector>

namespace nih {

class StackTrace {
  std::vector<std::string> _trace;
  static size_t constexpr kDefaultSize = 16;
  size_t _stack_size;

 public:
  StackTrace(size_t stack_size = kDefaultSize);

  std::vector<std::string> const& refresh();
  std::vector<std::string> const& refresh(size_t stack_size);

  std::vector<std::string> const& get() const;
  std::string str() const;
};


class NIHError : public std::exception {
  std::string _error;
  StackTrace _trace;

 public:
  explicit NIHError(std::string const& what_arg) : _error{what_arg} {}
  explicit NIHError(char const* what_arg) : _error{what_arg} {}

  virtual const char* what() const noexcept {  // NOLINT
    return _error.c_str();
  }
  virtual StackTrace const& trace() const {
    return _trace;
  }
  virtual ~NIHError() = default;
};

class FatalError : public NIHError {
 public:
  explicit FatalError(std::string const& what_arg) : NIHError(what_arg) {}
  explicit FatalError(char const* what_arg) : NIHError(what_arg) {}
};

class RecoverableError : public NIHError {
 public:
  explicit RecoverableError(std::string const& what_arg) : NIHError(what_arg) {}
  explicit RecoverableError(char const* what_arg) : NIHError(what_arg) {}
};

}  // namespace nih

#endif  // _ERRORS_HH_