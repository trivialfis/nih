#ifndef _URI_HH_
#define _URI_HH_

#include <cinttypes>
#include <string>
#include <map>
#include <memory>
#include <nih/singleton.hh>
#include <nih/registry.hh>

namespace nih {

class Uri;

class UriScheme {
  std::string _name;
  using RegistryT = RegistryEntry<UriScheme*(Uri const&, std::string flags)>;

 public:
  UriScheme() = delete;
  UriScheme(std::string name) : _name{std::move(name)} {}

  virtual UriScheme& write(std::string input) = 0;
  virtual UriScheme& write(char* input, size_t size) = 0;
  virtual UriScheme& read(std::string* output, size_t size) = 0;
  virtual UriScheme& read(char* output, size_t size) = 0;

  virtual void flush() = 0;

  virtual std::string const& str() { return _name; }

  static UriScheme* create(std::string scheme, Uri const * const);
  static RegistryT& registry(
      std::string name,
      std::function<UriScheme*(Uri const&, std::string flags)> creator);
};

enum class UriErrorCode : uint8_t {
  kValid = 0,
  kEmpty = 1,
  kHost = 2
};

class Uri {
  std::string _uri;
  std::string _scheme_str;
  std::string _host_str;
  std::string _flags;

  bool _is_valid;
  UriErrorCode _code;

  std::shared_ptr<UriScheme> _scheme;

  void initialize() {
    if (!_scheme) {
      _scheme.reset(UriScheme::create(_scheme_str, this));
    }
  }

  template <typename Comp>
  static bool compare(Uri const& lhs, Uri const& rhs, Comp comp) {
    if (comp(lhs.scheme(), rhs.scheme())) {
      return true;
    }
    if (comp(lhs.host(), rhs.host())) {
      return true;
    }
    return false;
  }

 public:
  Uri(std::string uri, std::string flags);
  Uri(Uri const& that) = default;
  Uri(Uri&& that) = default;

  Uri& operator=(Uri const& that) {
    this->_uri = that._uri;
    this->_scheme_str = that._scheme_str;
    this->_host_str = that._host_str;
    this->_flags = that._flags;
    this->_is_valid = that._is_valid;
    this->_code = that._code;

    this->_scheme = that._scheme;
    return *this;
  }
  Uri& operator=(Uri&& that) {
    this->_uri = std::move(that._uri);
    this->_scheme_str = std::move(that._scheme_str);
    this->_host_str = std::move(that._host_str);
    this->_flags = std::move(that._flags);
    this->_is_valid = that._is_valid;
    this->_code = that._code;

    this->_scheme = std::move(that._scheme);
    return *this;
  }

  std::string const& scheme() const {
    return _scheme_str;
  }
  std::string const& host() const {
    return _host_str;
  }
  std::shared_ptr<UriScheme> schemeObj() const {
    return _scheme;
  }

  bool isValid() { return _is_valid; }
  bool isAbsolute() { return true; }
  bool isOpaque() { return true; }

  Uri& write(std::string const& input) {
    initialize();
    _scheme->write(input);
    return *this;
  }
  Uri& write(char* input, size_t size) {
    initialize();
    return *this;
  }
  Uri& read(std::string* output, size_t size) {
    initialize();
    _scheme->read(output, size);
    return *this;
  }
  Uri& read(char* output, size_t size) {
    initialize();
    return *this;
  }

  void flush() {
    initialize();
    _scheme->flush();
  }

  bool operator<(Uri const& that) const {
    return compare(*this, that,
                   [](std::string const& l, std::string const& r){
                     return l < r;
                   });
  }
  bool operator<=(Uri const& that) const {
    return compare(*this, that,
                   [](std::string const& l, std::string const& r){
                     return l <= r;
                   });
  }
  bool operator>(Uri const& that) const {
    return !(*this <= that);
  }
  bool operator>=(Uri const& that) const {
    return !(*this < that);
  }

  bool operator==(Uri const& that) const {
    return compare(*this, that,
                   [](std::string const& l, std::string const& r){
                     return l == r;
                   });
  }
  bool operator!=(Uri const& that) const {
    return !(*this == that);
  }

  std::string const& flags() const { return _flags; }
  UriErrorCode error() { return _code; }
};

class FileScheme : public UriScheme {
  std::string _path;
  std::string _flags;
  FILE* _fd;
 protected:
  FileScheme(Uri const& uri, std::string scheme, FILE* fd);

 public:
  FileScheme(Uri const& uri, std::string flags);
  ~FileScheme();
  UriScheme& write(std::string input) override;
  UriScheme& write(char* input, size_t size) override;
  UriScheme& read(std::string* output, size_t size) override;
  UriScheme& read(char* output, size_t size) override;

  void flush() override;

  FILE* descriptor() { return _fd; }
};

extern Uri StdOut;
extern Uri StdErr;
extern Uri StdIn;

bool isTTY(Uri const& uri);

template <typename Type>
std::string str(Type value) {
  return std::to_string(value);
}

// Adopted from googletest.
class CapturedStream {
 public:
  explicit CapturedStream(int fd);
  ~CapturedStream();

  std::string getCapturedString();

 private:
  const int _fd;  // A stream to capture.
  int _uncaptured_fd;
  // Name of the temporary file holding the stderr output.
  std::string _filename;
};

}  // namespace nih

#endif  // _URI_HH_