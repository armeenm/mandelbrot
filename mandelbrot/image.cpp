#include "image.h"

#include <algorithm>
#include <cstdio>
#include <fmt/core.h>
#include <immintrin.h>
#include <numeric>

Image::Image(Args const& args) noexcept
    : resolution_(args.resolution), frame_(args.frame), maxiter_(args.maxiter) {
  update_c();

  std::iota(pixel_offset_x_.lanes.begin(), pixel_offset_x_.lanes.end(), 0);
}

auto Image::clean() noexcept -> bool {
  /* Check lane empty status */
  auto const l2sqnorm = z_.l2sqnorm();

  auto const static fset_4 = FloatSet{4.0F};
  auto const over_4 = l2sqnorm > fset_4;

  auto const static iset_iter_max = IntSet{maxiter_ - 1};
  auto const reached_iter_limit = iter_ > iset_iter_max;

  auto const empty = reached_iter_limit | over_4;

  /* If nothing is empty/finished, move on */
  if (!_mm256_movemask_epi8(empty.vec))
    return false;

  /* Calculate pixel index */
  auto const static uset_x_res = IntSet{resolution_.x};
  auto const px_idx = current_.y * uset_x_res + current_.x;

  /* Update picture */
  for (std::size_t i = 0; i < iter_.lanes.size(); ++i)
    if (empty.lanes[i])
      data_[px_idx.lanes[i]] = iter_.lanes[i];

  /* Check lane completion status */
  auto const static iset_px_idx_max = IntSet{pixel_count_ - 9};
  auto const finished = empty & (px_idx > iset_px_idx_max);

  [[unlikely]] if (_mm256_movemask_epi8(finished.vec) == -1) return true;

  auto const static iset_x_max = IntSet{resolution_.x - 9};
  auto const x_max_reached = current_.x > iset_x_max;

  /* Update current pixel indices */
  auto const static uset_lanes = IntSet{static_cast<std::uint32_t>(iter_.lanes.size())};
  auto const only_empty = empty & (px_idx <= iset_px_idx_max);

  current_.x = (~empty & current_.x) | (only_empty & ~x_max_reached & (current_.x + uset_lanes)) |
               (only_empty & x_max_reached & pixel_offset_x_);

  auto const static uset_1 = IntSet<std::uint32_t>{1};
  current_.y += uset_1 & empty & x_max_reached;

  iter_ &= ~empty;
  z_.real &= ~empty;
  z_.imag &= ~empty;

  update_c();

  return false;
}

auto Image::calc() noexcept -> void {
  auto constexpr ITERS = 1U;
  auto constexpr INCR = 1U;

  for (auto i = 0U; i < ITERS; ++i) {
    auto const temp = z_.real * z_.real - z_.imag * z_.imag + c_.real;

    auto const static fset_2 = FloatSet{2.0F};
    z_.imag = fset_2 * z_.real * z_.imag + c_.imag;
    z_.real = temp;

    auto const static uset_iter_incr = IntSet{INCR};
    iter_ += uset_iter_incr;
  }
}

auto Image::save_pgm(std::string_view filename) const noexcept -> bool {
  auto fp = std::fopen(filename.data(), "w");

  if (!fp)
    return false;

  /* Header */
  fmt::print(fp, "P2\n{} {} \n{} \n", resolution_.x, resolution_.y, maxiter_);

  /* Data */
  for (auto i = 0U; i < pixel_count_; ++i) {
    if (i % resolution_.x == 0)
      fmt::print(fp, "\n");

    fmt::print(fp, "{} ", data_[i]);
  }

  std::fclose(fp);
  return true;
}

auto Image::update_c() noexcept -> void {
  auto const static fset_scaling =
      Complex<FloatSet>{FloatSet{scaling_.real}, FloatSet{scaling_.imag}};
  auto const static fset_frame_lower = GenCoord<FloatSet>{frame_.lower.x, frame_.lower.y};

  auto fset_px = FloatSet{};
  auto fset_py = FloatSet{};

  std::copy(current_.x.lanes.begin(), current_.x.lanes.end(), fset_px.lanes.begin());
  std::copy(current_.y.lanes.begin(), current_.y.lanes.end(), fset_py.lanes.begin());

  c_.real = fset_px * fset_scaling.real + fset_frame_lower.x;
  c_.imag = fset_py * fset_scaling.imag + fset_frame_lower.y;
}
