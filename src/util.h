#pragma once

#include <chrono>
#include <type_traits>

template <typename...> constexpr std::false_type always_false{};

template <typename T>[[nodiscard]] auto constexpr to_ms(T const& start, T const& end) {
  return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}
