cmake_minimum_required(VERSION 3.17)

project(icr_serial_writer LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(C_EXTENSIONS FALSE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_FLAGS "-Wall -Wextra -Wpedantic -pedantic-errors ")

set(PROJECT_LINK_LIBRARIES brdlib )

set(PROJECT_SOURCES
 	${PROJECT_SOURCE_DIR}/src/main.c)

if(MSVC)
    set(PROJECT_COMPILE_DEFINES _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif(MSVC)

include(executable)
