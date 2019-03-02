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
#include <locale>
#include <sstream>

#include "nih/json.hh"

namespace nih {
namespace json {

class JsonWriter {
  static constexpr size_t kIndentSize = 2;

  size_t n_spaces_;
  std::ostream* stream_;

  std::locale original_locale;

 public:
  JsonWriter(std::ostream* stream) : n_spaces_{0}, stream_{stream} {
    original_locale = std::locale("");
    stream_->imbue(std::locale("en_US.UTF-8"));
  }
  ~JsonWriter() {
    stream_->imbue(original_locale);
  }

  void newLine() {
    *stream_ << u8"\n" << std::string(n_spaces_, ' ');
  }

  void beginIndent() {
    n_spaces_ += kIndentSize;
  }
  void endIndent() {
    n_spaces_ -= kIndentSize;
  }

  void write(std::string str) {
    *stream_ << str;
  }

  void save(Json json) {
    json.ptr_->save(this);
  }
};

class JsonReader {
 private:
  struct SourceLocation {
    int cl_;      // current line
    int cc_;      // current column
    size_t pos_;  // current position in raw_str_

   public:
    SourceLocation() : cl_(0), cc_(0), pos_(0) {}

    int line() const { return cl_;  }
    int col()  const { return cc_;  }
    size_t pos()  const { return pos_; }

    SourceLocation& forward(char c=0) {
      if (c == '\n') {
        cc_ = 0;
        cl_++;
      } else {
        cc_++;
      }
      pos_++;
      return *this;
    }
  } cursor_;

  std::string raw_str_;

 private:
  void skipSpaces();

  char getNextChar() {
    if (cursor_.pos() == raw_str_.size()) {
      return -1;
    }
    char ch = raw_str_[cursor_.pos()];
    cursor_.forward();
    return ch;
  }

  char peekNextChar() {
    if (cursor_.pos() == raw_str_.size()) {
      return -1;
    }
    char ch = raw_str_[cursor_.pos()];
    return ch;
  }

  char getNextNonSpaceChar() {
    skipSpaces();
    return getNextChar();
  }

  char getChar(char c) {
    char result = getNextNonSpaceChar();
    if (result != c) { expect(c); }
    return result;
  }

  void error(std::string msg) const {
    std::istringstream str_s(raw_str_);

    msg += ", at ("
           + std::to_string(cursor_.line()) + ", "
           + std::to_string(cursor_.col()) + ")\n";
    std::string line;
    int line_count = 0;
    while (std::getline(str_s, line) && line_count < cursor_.line()) {
      line_count++;
    }
    msg+= line += '\n';
    std::string spaces (cursor_.col(), ' ');
    msg+= spaces + "^\n";

    throw std::runtime_error(msg);
  }

  // Report expected character
  void expect(char c) {
    std::string msg = "Expecting: \"";
    msg += std::to_string(c)
           + "\", got: \"" + raw_str_[cursor_.pos()-1] + "\"\n"; // FIXME
    error(msg);
  }

  Json parseString();
  Json parseObject();
  Json parseArray();
  Json parseNumber();
  Json parseBoolean();

  Json parse() {
    while (true) {
      skipSpaces();
      char c = peekNextChar();
      if (c == -1) { break; }

      if (c == '{') {
        return parseObject();
      } else if ( c == '[' ) {
        return parseArray();
      } else if ( c == '-' || std::isdigit(c)) {
        return parseNumber();
      } else if ( c == '\"' ) {
        return parseString();
      } else if ( c == 't' || c == 'f') {
        return parseBoolean();
      } else {
        error("Unknown construct");
      }
    }
    return Json();
  }

 private:
  std::locale original_locale_;
  std::istream* stream_;

 public:
  JsonReader(std::istream* stream) {
    original_locale_ = std::locale("");
    stream_ = stream;
    stream->imbue(std::locale("en_US.UTF-8"));
  }
  ~JsonReader(){
    stream_->imbue(original_locale_);
  }

