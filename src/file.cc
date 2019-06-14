#include <unistd.h>
#include <cstdio>
#include "nih/errors.hh"
#include "nih/uri.hh"

#include <fstream>

namespace nih {

FileScheme::FileScheme(Uri const &uri, std::string flags)
    : UriScheme{"file"}, _fd{nullptr}, _flags{flags}
{
  if (uri.scheme() != "file") {
    throw NIHError{"Not a file uri: " + uri.scheme()};
  }
  if (_flags.size() > 3) {
    throw NIHError{"Invalid length of flags, expecting <= 3, get: " +
          std::to_string(flags.size())};
  }
  for (auto c : flags) {
    if (c != 'r' && c != 'w' && c != '+') {
      throw NIHError{"Invalid flag: " + std::to_string(c)};
    }
  }
  _path = uri.host();
  _fd = fopen(_path.c_str(), _flags.c_str());
}

FileScheme::FileScheme(Uri const& uri, std::string scheme, FILE* fd)
    : UriScheme(scheme) {
  _fd = fd;
  _path = uri.host();
}

UriScheme& FileScheme::write(std::string input) {
  fprintf(_fd, "%s", input.c_str());
  return *this;
}

UriScheme& FileScheme::write(char* input, size_t size) {
  fwrite(input, size, 1, _fd);
  return *this;
}

UriScheme& FileScheme::read(std::string* output, size_t size) {
  if (output->size() < size) {
    output->resize(size);
  }
  fgets(&(*output)[0], size, (FILE*)_fd);
  return *this;
}

UriScheme& FileScheme::read(char* output, size_t size) {
  fread(output, size, 1, _fd);
  return *this;
}

void FileScheme::flush() {
  fflush(_fd);
}

FileScheme::~FileScheme() {
  this->flush();
  fclose(_fd);
}

class StdScheme : public FileScheme {
  FILE* dispatch(std::string descriptor) {
    if (descriptor == "/dev/stdout") {
      return stdout;
    } else if (descriptor == "/dev/stderr") {
      return stderr;
    } else if (descriptor == "/dev/stdin") {
      return stdin;
    } else {
      throw NIHError("Unknown device: " + descriptor);
      return nullptr;
    }
  }

 public:
  StdScheme(Uri const& uri, std::string flags) :
      FileScheme(uri, "fd", this->dispatch(uri.host())) {}
};

Uri StdOut = Uri{"std:/dev/stdout", "w"};
Uri StdIn  = Uri{"std:/dev/stdin" , "r"};
Uri StdErr = Uri{"std:/dev/stderr", "w"};

bool isTTY(Uri const& uri) {
  if (uri.scheme() == "std") {
    auto p_scheme = uri.schemeObj();
    if (p_scheme) {
      FILE* fd = dynamic_cast<FileScheme*>(p_scheme.get())->descriptor();
      return isatty(fileno(fd));
    } else {
      return false;
    }
  } else {
    return false;
  }
}

CapturedStream::CapturedStream(int fd) : _fd(fd), _uncaptured_fd(dup(fd)) {
  char name_template[] = "/tmp/captured_stream.XXXXXX";
  const int captured_fd = mkstemp(name_template);
  if (captured_fd == -1) {
    fprintf(stderr, "Failed to create tmp file: %s\n", name_template);
  }
  _filename = name_template;
  fflush(nullptr);
  dup2(captured_fd, _fd);
  int status = close(captured_fd);
  if (status != 0) {
    fprintf(stderr, "Failed to close: %d\n", captured_fd);
  }
  fflush(nullptr);
}

CapturedStream::~CapturedStream() {
  std::remove(_filename.c_str());
}

std::string CapturedStream::getCapturedString() {
  if (_uncaptured_fd != -1) {
    // Restores the original stream.
    fflush(nullptr);
    dup2(_uncaptured_fd, _fd);
    close(_uncaptured_fd);
    _uncaptured_fd = -1;
  }

  std::ifstream fin{_filename};
  if (!fin) {
    throw NIHError("Failed to open tmp file: " + _filename + " for capturing stream.");
  }
  std::string content{std::istreambuf_iterator<char>(fin),
                      std::istreambuf_iterator<char>()};
  return content;
}

}  // namespace nih

static __attribute__((unused)) auto const& __file_uri_scheme_registry__ =
    nih::UriScheme::registry(
        "file",
        [](nih::Uri const& uri, std::string flags){
          return static_cast<nih::UriScheme*>(
              new nih::FileScheme(uri, flags));
        }).describe("File scheme.");

static __attribute__((unused)) auto const& __descriptor_uri_scheme_registry__ =
    nih::UriScheme::registry(
        "std",
        [](nih::Uri const& uri, std::string flags){
          return static_cast<nih::UriScheme*>(
              new nih::StdScheme(uri, flags));
        }).describe("File scheme.");