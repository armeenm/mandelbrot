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
  // Setup //
  auto constexpr uset_1 = IntSet{1U};

  auto const frame_lower = GenCoord{frame_.lower.x, frame_.lower.y};
  auto c = Complex<FloatSet>{frame_lower.x, frame_lower.y};

  auto z = Complex<FloatSet>{};
  auto zsq = Complex<FloatSet>{};
  auto px = FloatSet{};
  auto iter = IntSet{0U};
  auto empty = IntSet{0U};
  auto data_pos = data_.get();

  auto const x_max = IntSet{resolution_.x - 9};
  auto const scaling = Complex<FloatSet>{{frame_.width() / static_cast<float>(resolution_.x)},
                                         {frame_.height() / static_cast<float>(resolution_.y)}};
  auto const lanes_incr = IntSet{static_cast<std::uint32_t>(iter.lanes.size())};
  auto const uset_maxiter = IntSet{maxiter_ - 1};

  auto const px_x_offset = []() {
    auto ret = IntSet{0U};
    std::iota(ret.lanes.begin(), ret.lanes.end(), 0);
    return ret;
  }();

  auto current = PixelSet{.x = px_x_offset, .y = 0U};

  // Loop //
  do {
    z.imag = (z.real + z.real) * z.imag + c.imag;
    z.real = zsq.real - zsq.imag + c.real;
    zsq.real = z.real * z.real;
    zsq.imag = z.imag * z.imag;

    iter += uset_1 & ~empty;

    // Check lane empty status //
    auto constexpr fset_4 = FloatSet{4.0F};
    auto const over_4 = (zsq.real + zsq.imag) > fset_4;
    auto const reached_iter_limit = iter > uset_maxiter;

    empty |= reached_iter_limit | over_4;

    // Check if all current pixels are empty //
    if (_mm256_movemask_epi8(empty.vec) == -1) {
      // Update picture //
      std::memcpy(data_pos, iter.lanes.cbegin(), sizeof(iter.lanes));
      data_pos += iter.lanes.size();

      // Update current pixel indices //
      auto const x_max_reached = current.x > x_max;

      current.x = (~x_max_reached & (current.x + lanes_incr)) | (x_max_reached & px_x_offset);
      current.y += uset_1 & x_max_reached;

      // Conver uint32's to floats and copy to px vector //
      std::copy(current.x.lanes.cbegin(), current.x.lanes.cend(), px.lanes.begin());

      c.real = px * scaling.real + frame_lower.x;
      c.imag = FloatSet{static_cast<float>(current.y.lanes[7])} * scaling.imag + frame_lower.y;

      // Clear out //
      iter ^= iter;
      empty ^= empty;
      z.real ^= z.real;
      z.imag ^= z.imag;
      zsq.real ^= zsq.real;
      zsq.imag ^= zsq.imag;
    }

  } while (__builtin_expect(data_pos != data_.get() + pixel_count_, 1));
}

auto Image::save_pgm(std::string_view const filename) const noexcept -> bool {
  auto fp = std::fopen(filename.data(), "w");

  if (!fp)
    return false;

  // Header //
  fmt::print(fp, "P2\n{} {} \n{} \n", resolution_.x, resolution_.y, maxiter_);

  // Data //
  for (auto i = 0U; i < pixel_count_; ++i) {
    if (i % resolution_.x == 0)
      std::putc('\n', fp);

    auto const s = fmt::format(FMT_COMPILE("{} "), data_[i]);
    std::fwrite(s.c_str(), sizeof(char), s.size(), fp);
  }

  std::fclose(fp);
  return true;
}
