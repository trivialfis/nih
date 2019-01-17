#include "nih/strings.hh"
#include "nih/primitives.hh"
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
  std::string input {"Hello, world"};
  std::vector<std::string> result;
  split(result, input, [](char c){ return c == ','; });
  ASSERT_EQ(result.size(), 2);
  ASSERT_EQ(result.at(0), "Hello");
  ASSERT_EQ(result.at(1), " world");

  input = "Hello";
  split(result, input, [](char c){ return c == ','; });
  ASSERT_EQ(result.at(2), "Hello");

  input = "";
  split(result, input, [](char c){ return c == ','; });
  ASSERT_EQ(result.size(), 3);
}

}  // namespace nih