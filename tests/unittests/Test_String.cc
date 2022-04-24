/*
 * Copyright 2021 The NIH Authors. All Rights Reserved.
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
#include <gtest/gtest.h>
#include <nih/String.h>
#include <nih/StringRef.h>

#include <cstddef>
#include <string>

namespace nih {

template <typename T>
struct IsDouble {
  static bool constexpr kValue = false;
};
template <>
struct IsDouble<double> {
  static bool constexpr kValue = true;
};

template <typename T>
struct IsFloat {
  static bool constexpr kValue = false;
};
template <>
struct IsFloat<float> {
  static bool constexpr kValue = true;
};

template <typename T>
struct IsInt {
  static bool constexpr kValue = false;
};
template <>
struct IsInt<std::int64_t> {
  static bool constexpr kValue = true;
};

TEST(Strings, StoN) {
  constexpr double kEps = 1e-7;
  auto f = str2n<float>("1.1");
  static_assert(IsFloat<decltype(f)>::kValue, "Return type must be float.");
  ASSERT_NEAR(f, 1.1, kEps);

  auto d = str2n<double>("1.1");
  static_assert(IsDouble<decltype(d)>::kValue, "Return type must be double.");
  ASSERT_NEAR(d, 1.1, kEps);

  auto i = str2n<std::int64_t>("1");
  static_assert(IsInt<decltype(i)>::kValue, "Return type must be int.");
  ASSERT_EQ(i, 1);
}

TEST(Strings, Strip) {
  std::string input{"\tHello, world\n\n\t"};
  std::string result = strip(input);
  std::string solution = "Hello, world";
  ASSERT_EQ(result, solution);
}

TEST(Strings, Split) {
  {
    std::string input{"Hello, world"};
    std::vector<std::string> result;
    split(input, &result, [](char c) { return c == ','; });
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result.at(0), "Hello");
    ASSERT_EQ(result.at(1), " world");

    input = "Hello";
    split(input, &result, [](char c) { return c == ','; });
    ASSERT_EQ(result.at(2), "Hello");

    input = "";
    split(input, &result, [](char c) { return c == ','; });
    ASSERT_EQ(result.size(), 3);
  }

  {
    std::string input{"foo bar"};
    auto result = split(input, [](char c) { return c == ' '; });
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result.front(), "foo");
    ASSERT_EQ(result.back(), "bar");
  }
}

TEST(StringRef, Constructor) {
  {
    std::string str{"foo-bar"};
    StringRef ref(str);
    ASSERT_EQ(ref.size(), str.size());
    ConstStringRef cref(str);
    ASSERT_EQ(cref.size(), str.size());

    for (size_t i = 0; i < ref.size(); ++i) {
      ASSERT_EQ(ref[i], cref[i]);
      ASSERT_EQ(ref[i], str[i]);
    }
  }
}

}  // namespace nih