cmake_minimum_required(VERSION 3.19)
project(stick-test)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-parentheses -Wno-switch -Wno-format-security)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-writable-strings -Wno-format -Wno-int-to-void-pointer-cast)
endif()

set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/../common)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/../common/exceptinfo.h
    ${PROJECT_SOURCE_DIR}/../common/strconv.h
)

set(PROJECT_SOURCES
	${PROJECT_SOURCE_DIR}/../common/brd_dev.cpp
	${PROJECT_SOURCE_DIR}/../common/mapper.cpp
	${PROJECT_SOURCE_DIR}/../common/exceptinfo.cpp
	${PROJECT_SOURCE_DIR}/../common/strconv.cpp
	${PROJECT_SOURCE_DIR}/../common/table_engine_console.cpp
	${PROJECT_SOURCE_DIR}/src/fiddle_stick_test.cpp
)

if(MSVC)
	set(PROJECT_COMPILE_DEFINES _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif(MSVC)

include(executable)
