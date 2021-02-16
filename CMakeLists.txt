cmake_minimum_required(VERSION 3.10)

# set the project name
project(THUAIEgg)


# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(unofficial-box2d CONFIG REQUIRED)
target_link_libraries(main PRIVATE unofficial::box2d::box2d)

# add the executable
add_executable(THUAIEgg main.cpp)
