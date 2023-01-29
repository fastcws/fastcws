
add_library(libfastcws SHARED libfastcws.cpp)
target_include_directories(libfastcws PUBLIC ${FASTCWS_INCLUDE_DIRS})
target_link_libraries(libfastcws PRIVATE fastcws_defaults_object)
target_link_options(libfastcws PRIVATE ${FASTCWS_LINKER_FLAGS})
target_compile_options(libfastcws PRIVATE ${FASTCWS_COMPILER_FLAGS})
set_target_properties(libfastcws PROPERTIES PUBLIC_HEADER "libfastcws.h" OUTPUT_NAME fastcws)

install(TARGETS libfastcws PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

