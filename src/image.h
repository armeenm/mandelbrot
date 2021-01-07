#pragma once

#include "complex.h"
#include "set.h"
#include "util.h"

#include <memory>
#include <string_view>
#include <thread>

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

  using Coord = GenCoord<n32>;
  using PixelSet = GenCoord<IntSet<n32>>;
  using Frame = GenFrame<f32>;

  struct Args {
    Coord resolution = {.x = 1024U, .y = 768U};
    Frame frame = {.lower = {-2.0F, -1.2F}, .upper = {1.0F, 1.2F}};
    n32 maxiter = 4096U;
    n32 threads = std::jthread::hardware_concurrency();
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
  auto calc_(n32 y_begin, n32 y_end) noexcept -> void;

  Coord resolution_;
  Frame frame_;
  n32 maxiter_;
  n32 threads_;

  n32 pixel_count_ = resolution_.x * resolution_.y;
  std::unique_ptr<n32[]> data_{new (std::align_val_t(64)) n32[pixel_count_]};
};
