#include "image.h"

#include <algorithm>
#include <cstdio>
#include <fmt/core.h>
#include <immintrin.h>
#include <numeric>

Image::Image(Args const& args) noexcept
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter} {
  update_c();

  std::iota(pixel_offset_x_.lanes.begin(), pixel_offset_x_.lanes.end(), 0);
}

auto Image::clean() noexcept -> bool {
  /* Check lane empty status */
  auto const l2sqnorm = z_.l2sqnorm();

  auto constexpr fset_4 = FloatSet{4.0F};
  auto const over_4 = l2sqnorm > fset_4;
  auto const reached_iter_limit = iter_ >= uset_maxiter_;
  auto const empty = reached_iter_limit | over_4;

  /* If nothing is empty/finished, move on */
  if (!_mm256_movemask_epi8(empty.vec))
    return false;

  /* Calculate pixel index */
  auto const px_idx = current_.y * uset_res_x_ + current_.x;

  /* Update picture */
  for (std::size_t i = 0; i < iter_.lanes.size(); ++i)
    if (empty.lanes[i])
      data_[px_idx.lanes[i]] = iter_.lanes[i];

  /* Check lane completion status */
  auto const finished = empty & (px_idx >= uset_px_idx_max_);

  [[unlikely]] if (_mm256_movemask_epi8(finished.vec) == -1) return true;

  auto const x_max_reached = current_.x > uset_x_max_;

  /* Update current pixel indices */
  auto const only_empty = empty & (px_idx <= uset_px_idx_max_);

  current_.x = (~empty & current_.x) | (only_empty & ~x_max_reached & (current_.x + uset_lanes_)) |
               (only_empty & x_max_reached & pixel_offset_x_);

  auto constexpr uset_1 = IntSet{1U};
  current_.y += uset_1 & empty & x_max_reached;

  iter_ &= ~empty;
  z_.real &= ~empty;
  z_.imag &= ~empty;

  update_c();

  return false;
}

auto Image::calc() noexcept -> void {
  auto const temp = z_.real * z_.real - z_.imag * z_.imag + c_.real;

  auto constexpr fset_2 = FloatSet{2.0F};
  z_.imag = fset_2 * z_.real * z_.imag + c_.imag;
  z_.real = temp;

  auto constexpr iter_incr = IntSet{1U};
  iter_ += iter_incr;
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
  auto fset_px = FloatSet{};
  auto fset_py = FloatSet{};

  std::copy(current_.x.lanes.begin(), current_.x.lanes.end(), fset_px.lanes.begin());
  std::copy(current_.y.lanes.begin(), current_.y.lanes.end(), fset_py.lanes.begin());

  c_.real = fset_px * fset_scaling_.real + fset_frame_lower_.x;
  c_.imag = fset_py * fset_scaling_.imag + fset_frame_lower_.y;
}
