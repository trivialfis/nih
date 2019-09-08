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
#include <unistd.h>
#include <sys/stat.h>
#include <numeric>
#include "nih/path.hh"
#include "nih/logging.hh"
#include "nih/strings.hh"

namespace nih {
Path Path::join(Path const& lhs, Path const& rhs) {
  auto append_dir = [](Path const& in) {
                      Path ret;
                      if (in._path.size() > 0 && in._path.back() == '/') {
                        ret._path = in._path;
                      } else if (in._path.size() > 0) {
                        ret._path = in._path + '/';
                      } else {
                        // empty path, do nothing.
                        ret._path = in._path;
                      }
                      return ret;
                    };
  Path ret;
  bool const left_ends_delimiter = lhs._path.size() > 0 && lhs._path.back() == '/';
  bool const right_begs_delimiter = rhs._path.size() > 0 && rhs._path.front() == '/';
  if (left_ends_delimiter && right_begs_delimiter) {
    ret = {lhs._path + rhs._path.substr(1)};
  } else if (left_ends_delimiter && !right_begs_delimiter) {
    ret = {lhs._path + rhs._path};
  } else if (!left_ends_delimiter && right_begs_delimiter) {
    ret = {lhs._path + '/' + rhs._path};
  } else if (!left_ends_delimiter && !right_begs_delimiter) {
    ret = {lhs._path + '/' + rhs._path};
  }
  return ret;
}

Path Path::curdir() {
  char buff[FILENAME_MAX];
  auto ptr = getcwd(buff, FILENAME_MAX);
  NIH_ASSERT(ptr) << std::string{strerror(errno)};
  Path ret {buff};
  return ret;
}

Path Path::dirname() const {
  // `.` -> `.`
  if (_path.size() == 1 && _path.front() == '.') {
    return *this;
  }

  auto splited = split(_path, [](char c) { return c == '/'; });

  // single word path, like `usr`.
  if (splited.size() == 1 && _path.size() > 1 && _path.front() != '/') {
    return Path{"."};
  }

  std::string dir_name { (_path.size() > 0 && _path.front() == '/') ? "/" : "" };
  dir_name = std::accumulate(splited.cbegin(), splited.cend() - 1, dir_name);
  if (dir_name.size() > 1 && dir_name.back() == '/') {
    dir_name = dir_name.substr(0, dir_name.size() - 1);
  }
  return Path{dir_name};
}

bool Path::isFile() const {
  struct stat path_stat;
  stat(_path.c_str(), &path_stat);
  return static_cast<bool>(S_ISREG(path_stat.st_mode));
}

bool Path::isDir() const {
  struct stat path_stat;
  stat(_path.c_str(), &path_stat);
  return static_cast<bool>(S_ISDIR(path_stat.st_mode));
}

bool Path::isSymlink() const {
  struct stat path_stat;
  stat(_path.c_str(), &path_stat);
  return static_cast<bool>(S_ISLNK(path_stat.st_mode));
}
}  // namespace nih