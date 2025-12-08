cmake_minimum_required(VERSION 3.10)
project(Lab4)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# First library
add_library(lib_1 SHARED ./src/lib1_impl.cpp)
set_target_properties(lib_1 PROPERTIES
    VERSION 1.0
    SOVERSION 1
)

# Second library
add_library(lib_2 SHARED ./src/lib2_impl.cpp)
set_target_properties(lib_2 PROPERTIES
    VERSION 1.0
    SOVERSION 1
)

# Program 1: Static linking to lib_1
add_executable(program1 ./src/program1.cpp)
target_link_libraries(program1 PRIVATE lib_1)
target_include_directories(program1 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Program 2: Dynamic loading of both libraries
add_executable(program2 ./src/program2.cpp)
target_link_libraries(program2 PRIVATE dl)
target_include_directories(program2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


