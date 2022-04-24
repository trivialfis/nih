/*
 * Copyright 2022 The NIH Authors. All Rights Reserved.
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
#include <nih/Logging.h>

#include <cstring>
#include <filesystem>
#include <string>

namespace nih {
class TemporaryDirectory {
  std::string _path;

 public:
  TemporaryDirectory() {
    std::string path = "/tmp/tmpdir.XXXXXX";
    auto ptr = mkdtemp(path.data());
    std::string msg;
    msg.resize(1024);
    if (!ptr) {
      auto ret = strerror_r(errno, msg.data(), msg.size());
      if (ret) {
        LOG(FATAL) << "Failed to create temporary directoroy:" << ret;
      } else {
        LOG(FATAL) << "Failed to create temporary directoroy, unknown error.";
      }
    } else {
      _path = ptr;
    }
  }

  ~TemporaryDirectory() noexcept(false) {
    if (!_path.empty()) {
      int32_t err = std::remove(_path.c_str());
      if (err != 0) {
        LOG(FATAL) << "Failed to remove file:" << _path;
      }
    }
  }

  [[nodiscard]] ConstStringRef path() const { return _path; }
};
}  // namespace nih