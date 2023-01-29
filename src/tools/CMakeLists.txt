add_executable(fastcws fastcws.cpp)
target_include_directories(fastcws PRIVATE ${FASTCWS_INCLUDE_DIRS})
target_link_libraries(fastcws PRIVATE fastcws_defaults_object)
if (WIN32)
	target_link_libraries(fastcws PRIVATE nowide)
endif()
target_link_options(fastcws PRIVATE ${FASTCWS_LINKER_FLAGS})
target_compile_options(fastcws PRIVATE ${FASTCWS_COMPILER_FLAGS})

add_executable(hmm_train hmm_train.cpp)
target_include_directories(hmm_train PRIVATE ${FASTCWS_INCLUDE_DIRS})
target_link_options(hmm_train PRIVATE ${FASTCWS_LINKER_FLAGS})
target_compile_options(hmm_train PRIVATE ${FASTCWS_COMPILER_FLAGS})

get_target_property(ZLIB_INCLUDE_DIRS zlibstatic INCLUDE_DIRECTORIES)

add_executable(snapshot_utils snapshot_utils.cpp)
target_include_directories(snapshot_utils PRIVATE ${FASTCWS_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
target_link_libraries(snapshot_utils PRIVATE zlibstatic)
target_link_options(snapshot_utils PRIVATE ${FASTCWS_LINKER_FLAGS})
target_compile_options(snapshot_utils PRIVATE ${FASTCWS_COMPILER_FLAGS})

install(TARGETS fastcws hmm_train)

