/* This file is part of NIH.
 *
 * Copyright (c) 2019 Jiaming Yuan <jm.yuan@outlook.com>
 *
 * NIH is free software: you can redistribute it and/or modify it under the
 * terms of the Lesser GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * NIH is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the Lesser GNU General Public License for more
 * details.
 *
 * You should have received a copy of the Lesser GNU General Public License
 * along with NIH.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _RANDOME_HH_
#define _RANDOME_HH_

#include <cinttypes>
#include <memory>
#include <nih/singleton.hh>
#include <nih/logging.hh>

namespace nih {

class RandomDeviceImpl {
 public:
  virtual uint32_t run() = 0;
  virtual uint32_t min() const = 0;
  virtual uint32_t max() const = 0;
  virtual ~RandomDeviceImpl() {}
};

// Oringinally implemented in XGBoost.
/*!
 * \brief Linear congruential generator.
 *
 * The distribution defined in std is not portable. Given the same seed, it
 * migth produce different outputs on different platforms or with different
 * compilers.  The SimpleLCG implemented here is to make sure all tests are
 * reproducible.
 */
class SimpleLCG : public RandomDeviceImpl {
 private:
  using StateType = uint32_t;
  static StateType constexpr default_init_ = 3;
  static StateType constexpr default_alpha_ = 61;
  static StateType constexpr max_value_ = ((StateType)1 << 31) - 1;

  StateType state_;
  StateType const alpha_;
  StateType const mod_;

  StateType const seed_;

 public:
  SimpleLCG() : state_{default_init_},
                alpha_{default_alpha_}, mod_{max_value_}, seed_{state_}{}
  /*!
   * \brief Initialize SimpleLCG.
   *
   * \param state  Initial state, can also be considered as seed. If set to
   *               zero, SimpleLCG will use internal default value.
   * \param alpha  multiplier
   * \param mod    modulo
   */
  SimpleLCG(StateType state,
            StateType alpha=default_alpha_, StateType mod=max_value_)
      : state_{state == 0 ? default_init_ : state},
        alpha_{alpha}, mod_{mod} , seed_{state} {}

  StateType run() override {
    state_ = (alpha_ * state_) % mod_;
    return state_;
  }

  StateType min() const override {
    return seed_ * alpha_;
  }
  StateType max() const override {
    return max_value_;
  }
};

class RandomDevice : public Singleton<RandomDevice> {
 public:
  using result_type = uint32_t;

 private:
  std::unique_ptr<RandomDeviceImpl> _impl;
  bool _changed;

 public:
  RandomDevice();

  void setImpl(std::unique_ptr<RandomDeviceImpl>&& impl) {
    if (_changed) {
      LOG(WARNING) << "Setting random device more than once";
    }
    _impl = std::move(impl);
    _changed = true;
  }

  result_type operator()() {
    auto res = _impl->run();
    return res;
  }
  result_type min() { return _impl->min(); }
  result_type max() { return _impl->max(); }
};

}  // namespace nih

#endif  // _RANDOME_HH_