  Json load() {
    stream_->imbue(std::locale("en_US.UTF-8"));
    raw_str_ = {std::istreambuf_iterator<char>(*stream_), {}};
    Json result = parse();
    stream_->imbue(original_locale_);
    return result;
  }
};

// Value
std::string Value::typeStr() const {
  switch (kind_) {
    case ValueKind::String: return "String";  break;
    case ValueKind::Number: return "Number";  break;
    case ValueKind::Object: return "Object";  break;
    case ValueKind::Array:  return "Array";   break;
    case ValueKind::Boolean:return "Boolean"; break;
    case ValueKind::Null:   return "Null";    break;
  }
  return "";
}

// Only used for keeping old compilers happy about non-reaching return
// statement.
Json& dummyJsonObject () {
  static Json obj;
  return obj;
}

// Json Object
JsonObject::JsonObject(std::map<std::string, Json> object)
    : Value(ValueKind::Object), object_{std::move(object)} {}

Json& JsonObject::operator[](std::string const & key) {
  return object_[key];
}

Json& JsonObject::operator[](int ind) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by Integer.");
  return dummyJsonObject();
}

bool JsonObject::operator==(Value const& rhs) const {
  if(!IsA<JsonObject>(&rhs)) { return false; }
  return object_ == Cast<JsonObject const>(&rhs)->getObject();
}

Value & JsonObject::operator=(Value const &rhs) {
  JsonObject const* casted = Cast<JsonObject const>(&rhs);
  object_ = casted->getObject();
  return *this;
}

void JsonObject::save(JsonWriter* writer) {
  writer->write("{");
  writer->beginIndent();
  writer->newLine();

  size_t i = 0;
  size_t size = object_.size();

  for (auto& value : object_) {
    writer->write("\"" + value.first + "\": ");
    writer->save(value.second);

    if (i != size-1) {
      writer->write(",");
      writer->newLine();
    }
    i++;
  }
  writer->endIndent();
  writer->newLine();
  writer->write("}");
}

// Json String
Json& JsonString::operator[](std::string const & key) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by string.");
  return dummyJsonObject();
}

Json& JsonString::operator[](int ind) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by Integer, " +
      "please try obtaining std::string first.");
  return dummyJsonObject();
}

bool JsonString::operator==(Value const& rhs) const {
  if (!IsA<JsonString>(&rhs)) { return false; }
  return Cast<JsonString const>(&rhs)->getString() == str_;
}

Value & JsonString::operator=(Value const &rhs) {
  JsonString const* casted = Cast<JsonString const>(&rhs);
  str_ = casted->getString();
  return *this;
}

// FIXME: UTF-8 parsing support.
void JsonString::save(JsonWriter* writer) {
  std::string buffer;
  buffer += '"';
  for (size_t i = 0; i < str_.length(); i++) {
    const char ch = str_[i];
    if (ch == '\\') {
      if (i < str_.size() && str_[i+1] == 'u') buffer += "\\";
      else buffer += "\\\\";
    } else if (ch == '"') {
      buffer += "\\\"";
    } else if (ch == '\b') {
      buffer += "\\b";
    } else if (ch == '\f') {
      buffer += "\\f";
    } else if (ch == '\n') {
      buffer += "\\n";
    } else if (ch == '\r') {
      buffer += "\\r";
    } else if (ch == '\t') {
      buffer += "\\t";
    } else if (static_cast<uint8_t>(ch) <= 0x1f) {
      // Unit separator
      char buf[8];
      snprintf(buf, sizeof buf, "\\u%04x", ch);
      buffer += buf;
    } else {
      buffer += ch;
    }
  }
  buffer += '"';
  writer->write(buffer);
}

// Json Array
Json& JsonArray::operator[](std::string const & key) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by string.");
  return dummyJsonObject();
}

Json& JsonArray::operator[](int ind) {
  return vec_.at(ind);
}

