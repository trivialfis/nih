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
#if defined(__unix__)
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif  // defined(__unix__)
#include <cstdio>
#include "nih/errors.hh"
#include "nih/logging.hh"
#include "nih/uri.hh"

#include <fstream>
#include <iostream>

namespace nih {

FileScheme::FileScheme(Uri const &uri, std::string flags)
    : UriScheme{uri}, _fd{nullptr}, _flags{flags}, _borrowed{false}
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
  if (!_fd) {
    throw NIHError("Failed to open file: " + _path);
  }
}

FileScheme::FileScheme(Uri const& uri, FILE* fd, std::string flags)
    : UriScheme(uri), _fd{fd}, _flags{flags}, _borrowed{true} {}

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
  if (!_borrowed) {
    fclose(_fd);
  }
}

class StdScheme : public FileScheme {
  std::pair<FILE*, std::string> dispatch(std::string host) {
    if (host == "/dev/stdout") {
      return {stdout, "r"};
    } else if (host == "/dev/stderr") {
      return {stderr, "r"};
    } else if (host == "/dev/stdin") {
      return {stdin, "w"};
    } else {
      throw NIHError("Unknown device: " + host);
      return {nullptr, "r"};
    }
  }

 public:
  StdScheme(Uri const& uri, std::string flags) :
      FileScheme(uri, this->dispatch(uri.host()).first, this->dispatch(uri.host()).second) {}
};

FileScheme StdOut = StdScheme{Uri{"file:///dev/stdout"}, "w"};
FileScheme StdErr = StdScheme{Uri{"file:///dev/stderr"}, "w"};
FileScheme StdIn  = StdScheme{Uri{"file:///dev/stdin"}, "r"};

// bool isTTY(Uri const& uri) {
//   if (uri.scheme() == "std") {
//     auto p_scheme = uri.schemeObj();
//     if (p_scheme) {
//       FILE* fd = dynamic_cast<FileScheme*>(p_scheme.get())->descriptor();
//       return isatty(fileno(fd));
//     } else {
//       return false;
//     }
//   } else {
//     return false;
//   }
// }

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

std::string loadSequentialFile(Uri uri) {
  std::string const& fname = uri.host();
  auto OpenErr = [&fname]() {
                   std::string msg;
                   msg = "Opening " + fname + " failed: ";
                   msg += strerror(errno);
                   LOG(FATAL) << msg;
                 };
  auto ReadErr = [&fname]() {
                   std::string msg {"Error in reading file: "};
                   msg += fname;
                   msg += ": ";
                   msg += strerror(errno);
                   LOG(FATAL) << msg;
                 };

  std::string buffer;
#if defined(__unix__)
  struct stat fs;
  if (stat(fname.c_str(), &fs) != 0) {
    OpenErr();
  }

  size_t f_size_bytes = fs.st_size;
  buffer.resize(f_size_bytes+1);
  int32_t fd = open(fname.c_str(), O_RDONLY);
  posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
  ssize_t bytes_read = read(fd, &buffer[0], f_size_bytes);
  if (bytes_read < 0) {
    close(fd);
    ReadErr();
  }
  close(fd);
#else
  FILE *f = fopen(fname.c_str(), "r");
  if (f == NULL) {
    std::string msg;
    OpenErr();
  }
  fseek(f, 0, SEEK_END);
  auto fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  buffer.resize(fsize + 1);
  fread(&buffer[0], 1, fsize, f);
  fclose(f);
#endif  // defined(__unix__)
  return buffer;
}

}  // namespace nih