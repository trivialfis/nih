/* This file is part of NIH.
 *
 * Copyright (c) 2019 Jiaming Yuan <jm.yuan@outlook.com>
 *
 * NIH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NIH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NIH.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _STRINGS_HH_
#define _STRINGS_HH_

#include <vector>
#include <string>

#include "nih/primitives.hh"

namespace nih {

template <typename Functor>
void split(std::vector<std::string>& result, std::string const& input,
           Functor func) {
  NihInt last_pos = 0;
  NihInt len = 0;
  for (auto c : input) {
    if (!func(c)) {
      len++;
    } else {
      result.push_back(input.substr(last_pos, len));
      last_pos += len + 1;
      len = 0;
    }
  }
  if (input.size() - last_pos > 0) {
    result.push_back(input.substr(last_pos));
  }
}

template <typename Functor>
std::string strip(std::string const& str, Functor func) {
  NihInt prefix_count = 0;
  for (char c : str) {
    if (func(c)) {
      prefix_count ++;
    } else {
      break;
    }
  }
  NihInt postfix_count = 0;
  for (auto riter = str.rbegin(); riter != str.rend(); ++riter) {
    if (func(*riter)) {
      postfix_count ++;
    } else {
      break;
    }
  }
  if (prefix_count == postfix_count) {
    return std::string{};
  }
  NihInt length = str.length() - postfix_count - prefix_count;
  std::string result = str.substr(prefix_count, length);
  return result;
}

inline bool isSpace(char c) {
  if (c == ' ' || c == '\n' || c == '\t') {
    return true;
  }
  return false;
}

inline std::string strip(std::string const& str) {
  return strip(str, isSpace);
}

template<typename T>
T str2n(std::string const& s);

template <>
inline NihFloat str2n<NihFloat>(std::string const& s) {
  return std::stod(s);
}

template <>
inline NihDouble str2n<NihDouble>(std::string const& s) {
  return std::stod(s);
}

template <>
inline NihInt str2n<NihInt>(std::string const& s) {
  return static_cast<NihInt>(std::stoi(s));
}

}  // namespace nih

#endif  // _STRINGS_HH_