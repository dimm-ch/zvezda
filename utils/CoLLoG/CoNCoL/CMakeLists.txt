#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(concol)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format-security)
endif()
set(PROJECT_COMPILE_DEFINES)
set(PROJECT_LINK_LIBRARIES)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/include/concol.h)
set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/concol.cpp)

include(lib-static)

option(TEST_CONCOL "Build CoNCoL test." OFF)

if(TEST_ALL OR TEST_CONCOL)
    add_subdirectory(test)
endif()
