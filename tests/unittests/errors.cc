#include <gtest/gtest.h>

#include "nih/errors.hh"

namespace nih {

TEST(Errors, StackTrace) {
  StackTrace trace;
  auto str = trace.str();
  ASSERT_GT(trace.get().size(), 1);
  ASSERT_NE(str.find("StackTrace::StackTrace"), std::string::npos);

  try {
    throw NIHError("Error");
  } catch (NIHError const& e) {
    ASSERT_GT(e.trace().get().size(), 1);
    ASSERT_NE(e.trace().str().find("StackTrace::StackTrace"), std::string::npos);
  }
}

}  // namespace nih