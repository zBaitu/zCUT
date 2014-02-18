function(add_unit_test name)
    add_executable(${name} "${name}.c")
    foreach (lib "${ARGN}")
        target_link_libraries(${name} ${lib})
    endforeach()
endfunction()

include_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/build/lib)

set(ZCUT_MAIN_LIB zcut_main)
set(ZCUT_LIB zcut)

add_unit_test(test_sample ${ZCUT_MAIN_LIB})
add_unit_test(test_structure ${ZCUT_MAIN_LIB})
add_unit_test(test_assertion ${ZCUT_MAIN_LIB})
add_unit_test(test_no_test ${ZCUT_MAIN_LIB})

add_unit_test(test_link_zcut ${ZCUT_LIB})
add_unit_test(test_ut_init_no_called_error ${ZCUT_LIB})
