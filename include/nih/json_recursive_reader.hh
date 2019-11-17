#ifndef JSON_RECURSIVE_PARSER_HH_
#define JSON_RECURSIVE_PARSER_HH_

#include <nih/json_experimental.hh>
#include <nih/luts.hh>
#include <nih/span.hh>
#include <nih/logging.hh>
#include <nih/charconv.hh>

#include <iomanip>
#include <map>
#include <cmath>

namespace nih {
namespace experimental {

double FastPath(double significand, int exp) {
  if (exp < -308) {
    return 0.0;
  } else if (exp >= 0) {
    return significand * NihExp10(exp);
  } else {
    return significand / NihExp10(-exp);
  }
}

constexpr double FastPathLimit() {
  return static_cast<double>((static_cast<uint64_t>(1) << 53) - 1);
}

#define JSON_PARSER_ASSERT_RETURN(__ret)                                       \
  {                                                                            \
    if (errno_ != jError::kSuccess) {                                          \
      return __ret;                                                            \
    }                                                                          \
  }

inline double Strtod(double significand, int exp, char *beg, char *end) {
  double result{std::numeric_limits<double>::quiet_NaN()};
  // The technique is picked up from rapidjson, they implemented a big integer
  // type for slow full precision, here we just use strtod for slow parsing.
  // Fast path:
  // http://www.exploringbinary.com/fast-path-decimal-to-floating-point-conversion/
  if (exp > 22 && exp < 22 + 16) {
    // Fast path cases in disguise
    significand *= NihExp10(exp - 22);
    exp = 22;
  }

  if (exp >= -22 && exp <= 22 && significand <= FastPathLimit()) { // 2^53 - 1
    result = FastPath(significand, exp);
    return result;
  }
  result = std::strtod(beg, &end);
  return result;
}

class JsonRecursiveReader {
  using Cursor = StringRef::iterator;

  ValueImpl* handler_;
  StringRef input_;
  Cursor cursor_;
  jError errno_{jError::kSuccess};

  bool IsSpace(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
  }

  Cursor SkipWhitespaces(Cursor p) {
    for (;;) {
      if (NIH_EXPECT(p == input_.cend(), false)) {
        return 0;
      } else if (IsSpace(*p)) {
        ++p;
      } else {
        return p;
      }
    }
  }

  void HandleNull(ValueImpl* number) { number->SetNull(); }
  void HandleTrue(ValueImpl* t) { t->SetTrue(); }
  void HandleFalse(ValueImpl* f) { f->SetFalse(); }

  /*\brief Guess whether parsed value is floating point or integer.  For value
   * produced by nih json this should always be correct as ryu produces `E` in
   * all floating points. */
  void HandleNumber(ValueImpl* number) {
    Cursor const beg = cursor_; // keep track of current pointer

    bool negative = false;
    switch (*cursor_) {
    case '-': {
      negative = true;
      ++cursor_;
      break;
    }
    case '+': {
      negative = false;
      ++cursor_;
      break;
    }
    default: {
      break;
    }
    }

    bool is_float = false;

    int64_t i = 0;
    double f = 0.0;

    if (*cursor_ == '0') {
      i = 0;
      cursor_++;
    }

    while (NIH_EXPECT(*cursor_ >= '0' && *cursor_ <= '9', true)) {
      i = i * 10 + (*cursor_ - '0');
      cursor_++;
    }

    int exp_frac {0};  // fraction of exponent

    if (*cursor_ == '.') {
      cursor_++;
      is_float = true;

      while (*cursor_ >= '0' && *cursor_ <= '9') {
        if (i > FastPathLimit()) {
          break;
        } else {
          i = i * 10 + (*cursor_ - '0');
          exp_frac --;
        }
        cursor_ ++;
      }

      f = static_cast<double>(i);
    }

    int exp {0};

    if (*cursor_ == 'E' || *cursor_ == 'e') {
      is_float = true;
      bool negative_exp {false};
      cursor_++;

      switch (*cursor_) {
      case '-': {
        negative_exp = true;
        cursor_++;
        break;
      }
      case '+': {
        cursor_++;
        break;
      }
      default:
        break;
      }

      if (NIH_EXPECT(*cursor_ >= '0' && *cursor_ <= '9', true)) {
        exp = *cursor_ - '0';
        cursor_ ++;
        while (*cursor_ >= '0' && *cursor_ <= '9') {
          exp = exp * 10 + static_cast<int>(*cursor_ - '0');
          cursor_++;
        }
      } else {
        errno_ = jError::kInvalidNumber;
        return;;
      }

      if (negative_exp) {
        exp = -exp;
      }
    }

    if (negative) {
      i = -i;
    }
    if (is_float) {
      f = Strtod(i, exp + exp_frac, beg, cursor_);
      number->SetFloat(f);
    } else {
      number->SetInteger(i);
    }
  }

