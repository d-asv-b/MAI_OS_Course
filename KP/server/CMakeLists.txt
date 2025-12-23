cmake_minimum_required(VERSION 3.10)
project(BullsAndCowsServer CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Add server executable
add_executable(server
    main.cpp
    ../common.cpp
    src/Game.cpp
    src/GameManager.cpp
    src/Server.cpp
)

# Include directories
target_include_directories(server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
)

set_target_properties(server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../build"
    RUNTIME_OUTPUT_NAME "server"
)
