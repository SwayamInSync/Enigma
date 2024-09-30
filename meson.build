project('enigma', 'cpp',
  version : '0.1',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

py_mod = import('python')
py3 = py_mod.find_installation()
py3_dep = py3.dependency()

incdir = include_directories('enigma/core')

shared_module('enigma',
  ['enigma/core/tensor.cpp', 'enigma/core/enigma_module.cpp'],
  include_directories : incdir,
  dependencies : py3_dep,
  install : true,
  install_dir : py3.get_install_dir() / 'enigma/core',
  name_prefix : ''
)

install_data('enigma/__init__.py', install_dir: py3.get_install_dir() / 'enigma')
install_data('enigma/core/__init__.py', install_dir: py3.get_install_dir() / 'enigma/core')