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
  std::string const& str() { return _name; }
  virtual UriScheme& write(std::string input) = 0;
  virtual UriScheme& read(std::string* output, size_t size) = 0;

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

 public:
  Uri(std::string uri, std::string flags);
  std::string const& scheme() const {
    return _scheme_str;
  }
  std::string const& host() const {
    return _host_str;
  }

  bool isValid() { return _is_valid; }
  bool isAbsolute() { return true; }
  bool isOpaque() { return true; }

  Uri& write(std::string const& input) {
    initialize();
    _scheme->write(input);
    return *this;
  }
  Uri& read(std::string* output, size_t size) {
    initialize();
    _scheme->read(output, size);
    return *this;
  }

  std::string const& flags() const { return _flags; }
  UriErrorCode error() { return _code; }
};

class FileScheme : public UriScheme {
  std::string _path;
  std::string _flags;
  FILE* _fd;

 public:
  FileScheme(Uri const& uri, std::string flags);
  ~FileScheme();
  UriScheme& write(std::string input) override;
  UriScheme& read(std::string* output, size_t size) override;

  FILE* getDescriptor() { return _fd; }
};

extern Uri StdOut;
extern Uri StdErr;
extern Uri StdIn;

}  // namespace nih

#endif  // _URI_HH_