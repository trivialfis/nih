#include <gtest/gtest.h>
#include <iostream>

#include <nih/monitor.h>

namespace nih {

TEST(Monitor, Timer) {
  Timer timer = Timer().tick();
  timer.tock();
  testing::internal::CaptureStdout();
  std::cout << timer << std::flush;
  std::string str = testing::internal::GetCapturedStdout();
  ASSERT_NE(str.find("Elapsed:"), std::string::npos);
}

TEST(Monitor, GlobalMonitor) {
  { FUNC_TIMER(testing); }
  std::string str = GlobalMonitor::ins().toString();
  ASSERT_NE(str.find("testing"), std::string::npos);
}

} // namespace nih