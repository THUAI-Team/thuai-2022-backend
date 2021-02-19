cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

# set the project name
project(egg2021)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# json lib
add_subdirectory(include/json)
include_directories(include/json/include)
include_directories(include/record-sdk/)

file(GLOB SOURCES "*.cpp" "include/record-sdk/cpp_record_sdk/entities.cpp" "include/record-sdk/cpp_record_sdk/record.cpp")

# add the executable
add_executable(egg2021 ${SOURCES})

find_package(unofficial-box2d CONFIG REQUIRED)
target_include_directories(egg2021 PRIVATE include/cpp_record_sdk)
target_link_libraries(egg2021 PRIVATE unofficial::box2d::box2d)
target_link_libraries(egg2021 PRIVATE nlohmann_json::nlohmann_json)
