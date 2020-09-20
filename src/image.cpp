#include "image.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <fmt/compile.h>
#include <fmt/core.h>
#include <immintrin.h>
#include <numeric>
#include <thread>
#include <vector>

Image::Image(Args const& args) noexcept
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter} {

  auto threads = std::vector<std::jthread>{};

  calc_(0U, resolution_.y);

  //  for (auto i = 0U; i < std::jthread::hardware_concurrency; ++i) {
  //    threads.emplace_back(calc_(0U, resolution_.y));
  //  }
}

auto Image::calc_(std::uint32_t const y_begin, std::uint32_t const y_end) noexcept -> void {
  fmt::print("Calc @@ {}-{}\n", y_begin, y_end);

  auto constexpr uset_1 = IntSet{1U};
  auto constexpr fset_4 = FloatSet{4.0F};
  auto constexpr uset_maxperiod = IntSet{150U};
  auto constexpr uset_simd_width = IntSet{static_cast<std::uint32_t>(uset_1.lanes.size())};
  auto constexpr px_x_offset = []() {
    auto ret = IntSet{0U};
    std::iota(ret.lanes.begin(), ret.lanes.end(), 0);
    return ret;
  }();

  auto const x_max = IntSet{resolution_.x - 9U};
  auto const scaling = Complex<FloatSet>{{frame_.width() / static_cast<float>(resolution_.x)},
                                         {frame_.height() / static_cast<float>(resolution_.y)}};
  auto const uset_iter_limit = IntSet{maxiter_ - 1};

  auto c = Complex<FloatSet>{frame_.lower.x, frame_.lower.y};
  auto z = Complex<FloatSet>{};
  auto zsq = Complex<FloatSet>{};
  auto zold = Complex<FloatSet>{};

  auto px = FloatSet{};
  auto iter = IntSet{0U};
  auto empty = IntSet{0U};
  auto period = IntSet{0U};
  auto data_pos = data_.get();

  auto current_x = px_x_offset;
  auto current_y = y_begin;

  // Loop //
  do {
    z.imag = (z.real + z.real) * z.imag + c.imag;
    z.real = zsq.real - zsq.imag + c.real;
    zsq.real = z.real * z.real;
    zsq.imag = z.imag * z.imag;

    iter += uset_1 & ~empty;
    period += uset_1 & ~empty;

    // Check lane empty status //
    auto const over_4 = (zsq.real + zsq.imag) > fset_4;
    auto const reached_iter_limit = iter > uset_iter_limit;
    auto const reached_period_limit = period > uset_maxperiod;
    auto const repeat = FloatSet{(z.real == zold.real) & (z.imag == zold.imag)};

    empty |= reached_iter_limit | over_4 | repeat;
    iter = (iter & ~repeat) | (uset_iter_limit & repeat);
    period &= ~reached_period_limit;
    zold.real = (zold.real & ~reached_period_limit) | (z.real & reached_period_limit);
    zold.imag = (zold.imag & ~reached_period_limit) | (z.imag & reached_period_limit);

    // Check if all current pixels are empty //
    if (_mm256_movemask_epi8(empty.vec) == -1) {
      // Update picture //
      std::memcpy(data_pos, iter.lanes.cbegin(), sizeof(iter.lanes));
      data_pos += iter.lanes.size();

      // Update current pixel indices //
      auto const x_max_reached = current_x > x_max;

      current_x = (~x_max_reached & (current_x + uset_simd_width)) | (x_max_reached & px_x_offset);
      current_y += 1U & x_max_reached.lanes[0];

      // Conver uint32's to floats and copy to px vector //
      std::copy(current_x.lanes.cbegin(), current_x.lanes.cend(), px.lanes.begin());

      c.real = px * scaling.real + frame_.lower.x;
      c.imag = FloatSet{static_cast<float>(current_y)} * scaling.imag + frame_.lower.y;

      // Clear out //
      iter ^= iter;
      empty ^= empty;
      z.real ^= z.real;
      z.imag ^= z.imag;
      zsq.real ^= zsq.real;
      zsq.imag ^= zsq.imag;
    }

  } while (__builtin_expect(current_y != y_end, 1));
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
