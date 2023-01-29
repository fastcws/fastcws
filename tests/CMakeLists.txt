
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
add_subdirectory(../external/gtest ${CMAKE_CURRENT_BINARY_DIR}/gtest)

enable_testing()
include(GoogleTest)

function(add_test test_name)
	add_executable(${test_name} ${test_name}.cpp)
	target_include_directories(${test_name} PRIVATE ${FASTCWS_INCLUDE_DIRS})
	target_link_libraries(${test_name} GTest::gtest_main)
	target_link_options(${test_name} PRIVATE ${FASTCWS_LINKER_FLAGS})
	target_compile_options(${test_name} PRIVATE ${FASTCWS_COMPILER_FLAGS})

	gtest_discover_tests(${test_name})
endfunction()

add_test(test_aho_corasick)
add_test(test_dict)
add_test(test_suspendable_region)
add_test(test_word_dag)
add_test(test_string_view)
add_test(test_unique_ptr)

