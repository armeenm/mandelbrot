#include <chrono>
#include <fmt/core.h>
#include <string>
#include <string_view>

#include "image.h"

auto constexpr inline filename_def = "mandelbrot.pgm";

auto main(int const argc, char const* const* const argv) -> int {
  if (argc == 3 || argc > 4) {
    fmt::print("Usage: {} FILENAME [XRES YRES]", argv[0]);
    return -1;
  }

  auto const filename = (argc > 1) ? argv[1] : filename_def;

  auto constexpr stoi = [](std::string_view str) {
    return static_cast<std::uint32_t>(std::stoul(str.data()));
  };

  auto const args = [=] {
    if (argc > 2)
      return Image::Args{.resolution = Image::Coord{.x = stoi(argv[2]), .y = stoi(argv[3])}};
    else
      return Image::Args{};
  }();

  auto img = Image{args};

  auto const start_comp = std::chrono::high_resolution_clock::now();

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
