#pragma once

#include <array>
#include <concepts>
#include <fmt/ostream.h>
#include <immintrin.h>
#include <type_traits>

#include "util.h"

#define PS_COMP_TYPE OQ
#define PS_COMP_HIDDEN2(a, b, op, type) _mm256_cmp_ps(a, b, _CMP_##op##_##type)
#define PS_COMP_HIDDEN(a, b, op, type) PS_COMP_HIDDEN2(a, b, op, type)
#define PS_COMP(a, b, op) PS_COMP_HIDDEN(a, b, op, PS_COMP_TYPE)

union FloatSet {
  __m256 vec;
  std::array<float, sizeof(vec) / sizeof(float)> lanes;

  [[nodiscard]] constexpr FloatSet() noexcept {
    if (std::is_constant_evaluated())
      lanes = {};
    else
      vec = _mm256_setzero_ps();
  }

  [[nodiscard]] constexpr FloatSet(__m256 const& in) noexcept : vec{in} {}
  [[nodiscard]] constexpr FloatSet(decltype(lanes) const& in) noexcept : lanes{in} {}
  [[nodiscard]] constexpr FloatSet(float fill) noexcept {
    if (std::is_constant_evaluated())
      for (auto&& lane : lanes)
        lane = fill;
    else
      vec = _mm256_set1_ps(fill);
  }

  [[nodiscard]] auto operator+(FloatSet const& other) const noexcept -> FloatSet {
    return _mm256_add_ps(vec, other.vec);
  }

  [[nodiscard]] auto operator-(FloatSet const& other) const noexcept -> FloatSet {
    return _mm256_sub_ps(vec, other.vec);
  }

  [[nodiscard]] auto operator*(FloatSet const& other) const noexcept -> FloatSet {
    return _mm256_mul_ps(vec, other.vec);
  }

  [[nodiscard]] auto operator/(FloatSet const& other) const noexcept -> FloatSet {
    return _mm256_div_ps(vec, other.vec);
  }

  auto operator+=(FloatSet const& other) noexcept -> FloatSet& {
    *this = *this + other;

    return *this;
  }

  auto operator-=(FloatSet const& other) noexcept -> FloatSet& {
    *this = *this - other;

    return *this;
  }

  auto operator*=(FloatSet const& other) noexcept -> FloatSet& {
    *this = *this * other;

    return *this;
  }

  template <typename U>[[nodiscard]] auto operator^(U const& other) const noexcept -> FloatSet {
    return _mm256_xor_ps(vec, *reinterpret_cast<__m256 const*>(&other.vec));
  }

  template <typename U>[[nodiscard]] auto operator&(U const& other) const noexcept -> FloatSet {
    return _mm256_and_ps(vec, *reinterpret_cast<__m256 const*>(&other.vec));
  }

  template <typename U>[[nodiscard]] auto operator|(U const& other) const noexcept -> FloatSet {
    return _mm256_or_ps(vec, *reinterpret_cast<__m256 const*>(&other.vec));
  }

  template <typename U> auto operator^=(U const& other) noexcept -> FloatSet& {
    *this = *this ^ other;

    return *this;
  }

  template <typename U> auto operator&=(U const& other) noexcept -> FloatSet& {
    *this = *this & other;

    return *this;
  }

  template <typename U> auto operator|=(U const& other) noexcept -> FloatSet& {
    *this = *this | other;

    return *this;
  }

  [[nodiscard]] auto operator~() const noexcept -> FloatSet {
    return _mm256_castsi256_ps(~_mm256_castps_si256(vec));
  }

  [[nodiscard]] friend auto operator<<(std::ostream& os, FloatSet const& fset) -> std::ostream& {
    os << "FloatSet: {";

    for (std::size_t i = 0; i < fset.lanes.size(); ++i) {
      os << fset.lanes[i];
      if (i < fset.lanes.size() - 1)
        os << ", ";
    }

    return os << '}';
  }
};

