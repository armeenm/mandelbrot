# Requirements
C++20, fmtlib

# Building

```
conan install --build fmt -if builds/Release .
cmake -H. -Bbuilds/Release
cmake --build builds/Release
```

Binary will be produced at `builds/Release/bin/mandelbrot`.

# Usage

```
mandelbrot FILENAME [XRES YRES]
```
