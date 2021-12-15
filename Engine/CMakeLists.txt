cmake_minimum_required(VERSION 3.10)
project(KoPL)

set(CMAKE_CXX_STANDARD 14)

add_library(KoPL STATIC fast_engine.cpp)
add_executable(debug_kopl fast_engine.cpp debug.cpp)

find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

target_link_libraries(KoPL ${PYTHON_LIBRARIES})
target_link_libraries(debug_kopl ${PYTHON_LIBRARIES})