template <typename T> union IntSet {
public:
  __m256i vec;
  std::array<T, sizeof(vec) / sizeof(T)> lanes;

  [[nodiscard]] constexpr IntSet() noexcept {
    if (std::is_constant_evaluated())
      lanes = {};
    else
      vec = _mm256_setzero_si256();
  }

  [[nodiscard]] constexpr IntSet(__m256i const& in) noexcept : vec(in) {}
  [[nodiscard]] constexpr IntSet(decltype(lanes) const& in) noexcept : lanes(in) {}

  [[nodiscard]] constexpr IntSet(T fill) noexcept {
    if (std::is_constant_evaluated()) {
      for (auto&& lane : lanes)
        lane = fill;
    } else {
      if constexpr (sizeof(T) == 8)
        vec = _mm256_set1_epi64x(fill);
      else if constexpr (sizeof(T) == 4)
        vec = _mm256_set1_epi32(fill);
      else if constexpr (sizeof(T) == 2)
        vec = _mm256_set1_epi16(fill);
      else if constexpr (sizeof(T) == 1)
        vec = _mm256_set1(epi8(fill));
      else
        static_assert(always_false<T>, "Invalid size");
    }
  }

  [[nodiscard]] auto operator+(IntSet const& other) const noexcept -> IntSet {
    if constexpr (sizeof(T) == 8)
      return _mm256_add_epi64(vec, other.vec);
    else if constexpr (sizeof(T) == 4)
      return _mm256_add_epi32(vec, other.vec);
    else if constexpr (sizeof(T) == 2)
      return _mm256_add_epi16(vec, other.vec);
    else if constexpr (sizeof(T) == 1)
      return _mm256_add_epi8(vec, other.vec);
    else
      static_assert(always_false<T>, "Invalid size");
  }

  [[nodiscard]] auto operator-(IntSet const& other) const noexcept -> IntSet {
    if constexpr (sizeof(T) == 8)
      return _mm256_sub_epi64(vec, other.vec);
    else if constexpr (sizeof(T) == 4)
      return _mm256_sub_epi32(vec, other.vec);
    else if constexpr (sizeof(T) == 2)
      return _mm256_sub_epi16(vec, other.vec);
    else if constexpr (sizeof(T) == 1)
      return _mm256_sub_epi8(vec, other.vec);
    else
      static_assert(always_false<T>, "Invalid size");
  }

  [[nodiscard]] auto operator*(IntSet const& other) const noexcept -> IntSet {
    if constexpr (sizeof(T) == 4)
      return _mm256_mullo_epi32(vec, other.vec);
    else if constexpr (sizeof(T) == 2)
      return _mm256_mullo_epi16(vec, other.vec);
    else
      static_assert(always_false<T>, "Invalid size");
  }

  auto operator+=(IntSet const& other) noexcept -> IntSet& {
    *this = *this + other;

    return *this;
  }

  auto operator-=(IntSet const& other) noexcept -> IntSet& {
    *this = *this - other;

    return *this;
  }

  auto operator*=(IntSet const& other) noexcept -> IntSet& {
    *this = *this * other;

    return *this;
  }

  template <typename U>[[nodiscard]] auto operator^(U const& other) const noexcept -> IntSet {
    return _mm256_xor_si256(vec, *reinterpret_cast<__m256i const*>(&other.vec));
  }

  template <typename U>[[nodiscard]] auto operator&(U const& other) const noexcept -> IntSet {
    return _mm256_and_si256(vec, *reinterpret_cast<__m256i const*>(&other.vec));
  }

  template <typename U>[[nodiscard]] auto operator|(U const& other) const noexcept -> IntSet {
    return _mm256_or_si256(vec, *reinterpret_cast<__m256i const*>(&other.vec));
  }

  template <typename U> auto operator^=(U const& other) noexcept -> IntSet& {
    *this = *this ^ other;

    return *this;
  }

  template <typename U> auto operator&=(U const& other) noexcept -> IntSet& {
    *this = *this & other;

    return *this;
  }

  template <typename U> auto operator|=(U const& other) noexcept -> IntSet& {
    *this = *this | other;

    return *this;
  }

  [[nodiscard]] auto operator~() const noexcept -> IntSet { return ~vec; }

  template <typename U>
  [[nodiscard]] friend auto operator<<(std::ostream& os, IntSet<U> const& iset) -> std::ostream& {
    os << "IntSet: {";

    for (std::size_t i = 0; i < iset.lanes.size(); ++i) {
      os << iset.lanes[i];
      if (i < iset.lanes.size() - 1)
        os << ", ";
    }

    return os << '}';
  }
};

[[nodiscard]] auto operator==(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet;
[[nodiscard]] auto operator<(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet;
[[nodiscard]] auto operator<=(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet;
[[nodiscard]] auto operator!=(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet;
[[nodiscard]] auto operator>(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet;
[[nodiscard]] auto operator>=(FloatSet const& a, FloatSet const& b) noexcept -> FloatSet;

template <typename T>
[[nodiscard]] auto operator==(IntSet<T> const& a, IntSet<T> const& b) noexcept -> IntSet<T> {
  if constexpr (sizeof(T) == 8)
    return _mm256_cmpeq_epi64(a.vec, b.vec);
  else if constexpr (sizeof(T) == 4)
    return _mm256_cmpeq_epi32(a.vec, b.vec);
  else if constexpr (sizeof(T) == 2)
    return _mm256_cmpeq_epi16(a.vec, b.vec);
  else if constexpr (sizeof(T) == 1)
    return _mm256_cmpeq_epi8(a.vec, b.vec);
  else
    static_assert(always_false<T>, "Invalid type size");
}

template <typename T>
[[nodiscard]] auto operator!=(IntSet<T> const& a, IntSet<T> const& b) noexcept -> IntSet<T> {
  return ~(a == b);
}

template <typename T>
[[nodiscard]] auto operator>(IntSet<T> const& a, IntSet<T> const& b) noexcept -> IntSet<T> {
  if constexpr (sizeof(T) == 8)
    return _mm256_cmpgt_epi64(a.vec, b.vec);
  else if constexpr (sizeof(T) == 4)
    return _mm256_cmpgt_epi32(a.vec, b.vec);
  else if constexpr (sizeof(T) == 2)
    return _mm256_cmpgt_epi16(a.vec, b.vec);
  else if constexpr (sizeof(T) == 1)
    return _mm256_cmpgt_epi8(a.vec, b.vec);
  else
    static_assert(always_false<T>, "Invalid type size");
}

template <typename T>
[[nodiscard]] auto operator<(IntSet<T> const& a, IntSet<T> const& b) noexcept -> IntSet<T> {
  return b > a;
}

template <typename T>
[[nodiscard]] auto operator<=(IntSet<T> const& a, IntSet<T> const& b) noexcept -> IntSet<T> {
  return ~(a > b);
}

template <typename T>
[[nodiscard]] auto operator>=(IntSet<T> const& a, IntSet<T> const& b) noexcept -> IntSet<T> {
  return ~(b > a);
}
