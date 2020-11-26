#include "image.h"
#include "util.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <fmt/compile.h>
#include <fmt/core.h>
#include <numeric>
#include <thread>
#include <vector>

Image::Image(Args const& args) noexcept
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter}, threads_{
                                                                                    args.threads} {
  auto threads = std::vector<std::jthread>{};

  // TODO: Make this stuff more robust
  auto const div = resolution_.y / threads_;
  for (auto i = 0U; i < threads_; ++i)
    threads.emplace_back(&Image::calc_, this, i * div, (i + 1) * div);
}

auto Image::calc_(n32 const y_begin, n32 const y_end) noexcept -> void {
  auto const start = std::chrono::high_resolution_clock::now();

  auto constexpr uset_1 = IntSet{1U};
  auto constexpr fset_4 = FloatSet{4.0F};
  auto constexpr uset_maxperiod = IntSet{150U};
  auto constexpr uset_simd_width = IntSet{static_cast<n32>(uset_1.lanes.size())};
  auto constexpr px_x_offset = []() {
    auto ret = IntSet{0U};
    std::iota(ret.lanes.begin(), ret.lanes.end(), 0);
    return ret;
  }();

  auto const x_max = IntSet{resolution_.x - 9};
  auto const scaling_real = FloatSet{frame_.width() / static_cast<f32>(resolution_.x)};
  auto const scaling_imag = frame_.height() / static_cast<f32>(resolution_.y);
  auto const uset_iter_limit = IntSet{maxiter_ - 1};

  auto c_real = FloatSet{frame_.lower.x};
  auto c_imag = static_cast<f32>(y_begin) * scaling_imag + frame_.lower.y;
  auto z = Complex<FloatSet>{};
  auto zsq = Complex<FloatSet>{};
  auto zold = Complex<FloatSet>{};

  auto px = FloatSet{};
  auto iter = IntSet{0U};
  auto done = IntSet{0U};
  auto period = IntSet{0U};
  auto data_pos = reinterpret_cast<decltype(iter.vec)*>(data_.get() + resolution_.x * y_begin);

  auto current_x = px_x_offset;
  auto current_y = y_begin;

  // Loop //
  do {
    z.imag = (z.real + z.real) * z.imag + c_imag;
    z.real = zsq.real - zsq.imag + c_real;
    zsq.real = z.real * z.real;
    zsq.imag = z.imag * z.imag;

    iter += uset_1 & ~done;
    period += uset_1 & ~done;

    // Check lane done status //
    auto const over_4 = (zsq.real + zsq.imag) > fset_4;
    auto const reached_iter_limit = iter > uset_iter_limit;
    auto const reached_period_limit = period > uset_maxperiod;
    auto const repeat = FloatSet{(z.real == zold.real) & (z.imag == zold.imag)};

    done |= reached_iter_limit | over_4 | repeat;
    iter = (iter & ~repeat) | (uset_iter_limit & repeat);
    period &= ~reached_period_limit;
    zold.real = (zold.real & ~reached_period_limit) | (z.real & reached_period_limit);
    zold.imag = (zold.imag & ~reached_period_limit) | (z.imag & reached_period_limit);

    // Check if all current pixels are done //
    if (done.movemask() == -1) {
      // TODO: Make this stuff aligned! //
      // Update picture //
      iter.store_unaligned(data_pos);
      ++data_pos;

      // Update current pixel indices //
      auto const x_max_reached = current_x > x_max;

      /* If all pixels are done, move on to the next row */
      if (x_max_reached.movemask() == -1) {
        current_x = px_x_offset;
        ++current_y;
      } else
        current_x += ~x_max_reached & uset_simd_width;

      // Update pixels from current_x position, converting from ints to floats */
      px = static_cast<FloatSet>(current_x);

      c_real = px * scaling_real + frame_.lower.x;
      c_imag = static_cast<f32>(current_y) * scaling_imag + frame_.lower.y;

      // Clear out //
      iter ^= iter;
      done ^= done;
      z.real ^= z.real;
      z.imag ^= z.imag;
      zsq.real ^= zsq.real;
      zsq.imag ^= zsq.imag;
    }

  } while (__builtin_expect(current_y != y_end, 1));

  auto const end = std::chrono::high_resolution_clock::now();
  fmt::print("calc_({}, {}): {}ms\n", y_begin, y_end, to_ms(start, end));
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
