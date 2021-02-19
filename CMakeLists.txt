cmake_minimum_required(VERSION 3.10)
message("123")

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
  CACHE STRING "Vcpkg toolchain file")

# strange bug on windows
if(CMAKE_SYSTEM_NAME MATCHES "Windows")
  set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/tools/vcpkg/installed/x86-windows/share/unofficial-box2d)
endif(CMAKE_SYSTEM_NAME MATCHES "Windows")

# set the project name
project(egg2021)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
set(JSON_BuildTests OFF CACHE INTERNAL "")

# json lib
add_subdirectory(include/json)

# add the executable
add_executable(egg2021 main.cpp)

find_package(unofficial-box2d CONFIG REQUIRED)
target_link_libraries(egg2021 PRIVATE unofficial::box2d::box2d)
target_link_libraries(egg2021 PRIVATE nlohmann_json::nlohmann_json)
