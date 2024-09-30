cmake_minimum_required(VERSION 3.10)
project(enigma)

set(CMAKE_CXX_STANDARD 14)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

add_library(enigma MODULE
    src/tensor.cpp
    src/enigma_module.cpp
)

target_include_directories(enigma PRIVATE src)
target_link_libraries(enigma ${PYTHON_LIBRARIES})

set_target_properties(enigma PROPERTIES PREFIX "")
if(WIN32)
    set_target_properties(enigma PROPERTIES SUFFIX ".pyd")
else()
    set_target_properties(enigma PROPERTIES SUFFIX ".so")
endif()