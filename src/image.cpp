#include "image.h"
#include "util.h"

#include <algorithm>
#include <atomic>
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
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter},
      thread_count_{args.thread_count} {

  auto thread_pool = ThreadPool{};
  auto idx = std::atomic<n32>{};

  for (auto i = 0U; i < thread_count_; ++i)
    thread_pool.emplace_back(&Image::calc_, this, std::ref(idx));
}

auto Image::calc_(std::atomic<n32>& idx) noexcept -> void {
  auto const t_start = std::chrono::high_resolution_clock::now();

  auto constexpr block_size = 1024U;
  auto constexpr uset_1 = IntSet{1U};
  auto constexpr fset_4 = FloatSet{4.0F};
  auto constexpr uset_maxperiod = IntSet{150U};
  auto constexpr simd_width = static_cast<n32>(uset_1.lanes.size());
  auto constexpr px_x_offset = []() {
    auto ret = IntSet{0U};
    std::iota(ret.lanes.begin(), ret.lanes.end(), 0);
    return ret;
  }();

  auto const scaling_real = FloatSet{frame_.width() / static_cast<f32>(resolution_.x)};
  auto const scaling_imag = frame_.height() / static_cast<f32>(resolution_.y);
  auto const uset_iter_limit = IntSet{maxiter_ - 1};
  auto const data = data_.get();

  auto zold = Complex<FloatSet>{};
  auto period = IntSet{0U};
  auto pxidx = n32{};

  while ((pxidx = idx.fetch_add(simd_width * block_size, std::memory_order_relaxed)) < pixel_count_)
    [[likely]] {

      for (auto i = 0U; i < block_size; ++i) {
        auto current_x = IntSet<n32>{pxidx % resolution_.x} + px_x_offset;
        auto current_y = IntSet<n32>{pxidx / resolution_.x};

        auto c_real = static_cast<FloatSet>(current_x) * scaling_real + FloatSet{frame_.lower.x};
        auto c_imag = static_cast<FloatSet>(current_y) * scaling_imag + frame_.lower.y;

        auto z = Complex<FloatSet>{};
        auto zsq = Complex<FloatSet>{};
        auto iter = IntSet{0U};
        auto done = IntSet{0U};

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

        } while (done.movemask() != -1);

        // Update picture //
        iter.stream_store(&data[pxidx]);

        pxidx += simd_width;
      }
    }

  auto const t_end = std::chrono::high_resolution_clock::now();
  fmt::print("calc_(): {}ms\n", to_ms(t_start, t_end));
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
