#include "set.h"

constexpr FloatSet::operator IntSet<i32>() const noexcept {
  if (std::is_constant_evaluated()) {
    auto ints = decltype(IntSet<i32>::lanes){};
    std::copy(lanes.cbegin(), lanes.cend(), ints.begin());
    return IntSet<i32>{ints};
  } else
    return IntSet<i32>{_mm256_cvttps_epi32(vec)};
}

auto operator==(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet {
  return PS_COMP(a.vec, b.vec, EQ);
}

auto operator<(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet {
  return PS_COMP(a.vec, b.vec, LT);
}

auto operator<=(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet {
  return PS_COMP(a.vec, b.vec, LE);
}

auto operator!=(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet {
  return PS_COMP(a.vec, b.vec, NEQ);
}

auto operator>(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet {
  return PS_COMP(a.vec, b.vec, GT);
}

auto operator>=(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet {
  return PS_COMP(a.vec, b.vec, GE);
}
