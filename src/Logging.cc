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
#include <cxxabi.h>   // demangle
#include <execinfo.h>  // backtrace

#include "nih/Logging.h"
#include "nih/json.h"
#include "nih/Intrinsics.h"

namespace nih {

void Logger::Stream::defaultHandler(char const *msg) {
  auto j_msg = Json::Load(msg);
  auto verbosity = get<Integer const>(j_msg["verbosity"]);
  auto msg_str = get<String const>(j_msg["msg"]);

  switch (static_cast<Verbosity>(verbosity)) {
  case kFatal: {
    throw std::runtime_error(msg);
  }
  case kWarning:
    std::cout << msg_str;
    break;
  case kInfo:
    std::cout << msg_str;
    break;
  case kDebug:
    std::cout << msg_str;
    break;
  default:
    throw std::runtime_error("Unexpected verbosity: " + std::to_string(verbosity));
  }
}

Logger::Stream::~Stream() noexcept(false) {
  if (TS_UNLIKELY(!_ignore)) {
    auto const& str = _ss.str();

    Json msg {Object{}};
    msg["verbosity"] = Integer(static_cast<Integer::Int>(this->_v));
    msg["msg"] = str;

    if (this->_v == kFatal) {
      StackTrace st;
      auto backtrace = st.str();
      msg["backtrace"] = backtrace;
    }

    std::string out;
    Json::Dump(msg, &out);
    getHandler()(out.c_str());
  }
}
}  // namespace nih
