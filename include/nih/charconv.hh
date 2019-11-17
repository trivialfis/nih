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
#ifndef _NIH_CHARCONV_HH_
#define _NIH_CHARCONV_HH_

#include <system_error>
#include <iterator>
#include <limits>
#include <nih/ryu.hh>
#include <nih/luts.hh>
#include <nih/logging.hh>

namespace nih {

struct to_chars_result {
  char* ptr;
  std::errc ec;
};

namespace {

constexpr uint32_t ten2() {
  return 10 * 10;
}
constexpr uint32_t ten3() {
  return 10 * ten2();
}
constexpr uint32_t ten4() {
  return ten2() * ten2();
}

constexpr uint32_t shortestDigit10Impl(uint64_t value, uint32_t n) {
  return value < 10 ? n :
      (value < ten2() ? n + 1 :
       (value < ten3() ? n + 2 :
        (value < ten4() ? n + 3 :
         shortestDigit10Impl(value / ten4(), n + 4))));
}

constexpr uint32_t shortestDigit10(uint64_t value) {
  return shortestDigit10Impl(value, 1);
}

// This is an implementation for base 10 inspired by the one in libstdc++v3.  The general
// scheme is by decomposing the value into multiple combination of base (which is 10) by
// mod, until the value is lesser than 10, then last char is just char '0' (ascii 48) plus
// that value.  Other popular implementations can be found in RapidJson and libc++ (in
// llvm-project), which uses the same general work flow with the same look up table, but
// probably with better performance as they are more complicated.
inline void itoaUnsignedImpl(char* first, uint32_t length, uint64_t value) {
  uint32_t position = length - 1;
  while (value > ten2()) {
    auto const num = (value % ten2()) * 2;
    value /= ten2();
    first[position] = kItoaLut[num + 1];
    first[position - 1] = kItoaLut[num];
    position -= 2;
  }
  if (value > 10) {
    auto const num = value * 2;
    first[0] = kItoaLut[num];
    first[1] = kItoaLut[num + 1];
  } else {
    first[0]= '0' + value;
  }
}

inline to_chars_result toCharsUnsignedImpl(char *first, char *last,
                                           uint64_t const value) {
  const uint32_t output_len = shortestDigit10(value);
  to_chars_result ret;
  if (NIH_EXPECT(std::distance(first, last) == 0, false)) {
    ret.ec = std::errc::value_too_large;
    ret.ptr = last;
    return ret;
  }

  itoaUnsignedImpl(first, output_len, value);
  ret.ptr = first + output_len;
  ret.ec = std::errc();
  return ret;
}
}  // anonymous namespace

to_chars_result to_chars(char *first, char *last, int64_t value) {
  if (NIH_EXPECT(first == last, false)) {
    return {first, std::errc::value_too_large};
  }
  // first write '-' and convert to unsigned, then write the rest.
  if (value == 0) {
    *first = '0';
    return {std::next(first), std::errc()};
  }
  uint64_t unsigned_value = value;
  if (value < 0) {
    *first = '-';
    std::advance(first, 1);
    unsigned_value = uint64_t(~value) + uint64_t(1);
  }
  return toCharsUnsignedImpl(first, last, unsigned_value);
}

to_chars_result to_chars(char  *first, char *last, float value) {
  auto index = f2s_buffered_n(value, first);
  to_chars_result ret;
  ret.ptr = first + index;
  if (NIH_EXPECT(ret.ptr < last, true)) {
    ret.ec = std::errc();
  } else {
    ret.ec =  std::errc::value_too_large;
    ret.ptr = last;
  }
  return ret;
}
}  // namespace nih

#endif   // _NIH_CHARCONV_HH_