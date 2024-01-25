cmake_minimum_required(VERSION 3.26)
project(SimpleECS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build SimpleECS unit tests" ON)

# Include scripts
include(${CMAKE_SOURCE_DIR}/cmake/FetchCMocka.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TestMacros.cmake)

# Testing targets
if (BUILD_TESTS)
    enable_testing()
    setup_test(ecs ecs.h ecs.c)
endif()

# Default target
add_library(SimpleECS ecs.h ecs.c)