bool JsonArray::operator==(Value const& rhs) const {
  if (!IsA<JsonArray>(&rhs)) { return false; }
  auto& arr = Cast<JsonArray const>(&rhs)->getArray();
  return std::equal(arr.cbegin(), arr.cend(), vec_.cbegin());
}

Value & JsonArray::operator=(Value const &rhs) {
  JsonArray const* casted = Cast<JsonArray const>(&rhs);
  vec_ = casted->getArray();
  return *this;
}

void JsonArray::save(JsonWriter* writer) {
  writer->write("[");
  size_t size = vec_.size();
  for (size_t i = 0; i < size; ++i) {
    auto& value = vec_[i];
    writer->save(value);
    if (i != size-1) { writer->write(", "); }
  }
  writer->write("]");
}

// Json Number
Json& JsonNumber::operator[](std::string const & key) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by string.");
  return dummyJsonObject();
}

Json& JsonNumber::operator[](int ind) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by Integer.");
  return dummyJsonObject();
}

bool JsonNumber::operator==(Value const& rhs) const {
  if(!IsA<JsonNumber>(&rhs)) { return false; }
  return number_ == Cast<JsonNumber const>(&rhs)->getNumber();
}

Value & JsonNumber::operator=(Value const &rhs) {
  JsonNumber const* casted = Cast<JsonNumber const>(&rhs);
  number_ = casted->getNumber();
  return *this;
}

void JsonNumber::save(JsonWriter* writer) {
  writer->write(std::to_string(this->getNumber()));
}

// Json Null
Json& JsonNull::operator[](std::string const & key) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by string.");
  return dummyJsonObject();
}

Json& JsonNull::operator[](int ind) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by Integer.");
  return dummyJsonObject();
}

bool JsonNull::operator==(Value const& rhs) const {
  if(!IsA<JsonNull>(&rhs)) { return false; }
  return true;
}

Value & JsonNull::operator=(Value const &rhs) {
  Cast<JsonNull const>(&rhs);  // Checking only.
  return *this;
}

void JsonNull::save(JsonWriter* writer) {
  writer->write("null");
}

// Json Boolean
Json& JsonBoolean::operator[](std::string const & key) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by string.");
  return dummyJsonObject();
}

Json& JsonBoolean::operator[](int ind) {
  throw std::runtime_error(
      "Object of type " +
      Value::typeStr() + " can not be indexed by Integer.");
  return dummyJsonObject();
}

bool JsonBoolean::operator==(Value const& rhs) const {
  if(!IsA<JsonBoolean>(&rhs)) { return false; }
  return boolean_ == Cast<JsonBoolean const>(&rhs)->getBoolean();
}

Value & JsonBoolean::operator=(Value const &rhs) {
  JsonBoolean const* casted = Cast<JsonBoolean const>(&rhs);
  boolean_ = casted->getBoolean();
  return *this;
}

void JsonBoolean::save(JsonWriter *writer) {
  if (boolean_) {
    writer->write(u8"true");
  } else {
    writer->write(u8"false");
  }
}

// Json class
void JsonReader::skipSpaces() {
  while (cursor_.pos() < raw_str_.size()) {
    char c = raw_str_[cursor_.pos()];
    if (std::isspace(c)) {
      cursor_.forward(c);
    } else {
      break;
    }
  }
}

Json JsonReader::parseString() {
  char ch = getChar('\"');
  std::ostringstream output;
  std::string str;
  while (true) {
    ch = getNextChar();
    if (ch == '\\') {
      char next = static_cast<char>(getNextChar());
      switch (next) {
        case 'r':  str += u8"\r"; break;
        case 'n':  str += u8"\n"; break;
        case '\\': str += u8"\\"; break;
        case 't':  str += u8"\t"; break;
        case '\"': str += u8"\""; break;
        case 'u':
          str += ch;
          str += 'u';
          break;
        default: error("Unknown escape");
      }
    } else {
      if (ch == '\"') break;
      str += ch;
    }
    if (ch == EOF || ch == '\r' || ch == '\n') {
      expect('\"');
    }
  }
  return Json(std::move(str));
}

