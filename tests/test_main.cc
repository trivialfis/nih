#include <gtest/gtest.h>
#include "nih/random.h"

int main(int argc, char ** argv) {
  std::unique_ptr<nih::RandomDeviceImpl> p_dev{ new nih::SimpleLCG };
  nih::RandomDevice::ins().setImpl(std::move(p_dev));

  testing::InitGoogleTest(&argc, argv);
  testing::FLAGS_gtest_death_test_style = "threadsafe";
  return RUN_ALL_TESTS();
}