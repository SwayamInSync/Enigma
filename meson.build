project(
  'enigma',
  'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++20']
)

# Include directories
inc_dir = include_directories('include')

# Source files
src_files = [
  'src/Allocator.cpp',
  'src/COW.cpp',
  'src/Device.cpp',
  'src/DeviceType.cpp',
  'src/Storage.cpp',
  'src/Scalar.cpp'
]

# Compiler flags
cpp_args = ['-Wall', '-Wextra']

# Main library
enigma_lib = static_library('enigma',
  src_files,
  include_directories: inc_dir,
  cpp_args: cpp_args
)

# Test Dependencies
gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')
gtest_main_dep = gtest_proj.get_variable('gtest_main_dep')

# Test files
test_files = [
  'tests/storage_cow_tests.cpp',
  'tests/scalar_tests.cpp'
]

# Build and register tests
foreach test_file : test_files
    test_name = test_file.split('/')[-1].split('.')[0]
    test_exe = executable(test_name,
        test_file,
        include_directories: inc_dir,
        link_with: enigma_lib,
        dependencies: [gtest_dep, gtest_main_dep],
        cpp_args: cpp_args
    )
    test(test_name, 
         test_exe, 
         protocol: 'gtest',
         args: ['--gtest_color=yes'],
         env: ['GTEST_COLOR=1'],
         verbose: true)
endforeach