Json JsonReader::parseArray() {
  std::vector<Json> data;

  char ch = getChar('[');
  while (true) {
    if (peekNextChar() == ']') {
      getChar(']');
      return Json(std::move(data));
    }
    auto obj = parse();
    data.push_back(obj);
    ch = getNextNonSpaceChar();
    if (ch == ']') break;
    if (ch != ',') {
      expect(',');
    }
  }

  return Json(std::move(data));
}

Json JsonReader::parseObject() {
  char ch = getChar('{');

  std::map<std::string, Json> data;
  if (ch == '}') return Json(std::move(data));

  while(true) {
    skipSpaces();
    ch = peekNextChar();
    if (ch != '"') {
      expect('"');
    }
    Json key = parseString();

    ch = getNextNonSpaceChar();

    if (ch != ':') {
      expect(':');
    }

    Json value {parse()};
    data[Cast<JsonString>(&(key.getValue()))->getString()] = std::move(value);

    ch = getNextNonSpaceChar();

    if (ch == '}') break;
    if (ch != ',') {
      expect(',');
    }
  }

  return Json(std::move(data));
}

Json JsonReader::parseNumber() {
  std::string substr = raw_str_.substr(cursor_.pos(), 17);
  size_t pos = 0;
  double number = std::stod(substr, &pos);
  for (size_t i = 0; i < pos; ++i) {
    getNextChar();
  }
  return Json(number);
}

Json JsonReader::parseBoolean() {
  bool result = false;
  char ch = getNextNonSpaceChar();
  std::string const t_value = u8"true";
  std::string const f_value = u8"false";
  std::string buffer;

  if (ch == 't') {
    for (size_t i = 0; i < 3; ++i) {
      buffer.push_back(getNextNonSpaceChar());
    }
    if (buffer != u8"rue") {
      error("Expecting boolean value \"true\".");
    }
    result = true;
  } else {
    for (size_t i = 0; i < 4; ++i) {
      buffer.push_back(getNextNonSpaceChar());
    }
    if (buffer != u8"alse") {
      error("Expecting boolean value \"false\".");
    }
    result = false;
  }
  return Json{JsonBoolean{result}};
}

Json Json::load(std::istream* stream) {
  JsonReader reader(stream);
  try {
    Json json{reader.load()};
    return json;
  } catch (std::runtime_error const& e) {
    std::cerr << e.what();
    return Json();
  }
}

void Json::dump(Json json, std::ostream *stream) {
  JsonWriter writer(stream);
  try {
    writer.save(json);
  } catch (std::runtime_error const& e) {
    std::cerr << e.what();
  }
}

Json& Json::operator=(Json const &other) {
  auto type = other.getValue().type();
  switch (type) {
  case Value::ValueKind::Array:
    ptr_.reset(new JsonArray(*Cast<JsonArray const>(&other.getValue())));
    break;
  case Value::ValueKind::Boolean:
    ptr_.reset(new JsonBoolean(*Cast<JsonBoolean const>(&other.getValue())));
    break;
  case Value::ValueKind::Null:
    ptr_.reset(new JsonNull(*Cast<JsonNull const>(&other.getValue())));
    break;
  case Value::ValueKind::Number:
    ptr_.reset(new JsonNumber(*Cast<JsonNumber const>(&other.getValue())));
    break;
  case Value::ValueKind::Object:
    ptr_.reset(new JsonObject(*Cast<JsonObject const>(&other.getValue())));
    break;
  case Value::ValueKind::String:
    ptr_.reset(new JsonString(*Cast<JsonString const>(&other.getValue())));
    break;
  default:
    throw std::runtime_error("Unknown value kind.");
  }
  return *this;
}
}  // namespace json
}  // namespace nih