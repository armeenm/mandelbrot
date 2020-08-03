#pragma once

#include "complex.h"
#include "set.h"

#include <array>
#include <cstdint>
#include <fmt/ostream.h>
#include <memory>
#include <string_view>

template <typename T> struct GenCoord { T x, y; };

class Image {
public:
  template <typename T> struct GenFrame;

  using Coord = GenCoord<std::uint32_t>;
  using PixelSet = GenCoord<IntSet<std::uint32_t>>;
  using Frame = GenFrame<float>;

  template <typename T> struct GenFrame {
    GenCoord<T> lower, upper;

    [[nodiscard]] auto width() const noexcept -> T { return upper.x - lower.x; }
    [[nodiscard]] auto height() const noexcept -> T { return upper.y - lower.y; }
  };

  struct Args {
    Coord resolution = {.x = 1024, .y = 768};
    Frame frame = {.lower = {-2.0F, -1.2F}, .upper = {1.0F, 1.2F}};
    std::uint32_t maxiter = 4096;
  };

  explicit Image(Args const&) noexcept;

  Image(Image const&) = delete;
  Image(Image&&) noexcept = default;

  auto operator=(Image const&) -> Image& = delete;
  auto operator=(Image&&) noexcept -> Image& = default;

  ~Image() = default;

  [[nodiscard]] [[gnu::cold]] auto frame() const noexcept { return frame_; }
  [[nodiscard]] [[gnu::cold]] auto resolution() const noexcept { return resolution_; }
  [[nodiscard]] [[gnu::cold]] auto maxiter() const noexcept { return maxiter_; }
  [[nodiscard]] [[gnu::cold]] auto data() const noexcept { return data_.get(); }

  auto calc() noexcept -> void;
  auto save_pgm(std::string_view filename) const noexcept -> bool;

private:
  IntSet<std::uint32_t> pixel_offset_x_;

  Coord resolution_;
  Frame frame_;
  std::uint32_t maxiter_;

  std::uint32_t pixel_count_ = resolution_.x * resolution_.y;

  std::unique_ptr<std::uint32_t[]> data_ = std::make_unique<std::uint32_t[]>(pixel_count_);
};
