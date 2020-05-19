#include "image.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fmt/core.h>
#include <immintrin.h>
#include <numeric>

auto constexpr inline uset_1 = IntSet{1U};

Image::Image(Args const& args) noexcept
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter} {

  std::iota(pixel_offset_x_.lanes.begin(), pixel_offset_x_.lanes.end(), 0);
}

auto Image::clean() noexcept -> bool {
  // Check lane empty status //
  auto const l2sqnorm = z_.l2sqnorm();

  auto constexpr fset_4 = FloatSet{4.0F};
  auto const over_4 = l2sqnorm > fset_4;
  auto const reached_iter_limit = iter_ > uset_maxiter_;

  empty_ |= reached_iter_limit | over_4;

  if (_mm256_movemask_epi8(empty_.vec) != -1)
    return false;

  // Update picture //

  std::memcpy(pos_, iter_.lanes.cbegin(), sizeof(iter_.lanes));
  pos_ += iter_.lanes.size();

  // Update current pixel indices //

  auto const x_max_reached = current_.x > uset_x_max_;

  current_.x =
      (~x_max_reached & (current_.x + uset_lanes_incr_)) | (x_max_reached & pixel_offset_x_);

  current_.y += uset_1 & x_max_reached;

  std::copy(current_.x.lanes.cbegin(), current_.x.lanes.cend(), fset_px_.lanes.begin());

  c_.real = fset_px_ * fset_scaling_.real + fset_frame_lower_.x;
  c_.imag =
      FloatSet{static_cast<float>(current_.y.lanes[7])} * fset_scaling_.imag + fset_frame_lower_.y;

  iter_ ^= iter_;
  empty_ ^= empty_;
  z_.real ^= z_.real;
  z_.imag ^= z_.imag;

  return pos_ == data_.get() + pixel_count_;
}

auto Image::calc() noexcept -> void {
  while (!clean()) {
    auto const temp = z_.real * z_.real - z_.imag * z_.imag + c_.real;

    auto constexpr fset_2 = FloatSet{2.0F};
    z_.imag = fset_2 * z_.real * z_.imag + c_.imag;
    z_.real = temp;

    iter_ += uset_1 & ~empty_;
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
