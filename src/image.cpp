#include "image.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fmt/compile.h>
#include <fmt/core.h>
#include <immintrin.h>
#include <numeric>

Image::Image(Args const& args) noexcept
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter} {}

auto Image::calc() noexcept -> void {
  auto constexpr uset_1 = IntSet{1U};
  auto iter = IntSet{0U};
  auto empty = IntSet{0U};

  auto pixel_x_offset = IntSet{0U};
  std::iota(pixel_x_offset.lanes.begin(), pixel_x_offset.lanes.end(), 0);

  auto fset_px = FloatSet{};
  auto fset_frame_lower = GenCoord{frame_.lower.x, frame_.lower.y};
  auto fset_scaling = Complex<FloatSet>{{frame_.width() / float(resolution_.x)},
                                        {frame_.height() / float(resolution_.y)}};

  auto current = PixelSet{.x = pixel_x_offset, .y = 0U};
  auto z = Complex<FloatSet>{};
  auto c = Complex<FloatSet>{fset_frame_lower.x, fset_frame_lower.y};

  auto uset_lanes_incr = IntSet{static_cast<std::uint32_t>(iter.lanes.size())};
  auto uset_x_max = IntSet{resolution_.x - 9};
  auto uset_maxiter = IntSet{maxiter_ - 1};

  auto data_pos = data_.get();

  do {
    auto const temp = z.real * z.real - z.imag * z.imag + c.real;

    auto constexpr fset_2 = FloatSet{2.0F};
    z.imag = fset_2 * z.real * z.imag + c.imag;
    z.real = temp;

    iter += uset_1 & ~empty;

    // Check lane empty status //
    auto const l2sqnorm = z.l2sqnorm();

    auto constexpr fset_4 = FloatSet{4.0F};
    auto const over_4 = l2sqnorm > fset_4;
    auto const reached_iter_limit = iter > uset_maxiter;

    empty |= reached_iter_limit | over_4;

    if (_mm256_movemask_epi8(empty.vec) == -1) {

      // Update picture //

      std::memcpy(data_pos, iter.lanes.cbegin(), sizeof(iter.lanes));
      data_pos += iter.lanes.size();

      // Update current pixel indices //

      auto const x_max_reached = current.x > uset_x_max;

      current.x =
          (~x_max_reached & (current.x + uset_lanes_incr)) | (x_max_reached & pixel_x_offset);

      current.y += uset_1 & x_max_reached;

      std::copy(current.x.lanes.cbegin(), current.x.lanes.cend(), fset_px.lanes.begin());

      c.real = fset_px * fset_scaling.real + fset_frame_lower.x;
      c.imag =
          FloatSet{static_cast<float>(current.y.lanes[7])} * fset_scaling.imag + fset_frame_lower.y;

      iter ^= iter;
      empty ^= empty;
      z.real ^= z.real;
      z.imag ^= z.imag;
    }

  } while (__builtin_expect(data_pos != data_.get() + pixel_count_, 1));
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
      std::putc('\n', fp);

    auto const s = fmt::format(FMT_COMPILE("{} "), data_[i]);
    std::fwrite(s.c_str(), sizeof(char), s.size(), fp);
  }

  std::fclose(fp);
  return true;
}
