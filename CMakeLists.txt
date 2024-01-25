cmake_minimum_required(VERSION 3.26)
project(SimpleECS C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include scripts
include(${CMAKE_SOURCE_DIR}/cmake/FetchCMocka.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/TestMacros.cmake)

# Enable tests
# TODO: Configure testing
enable_testing()
setup_test(ecs ecs.h ecs.c)

add_library(simpleECS_static ecs.h ecs.c)