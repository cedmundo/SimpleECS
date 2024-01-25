# Macros
macro(setup_test TARGET TEST_SOURCES)
    add_executable(${TARGET}_test ${ARGN} ${TARGET}_test.c)
    target_link_libraries(${TARGET}_test PRIVATE cmocka-static m)
    add_test(NAME ${TARGET}_test COMMAND ${TARGET})
endmacro()
