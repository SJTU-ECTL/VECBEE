# Set the minimum version of CMake that can be used
# To find the cmake version run
# $ cmake --version
cmake_minimum_required(VERSION 3.5)

# Set the project name
add_subdirectory(abc)
project (sasimi-vecbee)

# collect source files
aux_source_directory(${PROJECT_SOURCE_DIR}/src sources)

# Add include paths
include_directories(${PROJECT_SOURCE_DIR}/abc/src)
include_directories(${PROJECT_SOURCE_DIR}/inc)

# Add definitions
add_definitions(-DLIN64 -g -Wall -O3 -std=c++2a)

# set output path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR})

# Add an executable with the above sources
add_executable(sasimi-vecbee.out ${sources})

# link libraries
target_link_libraries(sasimi-vecbee.out libabc -lm -ldl -rdynamic -lreadline -ltermcap -lpthread -lstdc++ -lrt -lstdc++fs)
