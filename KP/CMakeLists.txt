cmake_minimum_required(VERSION 3.10)
project(BullsAndCows CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/build)

# Add subdirectories
add_subdirectory(server)
add_subdirectory(client)
