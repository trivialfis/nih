#include <gtest/gtest.h>
#include "nih/uri.hh"

namespace nih {

TEST(FileScheme, Std) {
  {
    auto out {CapturedStream(STDOUT_FILENO)};
    StdOut.write("I'm std::cout.\n");
    auto output = out.getCapturedString();
    ASSERT_EQ(output, "I'm std::cout.\n");
  }

  {
    auto err {CapturedStream(STDERR_FILENO)};
    StdErr.write("I'm std::cerr.\n").flush();
    auto output = err.getCapturedString();
    ASSERT_EQ(output, "I'm std::cerr.\n");
  }

  ASSERT_EQ(isTTY(StdOut), isatty(fileno(stdout)));
  ASSERT_EQ(isTTY(StdErr), isatty(fileno(stderr)));
}

}  // namespace nih
