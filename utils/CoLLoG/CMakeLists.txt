#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(collog)

set(PROJECT_COMPILE_OPTIONS)
set(PROJECT_COMPILE_DEFINES)
set(PROJECT_LINK_LIBRARIES)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/include/collog.h)

include(lib-header)

add_subdirectory(CoNCoL)

option(TEST_COLLOG "Build CoLLoG test." OFF)

if(TEST_ALL OR TEST_COLLOG)
    add_subdirectory(test)
endif()
