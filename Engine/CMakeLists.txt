cmake_minimum_required(VERSION 3.10)
project(KoPL)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")

add_library(KoPL SHARED value.cpp value.h engine.cpp engine.h PyEngine.cpp)
add_executable(main engine.cpp engine.h value.cpp value.h main.cpp)



find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})

set(nlohmann_json_DIR /home/user/workspace/json/build)
message(${nlohmann_json_DIR})
find_package(nlohmann_json REQUIRED)


target_link_libraries(KoPL ${PYTHON_LIBRARIES} nlohmann_json::nlohmann_json)
target_link_libraries(main ${PYTHON_LIBRARIES} nlohmann_json::nlohmann_json)

