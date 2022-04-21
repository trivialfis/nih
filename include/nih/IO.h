/*
 * Copyright 2021 The Tungsten Authors. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this
 * file except in compliance with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under
 * the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the specific language governing
 * permissions and limitations under the License.
 *
 */
#ifndef SUPPORT_IO_H_
#define SUPPORT_IO_H_

#include <string>
#include <fstream>
#include <cstring>
#include "Logging.h"

namespace tungsten {
inline std::string loadSequentialFile(std::string uri) {
  auto OpenErr = [&uri]() {
    std::string msg;
    msg = "Opening " + uri + " failed: ";
    msg += strerror(errno);
    LOG(FATAL) << msg;
  };

  std::string buffer;
  // Open in binary mode so that correct file size can be computed with
  // seekg(). This accommodates Windows platform:
  // https://docs.microsoft.com/en-us/cpp/standard-library/basic-istream-class?view=vs-2019#seekg
  std::ifstream ifs(uri, std::ios_base::binary | std::ios_base::in);
  if (!ifs) {
    // https://stackoverflow.com/a/17338934
    OpenErr();
  }

  ifs.seekg(0, std::ios_base::end);
  const size_t file_size = static_cast<size_t>(ifs.tellg());
  ifs.seekg(0, std::ios_base::beg);
  buffer.resize(file_size + 1);
  ifs.read(&buffer[0], file_size);
  buffer.back() = '\0';

  return buffer;
}
} // namespace tungsten
#endif
