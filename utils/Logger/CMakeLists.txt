#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(logger)

set(PROJECT_COMPILE_FEATURES cxx_std_14)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-unused-parameter -Wno-unused-variable)
endif()
set(PROJECT_COMPILE_DEFINES)
set(PROJECT_LINK_LIBRARIES)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INCLUDES)
# set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/main.cpp)

include(lib-header)
