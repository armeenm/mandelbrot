#pragma once

#include "complex.h"
#include "set.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fmt/ostream.h>
#include <memory>
#include <numeric>
#include <string_view>

template <typename T> struct GenCoord {
  T x, y;

  [[nodiscard]] GenCoord(T x_in, T y_in) noexcept : x{x_in}, y{y_in} {}
  [[nodiscard]] GenCoord() noexcept {}
};

class Image {
public:
  template <typename T> struct GenFrame {
    GenCoord<T> lower, upper;

    [[nodiscard]] auto width() const noexcept -> T { return upper.x - lower.x; }
    [[nodiscard]] auto height() const noexcept -> T { return upper.y - lower.y; }
  };

  using Coord = GenCoord<std::uint32_t>;
  using PixelSet = GenCoord<IntSet<std::uint32_t>>;
  using Frame = GenFrame<float>;

  struct Args {
    Coord resolution = {.x = 1024U, .y = 768U};
    Frame frame = {.lower = {-2.0F, -1.2F}, .upper = {1.0F, 1.2F}};
    std::uint32_t maxiter = 4096U;
    std::uint32_t threads = 12U;
  };

  explicit Image(Args const&) noexcept;

  Image(Image const&) = delete;
  Image(Image&&) noexcept = default;

  auto operator=(Image const&) -> Image& = delete;
  auto operator=(Image&&) noexcept -> Image& = default;

  ~Image() = default;

  [[nodiscard, gnu::cold]] auto frame() const noexcept { return frame_; }
  [[nodiscard, gnu::cold]] auto resolution() const noexcept { return resolution_; }
  [[nodiscard, gnu::cold]] auto maxiter() const noexcept { return maxiter_; }
  [[nodiscard, gnu::cold]] auto data() const noexcept { return data_.get(); }

  auto save_pgm(std::string_view filename) const noexcept -> bool;

private:
  auto calc_(std::uint32_t y_begin, std::uint32_t y_end) noexcept -> void;

  Coord resolution_;
  Frame frame_;
  std::uint32_t maxiter_;

  std::uint32_t pixel_count_ = resolution_.x * resolution_.y;
  std::unique_ptr<std::uint32_t[]> data_ = std::make_unique<std::uint32_t[]>(pixel_count_);
};
