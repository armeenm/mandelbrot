from conans import ConanFile, CMake, tools

class Mandelbrot(ConanFile):
    name = 'mandelbrot'
    version = '1.0.0'
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake_find_package_multi'
    requires = ['fmt/7.0.3']
    build_policy = 'missing'

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
