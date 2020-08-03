# Requirements
C++20, fmtlib

# Building

```
conan install --build fmt -if build .
cmake -S. -Bbuild
cmake --build build
```

Binary will be produced at `build/bin/mandelbrot`.

# Usage

```
mandelbrot FILENAME [XRES YRES]
```
