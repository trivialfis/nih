#ifndef _ERRORS_HH_
#define _ERRORS_HH_

#include <exception>
#include <string>

namespace nih {
class NIHError : public std::exception {
  std::string error_;

 public:
  explicit NIHError(std::string const& what_arg) : error_{what_arg} {}
  explicit NIHError(char const* what_arg) : error_{what_arg} {}

  virtual const char* what() const noexcept {  // NOLINT
    return error_.c_str();
  }
  virtual ~NIHError() = default;
};
}  // namespace nih

#endif  // _ERRORS_HH_