  ConstStringRef HandleString() {
    auto ret = this->Skip(&cursor_, '\"');
    if (NIH_EXPECT(!ret, false)) {
      errno_ = jError::kInvalidObject;
      return {0, 0};
    }

    auto const beg = cursor_;
    bool still_parsing = true;
    bool has_escape = false;

    while (still_parsing) {
      char ch = *cursor_;
      if (NIH_EXPECT(ch >= 0 && ch < 0x20, false)) {
        errno_ = jError::kInvalidString;
        return {};
      }

      switch (ch) {
      case '"': {
        still_parsing = false;
        break;
      }
      case '\\': {
        has_escape = true;

        if (NIH_EXPECT(cursor_ + 1 == input_.cbegin(), false)) {
          errno_ = jError::kInvalidString;
          return {};
        }

        *cursor_ = '\0';  // mark invalid
        cursor_++;
        ch = *cursor_;

        switch (ch) {
        case 'r': {
          *cursor_ = '\r'; break;
        }
        case 't': {
          *cursor_ = '\t'; break;
          break;
        }
        case 'n': {
          *cursor_ = '\n'; break;
          break;
        }
        case 'b': {
          *cursor_ = '\b'; break;
        }
        case 'f': {
          *cursor_ = '\f'; break;
        }
        case '\\': {
          *cursor_ = '\\'; break;
          break;
        }
        case '\"': {
          *cursor_ = '\"'; break;
          break;
        }
        case '/': {
          *cursor_ = '/';
          break;
        }
        }
        cursor_++;
        break;
      }
      default: {
        cursor_++;
        break;
      }
      }

      if (NIH_EXPECT(!still_parsing, false)) {
        break;
      }
    }

    auto end = cursor_;
    if (NIH_EXPECT(has_escape, false)) {
      Cursor last = beg;
      for (auto it = beg; it != cursor_; ++it) {
        if (NIH_EXPECT(*it == '\0', false)) {
          continue;
        }
        *last = *it;
        last++;
      }
      end = last;
    }

    if (NIH_EXPECT(!this->Skip(&cursor_, '"'), false)) {
      errno_ = jError::kInvalidString;
      return ConstStringRef{};
    }

    auto length = std::distance(beg, end);
    return ConstStringRef {beg, static_cast<size_t>(length)};
  }

  bool Skip(Cursor* p_cursor, char c) {
    auto cursor = *p_cursor;
    auto o = *cursor;
    (*p_cursor) ++;
    return c == o;
  }

  void HandleArray(ValueImpl* value) {
    if (NIH_EXPECT(!this->Skip(&cursor_, '['), false)) {
      errno_ = jError::kInvalidArray;
      return;
    }
    value->SetArray();

    cursor_ = this->SkipWhitespaces(cursor_);

    char ch = *cursor_;
    if (ch == ']') {
      this->Skip(&cursor_, ']');
      return;
    }

    while (true) {
      cursor_ = this->SkipWhitespaces(cursor_);

      auto elem = value->CreateArrayElem();
      this->ParseImpl(&elem);
      JSON_PARSER_ASSERT_RETURN();

      cursor_ = this->SkipWhitespaces(cursor_);

      if (*cursor_ == ']') {
        this->Skip(&cursor_, ']');
        break;
      }

      if (NIH_EXPECT(!this->Skip(&cursor_, ','), false)) {
        errno_ = jError::kInvalidArray;
        return;
      }
    }

    size_t offset = 1;
  }

  void HandleObject(ValueImpl* object) {
    this->Skip(&cursor_, '{');
    cursor_ = SkipWhitespaces(cursor_);
    char ch = *cursor_;
    object->SetObject();

    if (ch == '}') {
      this->Skip(&cursor_, '}');
      return;
    }

    while (true) {
      ValueImpl::ObjectElement elem;

      cursor_ = SkipWhitespaces(cursor_);
      ch = *cursor_;

      ConstStringRef key = HandleString();
      JSON_PARSER_ASSERT_RETURN();

      auto value = object->CreateMember(key);

      cursor_ = this->SkipWhitespaces(cursor_);
      if (NIH_EXPECT(!this->Skip(&cursor_, ':'), false)) {
        errno_ = jError::kInvalidObject;
        return;
      }

      this->ParseImpl(&value);
      JSON_PARSER_ASSERT_RETURN();

      cursor_ = this->SkipWhitespaces(cursor_);

      ch = *cursor_;
      if (ch == '}') {
        break;
      }
      NIH_ASSERT(this->Skip(&cursor_, ',')) << *cursor_;
    }

    NIH_ASSERT(this->Skip(&cursor_, '}'));
    return;
  }

 public:
   void ParseImpl(ValueImpl* value) {
     cursor_ = this->SkipWhitespaces(cursor_);
     if (cursor_ == input_.data() + input_.size()) {
       return;
     }
     char c = *cursor_;

     switch (c) {
     case '{': {
       HandleObject(value);
     } break;
     case '[': {
       HandleArray(value);
       break;
     }
     case '-':
     case '0':
     case '1':
     case '2':
     case '3':
     case '4':
     case '5':
     case '6':
     case '7':
     case '8':
     case '9': {
       HandleNumber(value);
       break;
     }
     case '\"': {
       ConstStringRef str = HandleString();
       value->SetString(str);
       break;
     }
     case 't': {
       HandleTrue(value);
       break;
     }
     case 'f': {
       HandleFalse(value);
       break;
     }
     case 'n': {
       HandleNull(value);
       break;
     }
     default:
       errno_ = jError::kUnknownConstruct;
       return;
     }

     return;
   }

  std::pair<jError, size_t> Parse() {
     this->ParseImpl(handler_);
     return {errno_, std::distance(input_.cbegin(), cursor_)};
   }

   JsonRecursiveReader(StringRef str, ValueImpl* handler)
       : input_{str}, cursor_{input_.begin()}, handler_{handler} {
   }
};

}      // namespace experimental
}      // namespace nih
#endif  // JSON_RECURSIVE_PARSER_HH_