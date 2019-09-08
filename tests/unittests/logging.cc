#include <gtest/gtest.h>
#include <cstdio>
#include <fstream>

#include "nih/logging.hh"
#include "nih/uri.hh"

namespace nih {

TEST(Logging, Basic) {
  ASSERT_THROW({LOG(FATAL) << "Fatal";}, NIHError);

  try {
    LOG(FATAL) << "Fatal";
  } catch(NIHError& e) {
    std::string error_msg = e.what();
    ASSERT_TRUE(error_msg.find("Fatal"));
  }

  try {
    LOG(FATAL) << "Fatal";
  } catch(FatalError& e) {
    std::string error_msg = e.what();
    ASSERT_TRUE(error_msg.find("Fatal"));
  }

  EXPECT_ANY_THROW(LOG(USER_E) << "User");

  std::string output;
  {
    auto capture {CapturedStream(STDERR_FILENO)};
    LOG(WARNING) << "Log warning.";
    output = capture.getCapturedString();
    ASSERT_NE(output.find("Log warning."), std::string::npos) << output;
    ASSERT_NE(output.find("[WARNING]"), std::string::npos);
  }

  {
    auto capture {CapturedStream(STDERR_FILENO)};
    LOG(ERROR) << "Log error.";
    output = capture.getCapturedString();
    ASSERT_NE(output.find("Log error."), std::string::npos);
    ASSERT_NE(output.find("[ERROR]"), std::string::npos);
  }

  {
    Log::setGlobalVerbosity(Log::ErrorType::kInfo);
    auto capture {CapturedStream(STDOUT_FILENO)};
    LOG(INFO) << "Log info.";
    output = capture.getCapturedString();
    ASSERT_NE(output.find("Log info."), std::string::npos);
    ASSERT_NE(output.find("[INFO]"), std::string::npos);
  }

  {
    Log::setGlobalVerbosity(Log::ErrorType::kDebug);
    auto capture {CapturedStream(STDOUT_FILENO)};
    LOG(DEBUG) << "Log debug.";
    output = capture.getCapturedString();
    ASSERT_NE(output.find("Log debug."), std::string::npos);
    ASSERT_NE(output.find("[DEBUG]"), std::string::npos);
  }

  {
    // Don't display anything with lower verbosity
    Log::setGlobalVerbosity(Log::defaultVerbosity());
    auto capture {CapturedStream(STDOUT_FILENO)};
    LOG(INFO) << "Log info.";
    output = capture.getCapturedString();
    ASSERT_EQ(output.find("Log info."), std::string::npos);
  }

  Log::setGlobalVerbosity(Log::ErrorType::kInfo);
  {
    int var = 17;
    auto capture {CapturedStream(STDOUT_FILENO)};
    LOG_VAR(var);
    output = capture.getCapturedString();
    ASSERT_NE(output.find("17"), std::string::npos);
  }
}

TEST(Logging, Thread) {
  Log::setThreadName("main-thread");
  std::string output;

  testing::internal::CaptureStdout();
  LOG(USER) << "Log User.";
  output = testing::internal::GetCapturedStdout();
  ASSERT_NE(output.find("main-thread"), std::string::npos);

  Log::setThreadName("");
  testing::internal::CaptureStdout();
  LOG(USER) << "Log User.";
  output = testing::internal::GetCapturedStdout();
  ASSERT_EQ(output.find("Thread"), std::string::npos);
}

TEST(Logging, Uri) {
  Log::setGlobalVerbosity(Log::ErrorType::kDebug);
  Uri tmp("file:///tmp/nih-test-logging");
  auto* scheme = new FileScheme(tmp, "w");
  Log::setUri(Log::ErrorType::kDebug, scheme);

  LOG(DEBUG) << "Debug";

  std::ifstream fin("/tmp/nih-test-logging");
  ASSERT_TRUE(fin);
  std::string out;
  std::getline(fin, out);
  ASSERT_NE(out.find("Debug"), std::string::npos);
  std::remove("/tmp/nih-test-logging");

  Log::reset();
  delete scheme;
}

TEST(Logging, Segfault) {
  int64_t* ptr {nullptr};
  // FIXME: doesn't seem to output anything in death test.
  EXPECT_DEATH({std::cout << *ptr << std::endl;}, "");  // NOLINT
}

}  // namespace nih