#include <gtest/gtest.h>
#include "nih/uri.hh"

namespace nih {

TEST(FileScheme, Std) {
  testing::internal::CaptureStdout();
  StdOut.write("I'm std::cout.");
  auto output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output, "I'm std::cout.");

  testing::internal::CaptureStderr();
  StdErr.write("I'm std::cerr.\n").flush();
  output = testing::internal::GetCapturedStderr();
  ASSERT_EQ(output, "I'm std::cerr.\n");
}

}  // namespace nih