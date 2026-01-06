project(
  'epsp_linux_client',
  'cpp',
  version: '0.01a',
  meson_version: '>=1.10.0',
  license: 'gpl',
  default_options: [
    'cpp_std=c++23',
    'warning_level=3',
    'buildtype=debugoptimized',
    'b_lto=true',
  ],
)

# Dependencies
asio = dependency(
  'asio',
  fallback: ['asio', 'asio_dep'],
  default_options: ['default_library=static'],
  required: true,
)
spdlog = dependency(
  'spdlog',
  fallback: ['spdlog', 'spdlog_dep'],
  default_options: ['default_library=static'],
  required: true,
)

# Include directories
# incl = include_directories('include')
subdir('src')
subdir('tests')

sanitize_opts = []
if get_option('buildtype') == 'debugoptimized'
  sanitize_opts = ['b_sanitize=address,undefined,leak']
  catch2 = dependency('catch2-with-main', required: true)
endif

epsp_lib = static_library(
  'epsp',
  lib_src,
  cpp_pch: 'src/pch.h',
  # include_directories: [incl],
  dependencies: [asio, spdlog],
  override_options: sanitize_opts,
)
executable(
  'epsp',
  'src/main.cpp',
  cpp_pch: 'src/pch.h',
  # include_directories: [incl],
  dependencies: [asio, spdlog],
  override_options: sanitize_opts,
  link_with: epsp_lib,
  install: true,
  build_subdir: 'bin',
)

if get_option('buildtype') == 'debugoptimized'
  epsp_tests = executable(
    'epsp_tests',
    test_src,
    cpp_pch: 'src/pch.h',
    # include_directories: [incl],
    dependencies: [asio, catch2, spdlog],
    override_options: sanitize_opts,
    link_with: epsp_lib,
    build_subdir: 'bin_test',
  )
  test('epsp_tests', epsp_tests)
endif
