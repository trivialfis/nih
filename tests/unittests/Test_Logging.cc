#include <gtest/gtest.h>
#include <iostream>
#include <string_view>

#include "nih/Logging.h"

namespace nih {

static void loggerHandlerForTest(char const* msg) {
  std::cout << "From test handler:" << msg << std::endl;
}

TEST(Logger, Basic) {
  ::testing::internal::CaptureStdout();
  Logging::getLogger("test").info() << "info";
  auto out = ::testing::internal::GetCapturedStdout();
  ASSERT_TRUE(out.empty());

  Logging::getLogger("test").registerHandler(loggerHandlerForTest);

  ::testing::internal::CaptureStdout();
  Logging::getLogger("test").warn() << "warn";
  out = ::testing::internal::GetCapturedStdout();
  ASSERT_NE(out.find("From test handler"), std::string::npos);
}

TEST(Logger, Fork) {
  Logging::getLogger("test").registerHandler(loggerHandlerForTest);
  auto& fork = Logging::getLogger("test").fork("fork");
  ::testing::internal::CaptureStdout();
  fork.warn() << "warn";
  auto out = ::testing::internal::GetCapturedStdout();
  ASSERT_NE(out.find("From test handler"), std::string::npos);

  ::testing::internal::CaptureStdout();
  fork.warn() << "warn";
  out = ::testing::internal::GetCapturedStdout();
  ASSERT_EQ(out.find(R"("msg":"")"), std::string::npos) << "Destructor should not output empty msg";
}

TEST(Logger, Fatal) {
  try {
    LOG(FATAL) << "fatal";
  } catch (std::runtime_error const& e) {
    std::string str = e.what();

    ASSERT_NE(str.find("[0]"), std::string::npos);
  }
}

TEST(Logger, Config) {
  auto v = Logging::getLogger(kLoggerName).verbosity();
  Logging::getLogger(kLoggerName).verbosity(Logger::kFatal);
  ::testing::internal::CaptureStdout();
  LOG(WARN) << "warn";
  auto out = ::testing::internal::GetCapturedStdout();
  ASSERT_TRUE(out.empty()) << out;
  Logging::getLogger(kLoggerName).verbosity(v);
}
}  // namespace nih
