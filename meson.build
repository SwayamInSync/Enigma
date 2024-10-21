project(
  'enigma',
  'cpp',
  version: '0.1.0',
  default_options: ['cpp_std=c++20']
)

# Dependencies
python3 = import('python').find_installation()

# Include directories
inc_dir = include_directories('include')



# Example programs
executable(
  'basic_example',
  'examples/basic_example.cpp',
  include_directories: inc_dir,
  dependencies: [cuda_dep]
)
