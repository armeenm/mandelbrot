cmake_minimum_required(VERSION 3.12...3.19)

include(cmake/BuildType.cmake)
include(cmake/ExportCompileCommands.cmake)

project(mandelbrot
  VERSION 1.0.0
  DESCRIPTION "Mandelbrot gen"
  LANGUAGES CXX)

option(FORCE_COLORED_OUTPUT
  "Always produce ANSI-colored output (GNU/Clang only)" 
  OFF)

option(STATIC_ANALYSIS
  "Enable static analysis tools"
  OFF)

file(GLOB_RECURSE SRCS
  LIST_DIRECTORIES false
  CONFIGURE_DEPENDS
  "src/*.cpp")

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)
find_package(fmt REQUIRED CONFIG)

add_executable(mandelbrot ${SRCS})

target_include_directories(mandelbrot PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set(LINK_COMPILE_OPTS)
set(COMPILE_OPTS
  ${LINK_COMPILE_OPTS}
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
  -pipe)
set(LINK_OPTS ${LINK_COMPILE_OPTS})

set(DEBUG_LINK_COMPILE_OPTS -fno-omit-frame-pointer -fsanitize=address,undefined)
set(DEBUG_COMPILE_OPTS ${DEBUG_LINK_COMPILE_OPTS})
set(DEBUG_LINK_OPTS ${DEBUG_LINK_COMPILE_OPTS})

set(RELEASE_LINK_COMPILE_OPTS -flto)
set(RELEASE_COMPILE_OPTS
  ${RELEASE_LINK_COMPILE_OPTS}
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
  -fgraphite-identity
  -floop-block
  -floop-interchange
  -floop-strip-mine
  -fdiagnostics-color=always
  -Ofast)
set(RELEASE_LINK_OPTS ${RELEASE_LINK_COMPILE_OPTS})

target_compile_features(mandelbrot PRIVATE cxx_std_20)
target_compile_options(mandelbrot PRIVATE
  ${COMPILE_OPTS}
  $<$<CONFIG:DEBUG>:${DEBUG_COMPILE_OPTS}>
  $<$<CONFIG:RELEASE>:${RELEASE_COMPILE_OPTS}>)

target_link_libraries(mandelbrot PRIVATE fmt::fmt Threads::Threads)
target_link_options(mandelbrot PRIVATE
  ${LINK_OPTS}
  $<$<CONFIG:DEBUG>:${DEBUG_LINK_OPTS}>
  $<$<CONFIG:RELEASE>:${RELEASE_LINK_OPTS}>)

if(FORCE_COLORED_OUTPUT OR CMAKE_GENERATOR MATCHES "^Ninja")
  add_compile_options(
    $<$<CXX_COMPILER_ID:GNU>:-fdiagnostics-color=always>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-fcolor-diagnostics>)
endif()

if(STATIC_ANALYSIS)
  set_target_properties(mandelbrot PROPERTIES
    CXX_CLANG_TIDY "clang-tidy;-checks=*"
    CXX_CPPCHECK "cppcheck;--std=c++17")
endif()
