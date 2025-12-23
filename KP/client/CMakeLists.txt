cmake_minimum_required(VERSION 3.10)
project(BullsAndCowsClient CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add client executable
add_executable(client
    main.cpp
    src/Client.cpp
    ../common.cpp
)

# Include directories
target_include_directories(client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_target_properties(client PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build"
    RUNTIME_OUTPUT_NAME "client"
)
