cmake_minimum_required(VERSION 3.12...3.17)

include(cmake/BuildType.cmake)
include(cmake/ExportCompileCommands.cmake)

project(mandelbrot
  VERSION 0.0.1
  DESCRIPTION "Mandelbrot gen"
  LANGUAGES CXX)

option(FORCE_COLORED_OUTPUT
  "Always produce ANSI-colored output (GNU/Clang only)" 
  OFF)

option(STATIC_ANALYSIS
  "Enable static analysis tools"
  OFF)

set(CONAN_SYSTEM_INCLUDES ON)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

file(GLOB_RECURSE SRCS
  LIST_DIRECTORIES false
  CONFIGURE_DEPENDS
  "mandelbrot/*.cpp")

add_executable(mandelbrot ${SRCS})

target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

set(BUILD_OPTS
  -march=native
  -Wshadow
  -Wduplicated-cond
  -Wlogical-op
  -Wduplicated-branches
  -Wnull-dereference
  -Wold-style-cast
  -Wuseless-cast
  -Wformat=2
  -Wall
  -Wextra
  -Wconversion
  -Wpedantic
  -pipe
  -flto
  -fmerge-all-constants
  -fmodulo-sched-allow-regmoves
  -fgcse-sm
  -fgcse-las
  -faggressive-loop-optimizations
  -flive-range-shrinkage
  -fipa-pta
  -fisolate-erroneous-paths-attribute
  -ftree-loop-ivcanon
  -fivopts
  -funroll-loops
  -frounding-math
  -fsplit-loops
  -funswitch-loops
  -fpeel-loops
  -fno-signed-zeros
  -fno-trapping-math
  -Ofast)

target_link_libraries(mandelbrot PRIVATE gcov ${CONAN_LIBS})
target_compile_options(mandelbrot PRIVATE ${BUILD_OPTS})
target_compile_features(mandelbrot PRIVATE cxx_std_20)

#target_compile_options(mandelbrot PRIVATE
#  -fprofile-dir=${CMAKE_CURRENT_SOURCE_DIR}/profile
#  -fprofile-use)

if(${FORCE_COLORED_OUTPUT} OR ${CMAKE_GENERATOR} EQUAL "Ninja")
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(mandelbrot PRIVATE -fdiagnostics-color=always)
  elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    target_compile_options(mandelbrot PRIVATE -fcolor-diagnostics)
  endif()
endif()

if(${STATIC_ANALYSIS})
  set_target_properties(mandelbrot PROPERTIES
    CXX_CLANG_TIDY "clang-tidy;-checks=*"
    CXX_CPPCHECK "cppcheck;--std=c++17")
endif()
