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
#include <cxxabi.h>
#include <execinfo.h>

#include "nih/errors.hh"

namespace nih {

size_t constexpr StackTrace::kDefaultSize;

StackTrace::StackTrace(size_t stack_size) : _stack_size(stack_size) {
  this->refresh(_stack_size);
}
std::vector<std::string> const& StackTrace::refresh() {
  return this->refresh(_stack_size);
}

std::vector<std::string> const& StackTrace::refresh(size_t stack_size) {
  _trace.clear();
  std::vector<void*> stack(stack_size);
  int32_t nframes = backtrace(stack.data(), static_cast<int32_t>(stack_size));
  char** c_msgs = backtrace_symbols(stack.data(), nframes);
  if (c_msgs != nullptr) {
    for (auto i = 1; i < nframes; ++i) {
      std::string msg(c_msgs[i]);
      auto beg = msg.find("_Z");
      auto end = msg.find_first_of('+', beg);
      if (beg == std::string::npos || end == std::string::npos) {
        _trace.emplace_back(msg);
        continue;
      }

      size_t length = 0;
      int status = 0;
      std::string raw_symbol (msg, beg, end - beg);
      char* c_symbol = abi::__cxa_demangle(raw_symbol.c_str(), 0, &length, &status);
      if (status != 0) {
        _trace.emplace_back(msg);
        continue;
      }
      std::string symbol{c_symbol};
      std::free(c_symbol);
      _trace.emplace_back(symbol);
    }
    std::free(c_msgs);
  }
  return _trace;
}

std::vector<std::string> const& StackTrace::get() const { return _trace; }

std::string StackTrace::str() const {
  std::string trace_str;
  uint32_t count = 0;
  for (auto const& symbol : _trace) {
    trace_str += "[" + std::to_string(count) + "] ";
    trace_str += symbol;
    trace_str += '\n';
    count++;
  }
  return trace_str;
}

}  // namespace nih