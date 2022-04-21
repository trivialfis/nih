#include "nih/strings.hh"
#include "nih/StringRef.h"
#include "nih/primitives.hh"
#include <cstddef>
#include <gtest/gtest.h>

#include <string>

namespace nih {

template<typename T>
struct IsDouble {
  static bool constexpr kValue = false;
};
template <>
struct IsDouble<NihDouble> {
  static bool constexpr kValue = true;
};

template <typename T>
struct IsFloat {
  static bool constexpr kValue = false;
};
template <>
struct IsFloat<NihFloat> {
  static bool constexpr kValue = true;
};

template <typename T>
struct IsInt {
  static bool constexpr kValue = false;
};
template <>
struct IsInt<NihInt> {
  static bool constexpr kValue = true;
};

TEST(Strings, StoN) {
  constexpr double kEps = 1e-7;
  auto f = str2n<NihFloat>("1.1");
  static_assert(IsFloat<decltype(f)>::kValue, "Return type must be float.");
  ASSERT_NEAR(f, 1.1, kEps);

  auto d = str2n<NihDouble>("1.1");
  static_assert(IsDouble<decltype(d)>::kValue, "Return type must be double.");
  ASSERT_NEAR(d, 1.1, kEps);

  auto i = str2n<NihInt>("1");
  static_assert(IsInt<decltype(i)>::kValue, "Return type must be int.");
  ASSERT_EQ(i, 1);
}

TEST(Strings, Strip) {
  std::string input {"\tHello, world\n\n\t"};
  std::string result = strip(input);
  std::string solution = "Hello, world";
  ASSERT_EQ(result, solution);
}

TEST(Strings, Split) {
  {
    std::string input {"Hello, world"};
    std::vector<std::string> result;
    split(input, &result, [](char c){ return c == ','; });
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result.at(0), "Hello");
    ASSERT_EQ(result.at(1), " world");

    input = "Hello";
    split(input, &result, [](char c){ return c == ','; });
    ASSERT_EQ(result.at(2), "Hello");

    input = "";
    split(input, &result, [](char c){ return c == ','; });
    ASSERT_EQ(result.size(), 3);
  }

  {
    std::string input {"foo bar"};
    auto result = split(input, [](char c){ return c == ' '; });
    ASSERT_EQ(result.size(), 2);
    ASSERT_EQ(result.front(), "foo");
    ASSERT_EQ(result.back(), "bar");
  }
}

TEST(StringRef, Constructor) {
  {
    std::string str {"foo-bar"};
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