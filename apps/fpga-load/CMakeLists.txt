#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(fpga-load)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-parentheses -Wno-switch -Wno-format-security)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-writable-strings -Wno-format -Wno-int-to-void-pointer-cast)
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/bardy.h
    ${PROJECT_SOURCE_DIR}/src/brd_dev.h
    ${PROJECT_SOURCE_DIR}/src/exceptinfo.h
    ${PROJECT_SOURCE_DIR}/src/strconv.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/brd_dev.cpp
    ${PROJECT_SOURCE_DIR}/src/exceptinfo.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
    ${PROJECT_SOURCE_DIR}/src/strconv.cpp
)
if(MSVC)
    set(PROJECT_COMPILE_DEFINES _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif(MSVC)

include(executable)
