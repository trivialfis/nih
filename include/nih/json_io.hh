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
#ifndef _JSON_IO_HH_
#define _JSON_IO_HH_

#include <memory>
#include <string>
#include <cinttypes>
#include <utility>
#include <map>
#include <limits>
#include <sstream>
#include <locale>

#include <nih/json.hh>

namespace nih {

template <typename Allocator>
class FixedPrecisionStreamContainer : public std::basic_stringstream<
  char, std::char_traits<char>, Allocator> {
 public:
  FixedPrecisionStreamContainer() {
    this->precision(std::numeric_limits<Number::Float>::max_digits10);
    this->imbue(std::locale("C"));
    this->setf(std::ios::scientific);
  }
};

using FixedPrecisionStream = FixedPrecisionStreamContainer<std::allocator<char>>;

/*
 * \brief A json reader, currently error checking and utf-8 is not fully supported.
 */
class JsonReader {
 protected:
  size_t constexpr static kMaxNumLength =
      std::numeric_limits<double>::max_digits10 + 1;

  struct SourceLocation {
    size_t pos_;  // current position in raw_str_

   public:
    SourceLocation() : pos_(0) {}
    size_t  Pos()  const { return pos_; }

    SourceLocation& Forward() {
      pos_++;
      return *this;
    }
    SourceLocation& Forward(uint32_t n) {
      pos_ += n;
      return *this;
    }
  } cursor_;

  StringView raw_str_;

 protected:
  void SkipSpaces();

  char GetNextChar() {
    if (cursor_.Pos() == raw_str_.size()) {
      return -1;
    }
    char ch = raw_str_[cursor_.Pos()];
    cursor_.Forward();
    return ch;
  }

  char PeekNextChar() {
    if (cursor_.Pos() == raw_str_.size()) {
      return -1;
    }
    char ch = raw_str_[cursor_.Pos()];
    return ch;
  }

  char GetNextNonSpaceChar() {
    SkipSpaces();
    return GetNextChar();
  }

  char GetChar(char c) {
    char result = GetNextNonSpaceChar();
    if (result != c) { Expect(c, result); }
    return result;
  }

  void Error(std::string msg) const;

  // Report expected character
  void Expect(char c, char got) {
    std::string msg = "Expecting: \"";
    msg += c;
    msg += "\", got: \"";
    msg += std::string {got} + " \"";
    Error(msg);
  }

  virtual Json ParseString();
  virtual Json ParseObject();
  virtual Json ParseArray();
  virtual Json ParseNumber();
  virtual Json ParseBoolean();
  virtual Json ParseNull();

  Json Parse();

 public:
  explicit JsonReader(StringView str) :
      raw_str_{str} {}

  virtual ~JsonReader() = default;

  Json Load();
};

class JsonWriter {
  static constexpr size_t kIndentSize = 2;
  FixedPrecisionStream convertor_;

  size_t n_spaces_;
  std::ostream* stream_;
  bool pretty_;

 public:
  JsonWriter(std::ostream* stream, bool pretty) :
      n_spaces_{0}, stream_{stream}, pretty_{pretty} {}

  virtual ~JsonWriter() = default;

  void NewLine() {
    if (pretty_) {
      *stream_ << u8"\n" << std::string(n_spaces_, ' ');
    }
  }

  void BeginIndent() {
    n_spaces_ += kIndentSize;
  }
  void EndIndent() {
    n_spaces_ -= kIndentSize;
  }

  void Write(std::string str) {
    *stream_ << str;
  }
  void Write(StringView str) {
    stream_->write(str.c_str(), str.size());
  }

  void Save(Json json);

  virtual void Visit(JsonArray  const* arr);
  virtual void Visit(JsonObject const* obj);
  virtual void Visit(JsonNumber const* num);
  virtual void Visit(JsonInteger const* num);
  virtual void Visit(JsonNull   const* null);
  virtual void Visit(JsonString const* str);
  virtual void Visit(JsonBoolean const* boolean);
};
}  // namespace nih

#endif  // _JSON_IO_HH_