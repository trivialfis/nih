#include <nih/strings.hh>
#include <vector>
#include <string>
#include <iostream>

int main() {
  std::vector<std::string> result;
  nih::split(result, "hello world", [](char c) { return c == ' '; });
  // outputs:
  // hello
  // world
  for (auto const &str : result) {
    std::cout << str << std::endl;
  }
  return 0;
}