/*
 * \brief An implemenation of Ryu algorithm:
 *
 * https://dl.acm.org/citation.cfm?id=3192369
 *
 * The code is adopted from original (half) c implementation:
 * https://github.com/ulfjack/ryu.git with some more comments and tidying.  License is
 * attached below.
 *
 * Copyright 2018 Ulf Adams
 *
 * The contents of this file may be used under the terms of the Apache License,
 * Version 2.0.
 *
 *    (See accompanying file LICENSE-Apache or copy at
 *     http: *www.apache.org/licenses/LICENSE-2.0)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE-Boost or copy at
 *     https://www.boost.org/LICENSE_1_0.txt)
 *
 * Unless required by applicable law or agreed to in writing, this software
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.

 */

#ifndef RYU_HH_
#define RYU_HH_

#include <cassert>
#include <cinttypes>
#include <cstring>
#include <cmath>

#include <nih/logging.hh>
#include <nih/luts.hh>

namespace nih {

struct UnsignedFloatBase2;

struct UnsignedFloatBase10 {
  uint32_t mantissa;
  // Decimal exponent's range is -45 to 38
  // inclusive, and can fit in a short if needed.
  int32_t exponent;
};

struct IEEE754 {
  static constexpr uint32_t kFloatMantissaBits = 23;
  static constexpr uint32_t kFloatBias = 127;
  static constexpr uint32_t kFloatExponentBits = 8;

  static void Decode(float f, UnsignedFloatBase2* uf, bool* signbit);
};

struct UnsignedFloatBase2 {
  uint32_t mantissa;
  // Decimal exponent's range is -45 to 38
  // inclusive, and can fit in a short if needed.
  uint32_t exponent;

  bool Infinite() const {
    return exponent == ((1u << IEEE754::kFloatExponentBits) - 1u);
  }
  bool Zero() const {
    return mantissa == 0 && exponent == 0;
  }
};

inline void IEEE754::Decode(float f, UnsignedFloatBase2 *uf, bool *signbit) {
  uint32_t bits;
  std::memcpy(&bits, &f, sizeof(bits));
  // Decode bits into sign, mantissa, and exponent.
  *signbit = std::signbit(f);
  uf->mantissa = bits & ((1u << kFloatMantissaBits) - 1);
  uf->exponent = (bits >> IEEE754::kFloatMantissaBits) &
                 ((1u << IEEE754::kFloatExponentBits) - 1);
}

// Represents the interval of information-preserving outputs.
struct MantissaInteval {
  int32_t exponent;
  // low: smaller half way point
  uint32_t mantissa_low;
  // correct: f
  uint32_t mantissa_correct;
  // high: larger half way point
  uint32_t mantissa_high;
};

struct RyuPowLogUtils {
  // This table is generated by PrintFloatLookupTable from ryu.
  uint32_t constexpr static kFloatPow5InvBitcount = 59;
  static constexpr uint64_t kFloatPow5InvSplit[31] = {
      576460752303423489u, 461168601842738791u, 368934881474191033u,
      295147905179352826u, 472236648286964522u, 377789318629571618u,
      302231454903657294u, 483570327845851670u, 386856262276681336u,
      309485009821345069u, 495176015714152110u, 396140812571321688u,
      316912650057057351u, 507060240091291761u, 405648192073033409u,
      324518553658426727u, 519229685853482763u, 415383748682786211u,
      332306998946228969u, 531691198313966350u, 425352958651173080u,
      340282366920938464u, 544451787073501542u, 435561429658801234u,
      348449143727040987u, 557518629963265579u, 446014903970612463u,
      356811923176489971u, 570899077082383953u, 456719261665907162u,
      365375409332725730u};

  uint32_t constexpr static kFloatPow5Bitcount = 61;
  static constexpr uint64_t kFloatPow5Split[47] = {
      1152921504606846976u, 1441151880758558720u, 1801439850948198400u,
      2251799813685248000u, 1407374883553280000u, 1759218604441600000u,
      2199023255552000000u, 1374389534720000000u, 1717986918400000000u,
      2147483648000000000u, 1342177280000000000u, 1677721600000000000u,
      2097152000000000000u, 1310720000000000000u, 1638400000000000000u,
      2048000000000000000u, 1280000000000000000u, 1600000000000000000u,
      2000000000000000000u, 1250000000000000000u, 1562500000000000000u,
      1953125000000000000u, 1220703125000000000u, 1525878906250000000u,
      1907348632812500000u, 1192092895507812500u, 1490116119384765625u,
      1862645149230957031u, 1164153218269348144u, 1455191522836685180u,
      1818989403545856475u, 2273736754432320594u, 1421085471520200371u,
      1776356839400250464u, 2220446049250313080u, 1387778780781445675u,
      1734723475976807094u, 2168404344971008868u, 1355252715606880542u,
      1694065894508600678u, 2117582368135750847u, 1323488980084844279u,
      1654361225106055349u, 2067951531382569187u, 1292469707114105741u,
      1615587133892632177u, 2019483917365790221u};

