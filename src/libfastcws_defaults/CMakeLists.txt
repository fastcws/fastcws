
add_custom_command(
	OUTPUT ${FASTCWS_GENERATED_DIR}/hmm_model.snapshot
	COMMAND snapshot_utils save_hmm_model_snapshot ${FASTCWS_ASSETS_DIR}/wseg_model.hmm ${FASTCWS_GENERATED_DIR}/hmm_model.snapshot
	DEPENDS snapshot_utils ${FASTCWS_ASSETS_DIR}/wseg_model.hmm
	VERBATIM
)

add_custom_command(
	OUTPUT ${FASTCWS_GENERATED_DIR}/hmm_model_gen.hpp
	COMMAND snapshot_utils snapshot2hpp hmm_model ${FASTCWS_GENERATED_DIR}/hmm_model.snapshot ${FASTCWS_GENERATED_DIR}/hmm_model_gen.hpp
	DEPENDS snapshot_utils ${FASTCWS_GENERATED_DIR}/hmm_model.snapshot
	VERBATIM
)

add_custom_command(
	OUTPUT ${FASTCWS_GENERATED_DIR}/freq_dict.snapshot
	COMMAND snapshot_utils save_dict_snapshot ${FASTCWS_ASSETS_DIR}/freq_dict.txt ${FASTCWS_GENERATED_DIR}/freq_dict.snapshot
	DEPENDS snapshot_utils ${FASTCWS_ASSETS_DIR}/freq_dict.txt
	VERBATIM
)

add_custom_command(
	OUTPUT ${FASTCWS_GENERATED_DIR}/freq_dict_gen.hpp
	COMMAND snapshot_utils snapshot2hpp freq_dict ${FASTCWS_GENERATED_DIR}/freq_dict.snapshot ${FASTCWS_GENERATED_DIR}/freq_dict_gen.hpp
	DEPENDS snapshot_utils ${FASTCWS_GENERATED_DIR}/freq_dict.snapshot
	VERBATIM
)

add_custom_target(gen_headers
	DEPENDS ${FASTCWS_GENERATED_DIR}/hmm_model_gen.hpp ${FASTCWS_GENERATED_DIR}/freq_dict_gen.hpp)

set(LIBFASTCWS_DEFAULTS_SOURCES hmm_model.cpp freq_dict.cpp)

get_target_property(ZLIB_SOURCES zlibstatic SOURCES)
get_target_property(ZLIB_SOURCE_DIR zlibstatic SOURCE_DIR)
get_target_property(ZLIB_INCLUDE_DIRS zlibstatic INCLUDE_DIRECTORIES)
foreach(SRC ${ZLIB_SOURCES})
	if (IS_ABSOLUTE ${SRC})
		list(APPEND ZLIB_ABS_SOURCES ${SRC})
	else()
		get_filename_component(ABS_SRC ${ZLIB_SOURCE_DIR}/${SRC} ABSOLUTE)
		list(APPEND ZLIB_ABS_SOURCES ${ABS_SRC})
	endif()
endforeach()

add_library(fastcws_defaults_object OBJECT ${LIBFASTCWS_DEFAULTS_SOURCES} ${ZLIB_ABS_SOURCES})
target_include_directories(fastcws_defaults_object PUBLIC ${FASTCWS_INCLUDE_DIRS} ${FASTCWS_GENERATED_DIR} ${ZLIB_INCLUDE_DIRS})
add_dependencies(fastcws_defaults_object gen_headers)

add_library(fastcws_defaults STATIC)
target_link_libraries(fastcws_defaults PRIVATE fastcws_defaults_object)

