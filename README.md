# Requirements
C++20, fmtlib

# Building

```
conan install -if build --build=missing .
conan build -bf build .
```

Binary will be produced at `build/mandelbrot`.

# Usage

```
mandelbrot FILENAME [XRES YRES]
```
