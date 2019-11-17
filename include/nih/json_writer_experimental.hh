#ifndef _NIH_JSON_WRITER_HH_
#define _NIH_JSON_WRITER_HH_

#include <cinttypes>
#include <nih/json_experimental.hh>
#include <nih/charconv.hh>
#include <nih/logging.hh>
#include <nih/fpconv.hh>

namespace nih {
namespace experimental {

template <typename T>
struct NumericLimits;

template <> struct NumericLimits<int64_t> {
  static constexpr size_t kDigit10 = 21;
};

template <> struct NumericLimits<float> {
  static constexpr size_t kDigit10 = 16;
};

class JsonWriter {
  std::vector<std::string::value_type> _buffer;
  char f2s_buffer[NumericLimits<float>::kDigit10];
  char i2s_buffer[NumericLimits<int64_t>::kDigit10];

 public:
  void HandleNull() {
    auto s = _buffer.size();
    _buffer.resize(s + 4);
    _buffer[s + 0] = 'n';
    _buffer[s + 1] = 'u';
    _buffer[s + 2] = 'l';
    _buffer[s + 3] = 'l';
  }
  void HandleTrue() {
    auto s = _buffer.size();
    _buffer.resize(s + 4);
    _buffer[s + 0] = 't';
    _buffer[s + 1] = 'r';
    _buffer[s + 2] = 'u';
    _buffer[s + 3] = 'e';
  }
  void HandleFalse() {
    auto s = _buffer.size();
    _buffer.resize(s + 5);
    _buffer[s + 0] = 'f';
    _buffer[s + 1] = 'a';
    _buffer[s + 2] = 'l';
    _buffer[s + 3] = 's';
    _buffer[s + 4] = 'e';
  }
  void visit_object() {
    auto s = _buffer.size();
    _buffer.resize(s + 5);  // at least 5 bytes
    _buffer[s] = '{';
  }

  void BeginObject() { _buffer.emplace_back('{'); }
  void EndObject() { _buffer.emplace_back('}'); }
  void KeyValue() { _buffer.emplace_back(':'); }
  void Comma() { _buffer.emplace_back(','); }

  void BeginArray() { _buffer.emplace_back('['); }
  void EndArray() { _buffer.emplace_back(']'); }


  void HandleString(ConstStringRef string) {
    std::string buffer;
    buffer.reserve(buffer.size() + string.size());

    buffer += '"';
    for (size_t i = 0; i < string.size(); i++) {
      const char ch = string[i];
      if (ch == '\\') {
        if (i < string.size() && string[i + 1] == 'u') {
          buffer += "\\";
        } else {
          buffer += "\\\\";
        }
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

    auto s = _buffer.size();
    _buffer.resize(s + buffer.size());
    std::memcpy(_buffer.data() + s, buffer.data(), buffer.size());
  }
  void HandleFloat(float f) {
    // {
    //   char buffer[24];
    //   size_t len = fpconv_dtoa(f, buffer);
    //   // LOG_VAR(len);
    //   auto ori_size = _buffer.size();
    //   _buffer.resize(_buffer.size() + len);
    //   std::memcpy(_buffer.data() + ori_size, buffer, len);
    // }
    {
      auto ret = to_chars(f2s_buffer, f2s_buffer + NumericLimits<float>::kDigit10, f);
      auto end = ret.ptr;
      NIH_ASSERT(ret.ec == std::errc());
      auto out_size = end - f2s_buffer;
      auto ori_size = _buffer.size();
      _buffer.resize(_buffer.size() + out_size);
      std::memcpy(_buffer.data() + ori_size, f2s_buffer, end - f2s_buffer);
    }
  }
  void HandleInteger(int64_t i) {
    auto ret = to_chars(i2s_buffer, i2s_buffer + NumericLimits<int64_t>::kDigit10, i);
    auto end = ret.ptr;
    NIH_ASSERT(ret.ec == std::errc());
    auto digits = std::distance(i2s_buffer, end);
    auto ori_size = _buffer.size();
    _buffer.resize(ori_size + digits);
    std::memcpy(_buffer.data() + ori_size, i2s_buffer, digits);
  }

  std::vector<std::string::value_type> const& write(Value const& value) {
    return _buffer;
  }

  void TakeResult(std::string *str) {
    str->resize(_buffer.size());
    std::copy(_buffer.cbegin(), _buffer.cend(), str->begin());
  }
};

}  // namespace experimental
}  // namespace nih

#endif  // _NIH_JSON_WRITER_HH_