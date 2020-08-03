#pragma once

#include <cstdint>
#include <fmt/ostream.h>

template <typename T> struct Complex {
  T real, imag;

  [[nodiscard]] Complex(T real_in, T imag_in) noexcept : real{real_in}, imag{imag_in} {}
  [[nodiscard]] Complex() noexcept {}

  auto l2sqnorm() const noexcept -> T { return real * real + imag * imag; }

  [[nodiscard]] friend auto operator<<(std::ostream& os, Complex const& c) -> std::ostream& {
    return os << "Complex: {.real = " << c.real << ", .imag = " << c.imag << '}';
  }
};
