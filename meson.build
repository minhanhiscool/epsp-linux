project(
  'epsp_linux_client',
  'cpp',
  version: '0.01a',
  meson_version: '>=1.10.0',
  default_options: ['cpp_std=c++20', 'warning_level=3', 'buildtype=debugoptimized'],
)

# Include directories
incl = include_directories('include')
subdir('3rdparty')

subdir('src')
executable(
  'epsp',
  src,
  include_directories: [incl, asio],
  install: true,
  build_subdir: 'bin',
)
