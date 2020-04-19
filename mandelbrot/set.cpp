#include "set.h"

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