  static uint32_t Pow5Factor(uint32_t value) {
    uint32_t count = 0;
    for (;;) {
      const uint32_t q = value / 5;
      const uint32_t r = value % 5;
      if (r != 0) {
        break;
      }
      value = q;
      ++count;
    }
    return count;
  }

  // Returns true if value is divisible by 5^p.
  static bool MultipleOfPowerOf5(const uint32_t value, const uint32_t p) {
    return Pow5Factor(value) >= p;
  }

  // Returns true if value is divisible by 2^p.
  static bool MultipleOfPowerOf2(const uint32_t value, const uint32_t p) {
#ifdef __GNUC__
    return __builtin_ctz(value) >= p;
#else
    return (value & ((1u << p) - 1)) == 0;
#endif  //  __GNUC__
  }

  // Returns e == 0 ? 1 : ceil(log_2(5^e)).
  static uint32_t Pow5Bits(const int32_t e) {
    return (uint32_t)(((e * 163391164108059ull) >> 46) + 1);
  }

  /*
   * \brief Multiply 32-bit and 64-bit -> 128 bit, then access the higher bits.
   */
  static uint32_t MulShift(const uint32_t x, const uint64_t y,
                           const int32_t shift) {
    // For 32-bit * 64-bit: x * y, it can be decomposed into:
    //
    //   x * (y_high + y_low) = (x * y_high) + (x * y_low)
    //
    // For more general case 64-bit * 64-bit, see https://stackoverflow.com/a/1541458
    const uint32_t y_low = (uint32_t)(y);
    const uint32_t y_high = (uint32_t)(y >> 32);

    const uint64_t low = (uint64_t)x * y_low;
    const uint64_t high = (uint64_t)x * y_high;

    const uint64_t sum = (low >> 32) + high;
    const uint64_t shifted_sum = sum >> (shift - 32);

    return (uint32_t)shifted_sum;
  }

  /*
   * \brief floor(5^q/2*k) and shift by j
   */
  static uint32_t MulPow5InvDivPow2(const uint32_t m, const uint32_t q,
                                    const int32_t j) {
    return MulShift(m, kFloatPow5InvSplit[q], j);
  }

  /*
   * \brief floor(2^k/5^q) + 1 and shift by j
   */
  static uint32_t MulPow5divPow2(const uint32_t m, const uint32_t i,
                                 const int32_t j) {
    return MulShift(m, kFloatPow5Split[i], j);
  }

  /*
   * \brief floor(e * log_10(2)).
   */
  static uint32_t log10Pow2(const int32_t e) {
    // The first value this approximation fails for is 2^1651 which is just
    // greater than 10^297.
    assert(e >= 0);
    assert(e <= 1 << 15);
    return (uint32_t)((((uint64_t)e) * 169464822037455ull) >> 49);
  }

