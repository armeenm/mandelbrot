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
#include <memory>
#include <numeric>
#include <thread>
#include <vector>

Image::Image(Args const& args) noexcept
    : resolution_{args.resolution}, frame_{args.frame}, maxiter_{args.maxiter},
      thread_count_{args.thread_count} {

  auto thread_pool = std::vector<std::jthread>{};
  auto idx = std::atomic<n32>{};

  for (auto i = 0U; i < thread_count_; ++i)
    thread_pool.emplace_back(&Image::calc_, this, std::ref(idx));
}

auto Image::calc_(std::atomic<n32>& idx) noexcept -> void {
  auto const t_start = std::chrono::high_resolution_clock::now();

  auto constexpr block_size = 128U;
  auto constexpr uset_1 = IntSet{1U};
  auto constexpr fset_4 = FloatSet{4.0F};
  auto constexpr maxperiod = 350U;
  auto constexpr simd_width = static_cast<n32>(uset_1.lanes.size());
  auto constexpr px_x_offset = []() {
    auto ret = IntSet{0U};
    std::iota(ret.lanes.begin(), ret.lanes.end(), 0);
    return ret;
  }();

  auto const uset_limiter = IntSet{maxiter_ - 1U};

  auto const scaling = Complex<FloatSet>{frame_.width() / static_cast<f32>(resolution_.x),
                                         frame_.height() / static_cast<f32>(resolution_.y)};

  auto period = 0U;
  auto pxidx = n32{};

  while ((pxidx = idx.fetch_add(simd_width * block_size, std::memory_order_relaxed)) <
         (pixel_count_ / 2))
    [[likely]] {

      for (auto i = 0U; i < block_size; ++i) {
        auto const px = Complex<IntSet<n32>>{IntSet{pxidx % resolution_.x} + px_x_offset,
                                             pxidx / resolution_.x};

        auto const [c, inside] = [&] {
          auto const px_float =
              Complex{static_cast<FloatSet>(px.real), static_cast<FloatSet>(px.imag)};

          auto const _c = Complex{px_float.real * scaling.real + frame_.lower.x,
                                  px_float.imag * scaling.imag + frame_.lower.y};

          auto const x = _c.real;
          auto const y = _c.imag;

          auto const a = x - 0.25F;
          auto const b = x + 1.0F;

          auto const q = a * a + y * y;

          auto const in_cardioid = q * (q + a) <= FloatSet{0.25F} * y * y;
          auto const in_b2 = b * b + y * y <= 0.0625F;

          return std::make_pair(std::move(_c),
                                IntSet<n32>{bit_cast<__m256i>((in_cardioid | in_b2).vec)});
        }();

        auto z = Complex<FloatSet>{};
        auto zsq = Complex<FloatSet>{};
        auto zold = Complex<FloatSet>{};

        auto iter = uset_limiter & inside;
        auto done = inside;

        while (done.movemask() != -1) {
          z.imag = (z.real + z.real) * z.imag + c.imag;
          z.real = zsq.real - zsq.imag + c.real;
          zsq.real = z.real * z.real;
          zsq.imag = z.imag * z.imag;

          // Check lane done status //
          auto const over_4 = (zsq.real + zsq.imag) > fset_4;
          auto const reached_iter_limit = iter >= uset_limiter;
          auto const reached_period_limit = period > maxperiod;
          auto const repeat = FloatSet{(z.real == zold.real) & (z.imag == zold.imag)};

          done |= reached_iter_limit | over_4 | repeat;
          iter = (iter & ~repeat) | (uset_limiter & repeat);

          if (reached_period_limit) {
            period = 0;
            zold = z;
          }

          iter += uset_1 & ~done;
          ++period;
        }

        // Update picture //
        // TODO: This should calculate the location of the y-axis //
        auto const mirror = resolution_.y - 1U - 2 * px.imag.lanes[0];

        iter.store(&data_[pxidx]);
        iter.store(&data_[pxidx + mirror * resolution_.x]);

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
