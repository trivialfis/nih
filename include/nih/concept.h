#include <type_traits>
#include <nih/functional.h>

namespace nih {
template <typename L, typename R, typename Op>
struct Comparable {
  template <typename T, typename U>
  static auto test(U*) -> decltype(std::declval<T>() == std::declval<U>());
  template <typename, typename>
  static auto test(...) -> std::false_type;

  using type = typename std::is_same<bool, decltype(test<L, R>(0))>::type;
  auto static constexpr value = type::value;
};

template <typename L, typename R = L>
struct EqualityComparable : Comparable<L, R, Equal<L, R>> {};

template <typename L, typename R = L>
struct LessComparable : Comparable<L, R, Less<L, R>> {};

template <typename L, typename R = L>
struct GreaterComparable : Comparable<L, R, Greater<L, R>> {};

template <typename L, typename R = L>
struct LessEqualComparable : Comparable<L, R, LessEqual<L, R>> {};

template <typename L, typename R = L>
struct GreaterEqualComparable : Comparable<L, R, GreaterEqual<L, R>> {};
}  // namespace nih