#include <random>
#include "nih/random.hh"

namespace nih {

class DefaultDeviceImpl : public RandomDeviceImpl {
  std::mt19937 _dev;

 public:
  uint32_t run() override {
    auto res = _dev();
    return res;
  }
  uint32_t min() const override {
    return _dev.min();
  }
  uint32_t max() const override {
    return _dev.max();
  }
};

RandomDevice::RandomDevice() :
    _impl { new DefaultDeviceImpl }, _changed { false } {}

}  // namespace nih