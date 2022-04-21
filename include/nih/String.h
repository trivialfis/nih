#include <string>
#include <vector>

namespace tungsten {
template <typename Functor>
void split(std::string const &input, Functor func, std::vector<std::string> *result) {
  size_t last_pos = 0;
  size_t len = 0;
  for (auto c : input) {
    if (!func(c)) {
      len++;
    } else {
      result->push_back(input.substr(last_pos, len));
      last_pos += len + 1;
      len = 0;
    }
  }
  if (input.size() - last_pos > 0) {
    result->push_back(input.substr(last_pos));
  }
}

inline void split(std::string const &input, char delimiter, std::vector<std::string> *result) {
  split(input, [delimiter](char c) { return c == delimiter; }, result);
}
} // namespace tungsten
