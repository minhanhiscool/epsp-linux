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
glfw = dependency(
  'glfw3',
  fallback: ['glfw', 'glfw_dep'],
  default_options: ['default_library=static'],
  required: true,
)
imgui_dep = subproject('imgui')
imgui = imgui_dep.get_variable('imgui_dep')

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
  dependencies: [asio, spdlog, glfw, imgui],
  override_options: sanitize_opts,
)
executable(
  'epsp',
  'src/main.cpp',
  cpp_pch: 'src/pch.h',
  # include_directories: [incl],
  dependencies: [asio, spdlog, glfw, imgui],
  override_options: sanitize_opts,
  link_with: [epsp_lib],
  install: true,
  build_subdir: 'bin',
)

assets_src = meson.project_source_root() / 'src/assets'
assets_dst = meson.project_build_root() / 'bin/assets'

custom_target(
  'copy_assets',
  output: 'assets.stamp',
  command: [
    'python3',
    '-c', '''
import shutil, pathlib
shutil.copytree(r"'''
    + assets_src
    + '''", r"'''
    + assets_dst
    + '''", dirs_exist_ok=True)
pathlib.Path(r"@OUTPUT@").touch()
''',
  ],
  build_by_default: true,
  console: true,
)

if get_option('buildtype') == 'debugoptimized'
  epsp_tests = executable(
    'epsp_tests',
    test_src,
    cpp_pch: 'src/pch.h',
    # include_directories: [incl],
    dependencies: [asio, catch2, spdlog, imgui],
    override_options: sanitize_opts,
    link_with: [epsp_lib],
    build_subdir: 'bin_test',
  )
  assets_dst_test = meson.project_build_root() / 'bin_test/assets'

  custom_target(
    'copy_assets_test',
    output: 'assets_test.stamp',
    command: [
      'python3',
      '-c', '''
import shutil, pathlib
shutil.copytree(r"'''
      + assets_src
      + '''", r"'''
      + assets_dst_test
      + '''", dirs_exist_ok=True)
pathlib.Path(r"@OUTPUT@").touch()
''',
    ],
    build_by_default: true,
    console: true,
  )
  test('epsp_tests', epsp_tests)
endif
