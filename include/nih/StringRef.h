#ifndef STRING_REF_H_
#define STRING_REF_H_

#include <nih/span.h>

#include <ostream>
#include <string>

namespace nih {
/*! \brief A mutable string_view. */
template <typename CharT> class StringRefImpl {
public:
  using pointer = CharT *;              // NOLINT
  using iterator = pointer;             // NOLINT
  using const_iterator = CharT const *; // NOLINT

  using value_type = CharT;                    // NOLINT
  using traits_type = std::char_traits<CharT>; // NOLINT
  using size_type = std::size_t;               // NOLINT
  using difference_type = std::ptrdiff_t;      // NOLINT

  using reference = CharT &;             // NOLINT
  using const_reference = CharT const &; // NOLINT

private:
  CharT *_chars{nullptr};
  size_t _size{0};

public:
  StringRefImpl(std::string &str)
      : _chars{&str[0]}, _size{str.size()} {} // NOLINT
  StringRefImpl(std::string const &str)
      : _chars{str.data()}, _size{str.size()} {} // NOLINT
  StringRefImpl(CharT *chars, size_t size) : _chars{chars}, _size{size} {}
  StringRefImpl(CharT *chars)
      : _chars{chars}, _size{traits_type::length(chars)} {} // NOLINT
  template <typename CharU,
            std::enable_if_t<!std::is_same_v<CharT, CharU>> * = nullptr>
  StringRefImpl(StringRefImpl<CharU> that)
      : _chars{that.data()}, _size{that.size()} {}

  const_iterator cbegin() const { return _chars; }        // NOLINT
  const_iterator cend() const { return _chars + size(); } // NOLINT

  iterator begin() { return _chars; }              // NOLINT
  iterator end() { return _chars + size(); }       // NOLINT
  iterator begin() const { return _chars; }        // NOLINT
  iterator end() const { return _chars + size(); } // NOLINT

  pointer data() const { return _chars; }  // NOLINT
  pointer c_str() const { return data(); } // NOLINT

  size_t size() const { return _size; }; // NOLINT
  CharT operator[](size_t i) const { return _chars[i]; }

  explicit operator std::string() const {
    std::string str;
    str.resize(this->size());
    std::copy(cbegin(), cend(), str.begin());
    return str;
  }

  bool operator==(StringRefImpl const &that) {
    return Span<CharT>{this, _size} == Span<CharT>{that.data(), that.size()};
  }
  bool operator!=(StringRefImpl const &that) { return !(that == *this); }
  bool operator<(StringRefImpl const &that) {
    return Span<CharT>{_chars, _size} < Span<CharT>{that.data(), that.size()};
  }
  bool operator>(StringRefImpl const &that) {
    return Span<CharT>{_chars, _size} > Span<CharT>{that.data(), that.size()};
  }
  bool operator<=(StringRefImpl const &that) {
    return Span<CharT>{_chars, _size} <= Span<CharT>{that.data(), that.size()};
  }
  bool operator>=(StringRefImpl const &that) {
    return Span<CharT>{_chars, _size} >= Span<CharT>{that.data(), that.size()};
  }

  StringRefImpl substr(size_t pos, size_t len) const {
    auto sub = Span<CharT>{_chars, _size}.subspan(pos, len);
    return StringRefImpl{sub.data(), sub.size()};
  }
};

using StringRef = StringRefImpl<std::string::value_type>;
using ConstStringRef = StringRefImpl<std::string::value_type const>;

inline std::ostream &operator<<(std::ostream &os, ConstStringRef const v) {
  for (auto c : v) {
    os.put(c);
  }
  return os;
}
inline std::ostream &operator<<(std::ostream &os, StringRef const v) {
  os << ConstStringRef{v};
  return os;
}
} // namespace nih
#endif // STRING_REF_H_
