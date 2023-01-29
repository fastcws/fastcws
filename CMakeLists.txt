cmake_minimum_required(VERSION 3.13)

project(fastcws)

set(CMAKE_CXX_STANDARD 17)
set(FASTCWS_COMPILER_FLAGS "")
set(FASTCWS_LINKER_FLAGS "")

option(FASTCWS_STATIC_STL "Link STL statically. If off, you need to install STL runtime to use the compiled progs & dyn libs." ON)
if (FASTCWS_STATIC_STL)
	if (CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
		list(APPEND FASTCWS_LINKER_FLAGS -static-libgcc -static-libstdc++)
	elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
		set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(FASTCWS_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/external/boost_containers)
set(FASTCWS_ASSETS_DIR ${PROJECT_SOURCE_DIR}/assets)
set(FASTCWS_GENERATED_DIR ${CMAKE_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${FASTCWS_GENERATED_DIR})

set(SKIP_INSTALL_ALL OFF BOOL FORCE)
add_subdirectory(external/zlib)
unset(SKIP_INSTALL_ALL)

if (WIN32)
	add_subdirectory(external/nowide)
endif()

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(examples)

