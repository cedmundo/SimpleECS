cmake_minimum_required(VERSION 3.26)
project(SimpleECS C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build SimpleECS unit tests" ON)

# Default target
add_library(SimpleECS STATIC)
target_sources(SimpleECS
        PRIVATE ecs.c
        PUBLIC
            FILE_SET HEADERS
            BASE_DIRS ${PROJECT_SOURCE_DIR}
            FILES ecs.h
)
install(TARGETS SimpleECS FILE_SET HEADERS)

# Testing targets
if (BUILD_TESTS)
    # Include scripts
    include(cmake/FetchCMocka.cmake)
    include(cmake/TestMacros.cmake)

    enable_testing()
    setup_test(ecs ecs.h ecs.c)
endif()