  // Returns floor(e * log_10(5)).
  static uint32_t Log10Pow5(const int32_t expoent) {
    // The first value this approximation fails for is 5^2621 which is just
    // greater than 10^1832.
    assert(expoent >= 0);
    assert(expoent <= 1 << 15);
    return static_cast<uint32_t>(
        ((static_cast<uint64_t>(expoent)) * 196742565691928ull) >> 48);
  }
};

class PowerBaseComputer {
 private:
  static void ToDecimalBase(bool acceptBounds, uint32_t mmShift,
                            MantissaInteval base2,
                            MantissaInteval* base10,
                            bool *vmIsTrailingZeros, bool *vrIsTrailingZeros) {
    uint8_t last_removed_digit = 0;
    if (base2.exponent >= 0) {
      const uint32_t q = RyuPowLogUtils::log10Pow2(base2.exponent);
      base10->exponent = (int32_t)q;
      const int32_t k = RyuPowLogUtils::kFloatPow5InvBitcount +
                        RyuPowLogUtils::Pow5Bits(static_cast<int32_t>(q)) -
                        1;
      const int32_t i = - base2.exponent + (int32_t)q + k;
      base10->mantissa_low =
          RyuPowLogUtils::MulPow5InvDivPow2(base2.mantissa_low, q, i);
      base10->mantissa_correct = RyuPowLogUtils::MulPow5InvDivPow2(base2.mantissa_correct, q, i);
      base10->mantissa_high =
          RyuPowLogUtils::MulPow5InvDivPow2(base2.mantissa_high, q, i);

      if (q != 0 && (base10->mantissa_high - 1) / 10 <= base10->mantissa_low / 10) {
        // We need to know one removed digit even if we are not going to loop
        // below. We could use q = X - 1 above, except that would require 33
        // bits for the result, and we've found that 32-bit arithmetic is faster
        // even on 64-bit machines.
        const int32_t l = RyuPowLogUtils::kFloatPow5InvBitcount +
                          RyuPowLogUtils::Pow5Bits((int32_t)(q - 1)) - 1;
        last_removed_digit =
            static_cast<uint8_t>(RyuPowLogUtils::MulPow5InvDivPow2(
                                     base2.mantissa_correct, q - 1,
                                     -base2.exponent + (int32_t)q - 1 + l) %
                                 10);
      }
      if (q <= 9) {
        // The largest power of 5 that fits in 24 bits is 5^10, but q <= 9 seems
        // to be safe as well. Only one of mp, mv, and mm can be a multiple of
        // 5, if any.
        if (base2.mantissa_correct % 5 == 0) {
          *vrIsTrailingZeros =
              RyuPowLogUtils::MultipleOfPowerOf5(base2.mantissa_correct, q);
        } else if (acceptBounds) {
          *vmIsTrailingZeros =
              RyuPowLogUtils::MultipleOfPowerOf5(base2.mantissa_low, q);
        } else {
          base10->mantissa_high -=
              RyuPowLogUtils::MultipleOfPowerOf5(base2.mantissa_high, q);
        }
      }
    } else {
      const uint32_t q = RyuPowLogUtils::Log10Pow5(-base2.exponent);
      base10->exponent = (int32_t)q + base2.exponent;
      const int32_t i = -base2.exponent - (int32_t)q;
      const int32_t k = RyuPowLogUtils::Pow5Bits(i) -
                        RyuPowLogUtils::kFloatPow5Bitcount;
      int32_t j = (int32_t)q - k;
      base10->mantissa_correct =
          RyuPowLogUtils::MulPow5divPow2(base2.mantissa_correct, (uint32_t)i, j);
      base10->mantissa_high =
          RyuPowLogUtils::MulPow5divPow2(base2.mantissa_high, (uint32_t)i, j);
      base10->mantissa_low =
          RyuPowLogUtils::MulPow5divPow2(base2.mantissa_low, (uint32_t)i, j);

      if (q != 0 && (base10->mantissa_high - 1) / 10 <= base10->mantissa_low / 10) {
        j = (int32_t)q - 1 -
            (RyuPowLogUtils::Pow5Bits(i + 1) -
             RyuPowLogUtils::kFloatPow5Bitcount);
        last_removed_digit =
            (uint8_t)(RyuPowLogUtils::MulPow5divPow2(base2.mantissa_correct,
                                                        (uint32_t)(i + 1), j) %
                      10);
      }
      if (q <= 1) {
        // {vr,vp,vm} is trailing zeros if {mv,mp,mm} has at least q trailing 0
        // bits. mv = 4 * m2, so it always has at least two trailing 0 bits.
        *vrIsTrailingZeros = true;
        if (acceptBounds) {
          // mm = mv - 1 - mmShift, so it has 1 trailing 0 bit iff mmShift == 1.
          *vmIsTrailingZeros = mmShift == 1;
        } else {
          // mp = mv + 2, so it always has at least one trailing 0 bit.
          --base10->mantissa_high;
        }
      } else if (q < 31) { // TODO(ulfjack): Use a tighter bound here.
        *vrIsTrailingZeros =
            RyuPowLogUtils::MultipleOfPowerOf2(base2.mantissa_correct, q - 1);
      }
    }
  }

