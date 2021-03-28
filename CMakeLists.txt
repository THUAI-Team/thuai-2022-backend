cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

# set the project name
project(egg2021)

message("Now building on:${CMAKE_SYSTEM_NAME}")

# strange bug on windows
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(unofficial-box2d_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/installed/x64-windows/share/unofficial-box2d")
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

# turn off asan on CI
if(NOT DEFINED ENV{CI})
  message("ASan enabled")
  set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
else()
  message("Building on CI, ASan disabled")
endif()

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# json lib
add_subdirectory(include/json)
include_directories(include/json/include)
include_directories(include/record-sdk/)

file(GLOB SOURCES "*.cpp" "include/record-sdk/cpp_record_sdk/entities.cpp" "include/record-sdk/cpp_record_sdk/record.cpp")

# add the executable
add_executable(main ${SOURCES})

find_package(unofficial-box2d CONFIG REQUIRED)
target_include_directories(main PRIVATE include/cpp_record_sdk)
target_link_libraries(main PRIVATE unofficial::box2d::box2d)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
