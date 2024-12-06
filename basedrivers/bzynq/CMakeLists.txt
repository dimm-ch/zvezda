#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(bzynq)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-mismatched-new-delete)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-address-of-packed-member) # gcc
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src/bzynq.h)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/bzynq.cpp
    ${PROJECT_SOURCE_DIR}/src/bzynqll.cpp

    ${BARDY_ROOT}/brdlib/basedllini.cpp
    ${BARDY_ROOT}/brdlib/baseentry.cpp
)
set(PROJECT_COMPILE_DEFINES _ZYNQ)

include(lib-shared)
