project('mandelbrot',
  'cpp',
  version: '0.0.1',
  meson_version: '>=0.57.0',
  default_options: [
    'cpp_std=c++20',
    'buildtype=release',
    'b_ndebug=if-release',
    'warning_level=3',
    'b_lto=true',
    'pkg_config_path=' + meson.current_build_dir()
  ]
)

if get_option('use_conan')
  conan = find_program('conan', required: false)

  if conan.found()
    if get_option('buildtype').startswith('release')
      conan_bt = 'build_type=Release'
    else
      conan_bt = 'build_type=Debug'
    endif

    r = run_command(conan,
      'install', meson.current_source_dir(),
      '-if', meson.current_build_dir(),
      '-s', conan_bt,
      '--build=missing')

    if r.returncode() != 0
      warning('Failed to install dependencies with Conan')
    endif
  endif
endif

fmt = dependency('fmt', include_type: 'system')

comp = meson.get_compiler('cpp')
if comp.get_argument_syntax() == 'msvc'
  ignore_warns = '/w'
else
  ignore_warns = '-w'
  add_project_arguments(
    '-Wshadow',
    '-Wconversion',
    '-Wlogical-op',
    '-Wduplicated-cond',
    '-Wduplicated-branches',
    '-Wformat=2',
    '-Wold-style-cast',
    '-Wuseless-cast',
    '-Wnull-dereference',
    language: 'cpp'
  )
endif

### Bot ###

mandelbrot_tgt = executable('mandelbrot',
  'src/main.cpp',
  dependencies: [fmt],
  install: true
)
