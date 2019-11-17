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
  std::vector<std::string::value_type> buffer_;
  char f2s_buffer_[NumericLimits<float>::kDigit10];
  char i2s_buffer_[NumericLimits<int64_t>::kDigit10];

 public:
  void HandleNull() {
    auto s = buffer_.size();
    buffer_.resize(s + 4);
    buffer_[s + 0] = 'n';
    buffer_[s + 1] = 'u';
    buffer_[s + 2] = 'l';
    buffer_[s + 3] = 'l';
  }
  void HandleTrue() {
    auto s = buffer_.size();
    buffer_.resize(s + 4);
    buffer_[s + 0] = 't';
    buffer_[s + 1] = 'r';
    buffer_[s + 2] = 'u';
    buffer_[s + 3] = 'e';
  }
  void HandleFalse() {
    auto s = buffer_.size();
    buffer_.resize(s + 5);
    buffer_[s + 0] = 'f';
    buffer_[s + 1] = 'a';
    buffer_[s + 2] = 'l';
    buffer_[s + 3] = 's';
    buffer_[s + 4] = 'e';
  }

  void BeginObject() { buffer_.emplace_back('{'); }
  void EndObject() { buffer_.emplace_back('}'); }
  void KeyValue() { buffer_.emplace_back(':'); }
  void Comma() { buffer_.emplace_back(','); }

  void BeginArray() { buffer_.emplace_back('['); }
  void EndArray() { buffer_.emplace_back(']'); }


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

    auto s = buffer_.size();
    buffer_.resize(s + buffer.size());
    std::memcpy(buffer_.data() + s, buffer.data(), buffer.size());
  }
  void HandleFloat(float f) {
    // {
    //   char buffer[24];
    //   size_t len = fpconv_dtoa(f, buffer);
    //   auto ori_size = _buffer.size();
    //   _buffer.resize(_buffer.size() + len);
    //   std::memcpy(_buffer.data() + ori_size, buffer, len);
    // }
    {
      auto ret = to_chars(f2s_buffer_, f2s_buffer_ + NumericLimits<float>::kDigit10, f);
      auto end = ret.ptr;
      NIH_ASSERT(ret.ec == std::errc());
      auto out_size = end - f2s_buffer_;
      auto ori_size = buffer_.size();
      buffer_.resize(buffer_.size() + out_size);
      std::memcpy(buffer_.data() + ori_size, f2s_buffer_, end - f2s_buffer_);
    }
  }
  void HandleInteger(int64_t i) {
    auto ret = to_chars(i2s_buffer_, i2s_buffer_ + NumericLimits<int64_t>::kDigit10, i);
    auto end = ret.ptr;
    NIH_ASSERT(ret.ec == std::errc());
    auto digits = std::distance(i2s_buffer_, end);
    auto ori_size = buffer_.size();
    buffer_.resize(ori_size + digits);
    std::memcpy(buffer_.data() + ori_size, i2s_buffer_, digits);
  }

  std::vector<std::string::value_type> const& write(Value const& value) {
    return buffer_;
  }

  void TakeResult(std::string *str) {
    str->resize(buffer_.size());
    std::copy(buffer_.cbegin(), buffer_.cend(), str->begin());
  }
};

}  // namespace experimental
}  // namespace nih

#endif  // _NIH_JSON_WRITER_HH_