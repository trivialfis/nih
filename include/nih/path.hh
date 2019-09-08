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
#ifndef _NIH_PATH_HH_
#define _NIH_PATH_HH_

#include <string>
#include <iostream>

namespace nih {

class Path {
  std::string _path;

 public:
  /*! \brief Return current working directory. */
  static Path curdir();

  static Path join(Path const& lhs, Path const& rhs);
  static Path join(Path const& that) { return that; }
  /*! \brief Join multiple path objects into one. */
  template <typename... Paths>
  static Path join(Path that, Paths const& ...args) {
    return join(that, join(args...));
  }

 public:
  Path() = default;
  Path(Path const& that) : _path{that._path} {}
  Path(Path&& that): _path{std::move(that._path)} {}
  explicit Path(std::string const& path) : _path{path} {}

  Path& operator=(std::string const& path) {
    _path = path;
    return *this;
  }
  Path& operator=(Path const& that) {
    _path = that._path;
    return *this;
  }
  Path& operator=(Path&& that) {
    _path = std::move(that._path);
    return *this;
  }

  Path operator+(Path const& that) const {
    return join(*this, that);
  }
  Path& operator+=(Path const& that) {
    *this = *this + that;
    return *this;
  }

  std::string const& str() const { return _path; }

  bool isFile() const;
  bool isDir() const;
  bool isSymlink() const;
};

std::ostream& operator<<(std::ostream& os, Path const& that) {
  os << that.str();
  return os;
}
}  // namespace nih

#endif  // _NIH_PATH_HH_