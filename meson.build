project('bitstruct', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++11'])

bitstruct_dep = declare_dependency(
  include_directories: include_directories('.'))

test_exe = executable('bitstruct_test', 'bitstruct_test.cpp', dependencies: bitstruct_dep)
test('bitstruct', test_exe)

install_headers('bitstruct.hpp', subdir : 'bitstruct')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'bitstruct',
  filebase : 'bitstruct',
  description : 'Library to replace bitfields',
  subdirs : 'bitstruct',
  version : '1.0',
)