  /*
   * \brief A varient of extended euclidean GCD algorithm.
   */
  static UnsignedFloatBase10
  ShortestRepresentation(bool mantissa_low_is_trailing_zeros,
                         bool vrIsTrailingZeros, bool const acceptBounds,
                         MantissaInteval base10) {
    int32_t removed {0};
    uint32_t output {0};
    uint8_t last_removed_digit {0};
    if (mantissa_low_is_trailing_zeros || vrIsTrailingZeros) {
      // General case, which happens rarely (~4.0%).
      while (base10.mantissa_high / 10 > base10.mantissa_low / 10) {
        mantissa_low_is_trailing_zeros &= base10.mantissa_low % 10 == 0;
        vrIsTrailingZeros &= last_removed_digit == 0;
        last_removed_digit = static_cast<uint8_t>(base10.mantissa_correct % 10);
        base10.mantissa_correct /= 10;
        base10.mantissa_high /= 10;
        base10.mantissa_low /= 10;
        ++removed;
      }

      if (mantissa_low_is_trailing_zeros) {
        while (base10.mantissa_low % 10 == 0) {
          vrIsTrailingZeros &= last_removed_digit == 0;
          last_removed_digit = static_cast<uint8_t>(base10.mantissa_correct % 10);
          base10.mantissa_correct /= 10;
          base10.mantissa_high /= 10;
          base10.mantissa_low /= 10;
          ++removed;
        }
      }

      if (vrIsTrailingZeros && last_removed_digit == 5 &&
          base10.mantissa_correct % 2 == 0) {
        // Round even if the exact number is .....50..0.
        last_removed_digit = 4;
      }
      // We need to take vr + 1 if vr is outside bounds or we need to round up.
      output = base10.mantissa_correct +
               ((base10.mantissa_correct == base10.mantissa_low &&
                 (!acceptBounds || !mantissa_low_is_trailing_zeros)) ||
                last_removed_digit >= 5);
    } else {
      // Specialized for the common case (~96.0%). Percentages below are
      // relative to this. Loop iterations below (approximately): 0: 13.6%,
      // 1: 70.7%, 2: 14.1%, 3: 1.39%, 4: 0.14%, 5+: 0.01%
      while (base10.mantissa_high / 10 > base10.mantissa_low / 10) {
        last_removed_digit = (uint8_t)(base10.mantissa_correct % 10);
        base10.mantissa_correct /= 10;
        base10.mantissa_high /= 10;
        base10.mantissa_low /= 10;
        ++removed;
      }

      // We need to take vr + 1 if vr is outside bounds or we need to round up.
      output = base10.mantissa_correct +
               (base10.mantissa_correct == base10.mantissa_low ||
                last_removed_digit >= 5);
    }
    const int32_t exp = base10.exponent + removed;

    UnsignedFloatBase10 fd;
    fd.exponent = exp;
    fd.mantissa = output;
    return fd;
  }

