#pragma once

#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <type_traits>

using Size = std::size_t;
using Addr = std::uintptr_t;

using n8 = std::uint8_t;
using n16 = std::uint16_t;
using n32 = std::uint32_t;
using n64 = std::uint64_t;

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;

using f32 = float;
using f64 = double;

template <Size size>
concept MachineCompatible = (size == 8) || (size == 16) || (size == 32) || (size == 64);

template <Size size>
requires MachineCompatible<size> using Nat = std::conditional_t<
    size == 8, n8,
    std::conditional_t<
        size == 16, n16,
        std::conditional_t<size == 32, n32, std::conditional_t<size == 64, n64, void>>>>;

template <Size size>
requires MachineCompatible<size> using Int = std::conditional_t<
    size == 8, i8,
    std::conditional_t<
        size == 16, i16,
        std::conditional_t<size == 32, i32, std::conditional_t<size == 64, n64, void>>>>;

using UMicroseconds = std::chrono::duration<n32, std::micro>;

auto inline constexpr operator"" _n8 [[gnu::always_inline, nodiscard]](unsigned long long const val)
-> n8 {
  return static_cast<n8>(val);
}
auto inline constexpr operator"" _n16
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> n16 {
  return static_cast<n16>(val);
}
auto inline constexpr operator"" _n32
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> n32 {
  return static_cast<n32>(val);
}
auto inline constexpr operator"" _n64
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> n64 {
  return val;
}

auto inline constexpr operator"" _i8 [[gnu::always_inline, nodiscard]](unsigned long long const val)
-> i8 {
  return static_cast<i8>(val);
}
auto inline constexpr operator"" _i16
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> i16 {
  return static_cast<i16>(val);
}
auto inline constexpr operator"" _i32
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> i32 {
  return static_cast<i32>(val);
}
auto inline constexpr operator"" _i64
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> i64 {
  return static_cast<i64>(val);
}

auto inline constexpr operator"" _uus
    [[gnu::always_inline, nodiscard]](unsigned long long const val) -> UMicroseconds {
  return UMicroseconds{val};
}

template <typename T> concept TriviallyCopyable = std::is_trivially_copyable_v<T>;

template <typename Dst, typename Src>
concept BitCastable =
    (sizeof(Dst) == sizeof(Src)) && TriviallyCopyable<Dst>&& TriviallyCopyable<Src>;

template <typename Dst, typename Src>
requires BitCastable<Dst, Src> auto inline bit_cast [[gnu::always_inline]] (Src const& src) -> Dst {
  auto dst = Dst{};
  std::memcpy(&dst, &src, sizeof(dst));
  return dst;
}

template <typename...> bool constexpr always_false = false;

struct Unit {};

template <typename T> concept Enum = std::is_enum_v<T>;
template <Enum T> using utype = std::underlying_type_t<T>;
template <Enum T> auto inline constexpr utype_cast(T x) { return static_cast<utype<T>>(x); }

template <typename T = void> class Ptr {
public:
  [[nodiscard]] constexpr Ptr(Addr addr) noexcept : addr_{addr} {}

  operator T*() const noexcept { return bit_cast<T*>(addr_); }

private:
  Addr addr_;
};

enum class BitOp : n8 { Clear, Set, Toggle };

template <std::unsigned_integral T>
auto inline constexpr change_bit [[nodiscard]] (T const val, n32 const idx, BitOp const op) -> T {
  switch (op) {
  case BitOp::Clear:
    return static_cast<T>(val & ~(T{1} << idx));
  case BitOp::Set:
    return static_cast<T>(val | (T{1} << idx));
  case BitOp::Toggle:
    return static_cast<T>(val ^ (T{1} << idx));
  }

  return val;
}

template <std::unsigned_integral T>
auto inline constexpr clear_bit [[nodiscard]] (T const val, n32 const idx) -> T {
  return change_bit<T>(val, idx, BitOp::Clear);
}

template <std::unsigned_integral T>
auto inline constexpr set_bit [[nodiscard]] (T const val, n32 const idx) -> T {
  return change_bit<T>(val, idx, BitOp::Set);
}

template <std::unsigned_integral T>
auto inline constexpr toggle_bit [[nodiscard]] (T const val, n32 const idx) -> T {
  return change_bit<T>(val, idx, BitOp::Toggle);
}

template <typename T> [[nodiscard]] auto constexpr to_ms(T const& start, T const& end) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}
