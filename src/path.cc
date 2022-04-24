/*
 * Copyright 2019-2021 The NIH Authors. All Rights Reserved.
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
#include "nih/path.h"

#include <sys/stat.h>
#include <unistd.h>

#include <numeric>

#include "nih/Logging.h"
#include "nih/String.h"

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
  NIH_ASSERT_T(ptr);
  Path ret{buff};
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

  std::string dir_name{(_path.size() > 0 && _path.front() == '/') ? "/" : ""};
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