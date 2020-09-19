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

  [[nodiscard]] [[gnu::cold]] auto frame() const noexcept { return frame_; }
  [[nodiscard]] [[gnu::cold]] auto resolution() const noexcept { return resolution_; }
  [[nodiscard]] [[gnu::cold]] auto maxiter() const noexcept { return maxiter_; }
  [[nodiscard]] [[gnu::cold]] auto data() const noexcept { return data_.get(); }

  auto calc() noexcept -> void;
  auto save_pgm(std::string_view filename) const noexcept -> bool;

private:
  IntSet<std::uint32_t> px_x_offset_ = []() {
    auto ret = IntSet{0U};
    std::iota(ret.lanes.begin(), ret.lanes.end(), 0);
    return ret;
  }();

  IntSet<std::uint32_t> pixel_offset_x_;
  Coord resolution_;
  Frame frame_;

  IntSet<std::uint32_t> x_max_ = {resolution_.x - 9};
  IntSet<std::uint32_t> lanes_incr_ = {static_cast<std::uint32_t>(x_max_.lanes.size())};

  GenCoord<float> frame_lower_ = {frame_.lower.x, frame_.lower.y};
  Complex<FloatSet> scaling_ = {{frame_.width() / static_cast<float>(resolution_.x)},
                                {frame_.height() / static_cast<float>(resolution_.y)}};

  std::uint32_t maxiter_;
  IntSet<std::uint32_t> uset_maxiter_ = {maxiter_ - 1};

  std::uint32_t pixel_count_ = resolution_.x * resolution_.y;

  std::unique_ptr<std::uint32_t[]> data_ = std::make_unique<std::uint32_t[]>(pixel_count_);
  std::vector<std::jthread> pool_;
};
