namespace nih {

template <typename T, typename U>
struct Less {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return lhs < rhs;
  }
};

template <typename T, typename U>
struct Greater {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return lhs > rhs;
  }
};

template <typename T, typename U>
struct GreaterEqual {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return !Less<T, U>()(lhs, rhs);
  }
};

template <typename T, typename U>
struct LessEqual {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return !Greater<T, U>()(lhs, rhs);
  }
};

template <typename T, typename U>
struct Equal {
  constexpr bool operator()(T&& lhs, U&& rhs) const {
    return lhs == rhs;
  }
};

}  // namespace nih