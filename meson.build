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
]

# Compiler flags
cpp_args = ['-Wall', '-Wextra', '-Werror']

# Main library
enigma_lib = static_library('enigma',
  src_files,
  include_directories: inc_dir,
  cpp_args: cpp_args
)

# Define the test executable
storage_cow_test = executable('storage_cow_test',
  'tests/storage_cow_tests.cpp',
  include_directories: inc_dir,
  link_with: enigma_lib,
  cpp_args: cpp_args
)

# Add the test
test('Storage and COW tests', storage_cow_test)