 public:
  static UnsignedFloatBase10 Binary2Decimal(UnsignedFloatBase2 const f) {
    MantissaInteval base2_range;
    uint32_t mantissa_base2;
    if (f.exponent == 0) {
      // We subtract 2 so that the bounds computation has 2 additional bits.
      base2_range.exponent =
          1 - IEEE754::kFloatBias - IEEE754::kFloatMantissaBits - 2;
      mantissa_base2 = f.mantissa;
    } else {
      base2_range.exponent = (int32_t)f.exponent - IEEE754::kFloatBias -
                             IEEE754::kFloatMantissaBits - 2;
      mantissa_base2 = (1u << IEEE754::kFloatMantissaBits) | f.mantissa;
    }
    const bool even = (mantissa_base2 & 1) == 0;
    const bool acceptBounds = even;

    // Step 2: Determine the interval of valid decimal representations.
    base2_range.mantissa_correct = 4 * mantissa_base2;
    base2_range.mantissa_high = 4 * mantissa_base2 + 2;
    // Implicit bool -> int conversion. True is 1, false is 0.
    const uint32_t mantissa_low_shift = f.mantissa != 0 || f.exponent <= 1;
    base2_range.mantissa_low = 4 * mantissa_base2 - 1 - mantissa_low_shift;

    // Step 3: Convert to a decimal power base using 64-bit arithmetic.
    MantissaInteval base10_range;
    bool mantissa_low_is_trailing_zeros = false;
    bool mantissa_out_is_trailing_zeros = false;
    PowerBaseComputer::ToDecimalBase(acceptBounds, mantissa_low_shift, base2_range,
                                     &base10_range,
                                     &mantissa_low_is_trailing_zeros,
                                     &mantissa_out_is_trailing_zeros);

    // Step 4: Find the shortest decimal representation in the interval of valid
    // representations.
    auto out = ShortestRepresentation(mantissa_low_is_trailing_zeros,
                                      mantissa_out_is_trailing_zeros,
                                      acceptBounds, base10_range);
    return out;
  }
};

namespace {
constexpr uint32_t Tens(uint32_t n) { return n == 1 ? 10 : (Tens(n - 1) * 10); }
} // namespace

static inline uint32_t OutputLength(const uint32_t v) {
  // Function precondition: v is not a 10-digit number.
  // (f2s: 9 digits are sufficient for round-tripping.)
  // (d2fixed: We print 9-digit blocks.)
  static_assert(100000000 == Tens(8), "");
  assert(v < Tens(9));
  if (v >= Tens(8)) {
    return 9;
  }
  if (v >= Tens(7)) {
    return 8;
  }
  if (v >= Tens(6)) {
    return 7;
  }
  if (v >= Tens(5)) {
    return 6;
  }
  if (v >= Tens(4)) {
    return 5;
  }
  if (v >= Tens(3)) {
    return 4;
  }
  if (v >= Tens(2)) {
    return 3;
  }
  if (v >= Tens(1)) {
    return 2;
  }
  return 1;
}
/*
 * \brief Print the floating point number in base 10.
 */
struct RyuPrinter {
  static int32_t PrintBase10Float(UnsignedFloatBase10 v, const bool sign,
                                  char *const result) {
    // Step 5: Print the decimal representation.
    int index = 0;
    if (sign) {
      result[index++] = '-';
    }

    uint32_t output = v.mantissa;
    const uint32_t out_length = OutputLength(output);

    // Print the decimal digits.
    // The following code is equivalent to:
    // for (uint32_t i = 0; i < olength - 1; ++i) {
    //   const uint32_t c = output % 10; output /= 10;
    //   result[index + olength - i] = (char) ('0' + c);
    // }
    // result[index] = '0' + output % 10;
    uint32_t i = 0;
    while (output >= Tens(4)) {
      const uint32_t c = output % Tens(4);
      output /= Tens(4);
      const uint32_t c0 = (c % 100) << 1;
      const uint32_t c1 = (c / 100) << 1;
      // This is used to speed up decimal digit generation by copying
      // pairs of digits into the final output.
      std::memcpy(result + index + out_length - i - 1, kItoaLut + c0, 2);
      std::memcpy(result + index + out_length - i - 3, kItoaLut + c1, 2);
      i += 4;
    }
    if (output >= 100) {
      const uint32_t c = (output % 100) << 1;
      output /= 100;
      std::memcpy(result + index + out_length - i - 1, kItoaLut + c, 2);
      i += 2;
    }
    if (output >= 10) {
      const uint32_t c = output << 1;
      // We can't use std::memcpy here: the decimal dot goes between these two
      // digits.
      result[index + out_length - i] = kItoaLut[c + 1];
      result[index] = kItoaLut[c];
    } else {
      result[index] = (char)('0' + output);
    }

    // Print decimal point if needed.
    if (out_length > 1) {
      result[index + 1] = '.';
      index += out_length + 1;
    } else {
      ++index;
    }

    // Print the exponent.
    result[index++] = 'E';
    int32_t exp = v.exponent + (int32_t)out_length - 1;
    if (exp < 0) {
      result[index++] = '-';
      exp = -exp;
    }

    if (exp >= 10) {
      std::memcpy(result + index, kItoaLut + 2 * exp, 2);
      index += 2;
    } else {
      result[index++] = (char)('0' + exp);
    }

    return index;
  }

  static int32_t PrintSpecialFloat(const bool sign, UnsignedFloatBase2 f,
                                   char *const result) {
    if (f.mantissa) {
      std::memcpy(result, "NaN", 3);
      return 3;
    }
    if (sign) {
      result[0] = '-';
    }
    if (f.exponent) {
      std::memcpy(result + sign, "Infinity", 8);
      return sign + 8;
    }
    std::memcpy(result + sign, "0E0", 3);
    return sign + 3;
  }
};

inline int32_t f2s_buffered_n(float f, char * const result) {
  // Step 1: Decode the floating-point number, and unify normalized and
  // subnormal cases.
  UnsignedFloatBase2 uf;
  bool sign;
  IEEE754::Decode(f, &uf, &sign);

  // Case distinction; exit early for the easy cases.
  if (uf.Infinite() || uf.Zero()) {
    return RyuPrinter::PrintSpecialFloat(sign, uf, result);
  }

  const UnsignedFloatBase10 v = PowerBaseComputer::Binary2Decimal(uf);
  const auto index = RyuPrinter::PrintBase10Float(v, sign, result);
  return index;
}

} // namespace nih

#endif