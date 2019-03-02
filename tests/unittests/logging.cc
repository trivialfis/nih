#include <gtest/gtest.h>

#include "nih/logging.hh"

namespace nih {

TEST(Basic, Logging) {
  ASSERT_THROW((LOG(FATAL) << "Fatal"), NIHError);

  try {
    LOG(FATAL) << "Fatal";
  } catch(NIHError& e) {
    std::string error_msg = e.what();
    ASSERT_TRUE(error_msg.find("Fatal"));
  }

  EXPECT_ANY_THROW(LOG(USER_E) << "User");

  std::string output;
  {
    testing::internal::CaptureStderr();
    LOG(WARNING) << "Log warning.";
    output = testing::internal::GetCapturedStderr();
    ASSERT_NE(output.find("Log warning."), std::string::npos);
    ASSERT_NE(output.find("[WARNING]"), std::string::npos);
  }

  {
    Log::setGlobalVerbosity(Log::ErrorType::kInfo);
    testing::internal::CaptureStdout();
    LOG(INFO) << "Log info.";
    output = testing::internal::GetCapturedStdout();
    ASSERT_NE(output.find("Log info."), std::string::npos);
    ASSERT_NE(output.find("[INFO]"), std::string::npos);
  }

  {
    Log::setGlobalVerbosity(Log::ErrorType::kDebug);
    testing::internal::CaptureStdout();
    LOG(DEBUG) << "Log debug.";
    output = testing::internal::GetCapturedStdout();
    ASSERT_NE(output.find("Log debug."), std::string::npos);
    ASSERT_NE(output.find("[DEBUG]"), std::string::npos);
  }

  {
    // Don't display anything with lower verbosity
    Log::setGlobalVerbosity(Log::defaultVerbosity());
    testing::internal::CaptureStdout();
    LOG(INFO) << "Log info.";
    output = testing::internal::GetCapturedStdout();
    ASSERT_EQ(output.find("Log info."), std::string::npos);
  }

  Log::setGlobalVerbosity(Log::ErrorType::kInfo);
  int var = 17;
  testing::internal::CaptureStdout();
  LOG_VAR(var);
  output = testing::internal::GetCapturedStdout();
  ASSERT_NE(output.find("17"), std::string::npos);
}

}  // namespace nih