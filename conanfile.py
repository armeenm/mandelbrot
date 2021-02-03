from conans import ConanFile, CMake, tools

class Mandelbrot(ConanFile):
    name = 'mandelbrot'
    version = '1.0.0'
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake_find_package_multi'
    requires = ['fmt/7.1.3', 'imgui/1.80', 'glfw/3.3.2', 'glew/2.2.0']
    build_policy = 'missing'

    def imports(self):
        self.copy('imgui_impl_glfw.cpp', src='./misc/bindings', dst='../bindings')
        self.copy('imgui_impl_opengl2.cpp', src='./misc/bindings', dst='../bindings')
        self.copy('imgui_impl_glfw.h', src='./misc/bindings', dst='../bindings')
        self.copy('imgui_impl_opengl2.h', src='./misc/bindings', dst='../bindings')

    def build(self):
        cmake = CMake(self)
        cmake.configure(args=['-G', 'Ninja Multi-Config'])
        cmake.build()
