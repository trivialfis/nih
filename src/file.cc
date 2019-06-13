#include <nih/errors.hh>
#include <nih/uri.hh>

namespace nih {

Uri StdOut = Uri{"file:/dev/stdout", "w"};
Uri StdIn  = Uri{"file:/dev/stdin", "r"};
Uri StdErr = Uri{"file:/dev/stderr", "w"};

FileScheme::FileScheme(Uri const& uri, std::string flags) : _fd{nullptr}, _flags{flags} {
  if (uri.scheme() != "file") {
    throw NIHError{"Not a file uri: " + uri.scheme()};
  }
  if (_flags.size() > 3) {
    throw NIHError{
      "Invalid length of flags, expecting <= 3, get: " + std::to_string(flags.size())};
  }
  for (auto c : flags) {
    if (c != 'r' && c != 'w' && c != '+') {
      throw NIHError {
        "Invalid flag: " + std::to_string(c)
      };
    }
  }
  _path = uri.host();
  _fd = fopen(_path.c_str(), _flags.c_str());
}

UriScheme& FileScheme::write(std::string input) {
  fprintf(_fd, "%s", input.c_str());
  return *this;
}

UriScheme& FileScheme::read(std::string* output, size_t size) {
  if (output->size() < size) {
    output->resize(size);
  }
  fgets(&(*output)[0], size, (FILE*)_fd);
  return *this;
}

FileScheme::~FileScheme() {
  fclose(_fd);
}

}  // namespace nih

static __attribute__((unused)) auto const& __file_uri_scheme_registry__ =
    nih::UriScheme::registry(
        "file",
        [](nih::Uri const& uri, std::string flags){
          return static_cast<nih::UriScheme*>(
              new nih::FileScheme(uri, flags));
        }).describe("File scheme.");