#ifndef HIGH_PRECISION_DOUBLE_H
#define HIGH_PRECISION_DOUBLE_H

#include <cmath>

namespace {
constexpr double pow_2(const int i) noexcept {
  double res = 2;
  const bool is_div = i < 0;
  const int N = (i < 0) ? (-i) : (i);
  for (int i = 0; i <= N; i++) {
    if (is_div)
      res /= 2;
    else
      res *= 2;
  }
  return res;
}
}  // namespace

static constexpr double two_power_neg_48 = pow_2(-48);

struct alignas(16) hpd {
  double big;
  double small;

  hpd() = default;

  void normalize() noexcept {
    // return;
    const bool is_big_negative = std::signbit(big);

    const bool is_small_negative = std::signbit(small);

    big = std::abs(big);
    small = std::abs(small);

    if (small > big * two_power_neg_48) {
      const double delta = small - big * two_power_neg_48;
      small -= delta;
      big += delta;
    }

    if (is_big_negative) big = -big;
    if (is_small_negative) small = -small;
  }

  inline explicit hpd(int i) : big(i), small(0) { this->normalize(); }
  inline explicit hpd(double d) : big(d), small(0) { this->normalize(); }
  inline explicit hpd(__float128 d) {
    this->big = d;
    this->small = d - this->big;
    this->normalize();
  }

  inline operator double() const noexcept { return big + small; }

  inline operator __float128() const noexcept {
    return __float128(big) + __float128(small);
  }

  inline hpd operator+(const hpd& another) const noexcept {
    hpd ret;

    if (std::abs(another.big) < std::abs(this->big) * two_power_neg_48) {
      ret.big = this->big;
      ret.small = (this->small + another.big) + another.small;
    } else if (std::abs(this->big) < std::abs(another.big) * two_power_neg_48) {
      ret.big = another.big;
      ret.small = (another.small + this->big) + this->small;
    } else {
      ret.big = this->big + another.big;
      ret.small = this->small + another.small;
    }

    ret.normalize();
    return ret;
  }

  inline hpd operator-(const hpd& another) const noexcept {
    hpd ret;

    if (std::abs(another.big) < std::abs(this->big) * two_power_neg_48) {
      ret.big = this->big;
      ret.small = (this->small - another.big) - another.small;
    } else if (std::abs(this->big) < std::abs(another.big) * two_power_neg_48) {
      ret.big = -another.big;
      ret.small = (this->big - another.small) + this->small;
    } else {
      ret.big = this->big - another.big;
      ret.small = this->small - another.small;
    }

    ret.normalize();
    return ret;
  }

  inline hpd operator*(const hpd another) const noexcept {
    hpd ret;
    ret.big = this->big * another.big;
    ret.small = (this->big * another.small + this->small * another.big) +
                this->small * another.small;
    ret.normalize();
    return ret;
  }
};

#endif  // HIGH_PRECISION_DOUBLE_H