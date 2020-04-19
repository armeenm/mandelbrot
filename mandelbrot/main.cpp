#include <chrono>
#include <fmt/core.h>
#include <string>

#include "image.h"

auto constexpr inline FILENAME = "mandelbrot.pgm";
auto constexpr inline X_RES = std::uint32_t{1024};
auto constexpr inline Y_RES = std::uint32_t{768};
auto constexpr inline MAXITER = std::uint32_t{4096};

auto main(int const argc, char const* const* const argv) -> int {
  if (argc == 3 || argc > 4) {
    fmt::print("Usage: {} FILENAME [XRES YRES]", argv[0]);
    return -1;
  }

  auto const filename = (argc > 1) ? argv[1] : FILENAME;
  auto const x_res = (argc > 2) ? static_cast<std::uint32_t>(std::stoul(argv[2])) : X_RES;
  auto const y_res = (argc > 2) ? static_cast<std::uint32_t>(std::stoul(argv[3])) : Y_RES;

  auto img = Image{{.resolution = Image::Coord{x_res, y_res}, .maxiter = MAXITER}};

  auto const start_comp = std::chrono::high_resolution_clock::now();

  while (!img.clean())
    img.calc();

  auto const end_comp = std::chrono::high_resolution_clock::now();

  img.save_pgm(filename);

  auto const end_save = std::chrono::high_resolution_clock::now();

  auto constexpr to_ms = [](auto const& start, auto const& end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  };

  fmt::print("Total time: {}ms\n", to_ms(start_comp, end_save));
  fmt::print("  Computation time: {}ms\n", to_ms(start_comp, end_comp));
  fmt::print("  Saving time: {}ms\n", to_ms(end_comp, end_save));
}
