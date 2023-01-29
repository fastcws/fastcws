
add_executable(word_break word_break.c)
target_include_directories(word_break PRIVATE ${PROJECT_SOURCE_DIR}/src/lib)
target_link_libraries(word_break PRIVATE libfastcws)

add_executable(custom_dict_and_model custom_dict_and_model.c)
target_include_directories(custom_dict_and_model PRIVATE ${PROJECT_SOURCE_DIR}/src/lib)
target_link_libraries(custom_dict_and_model PRIVATE libfastcws)

