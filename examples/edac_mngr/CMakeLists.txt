#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(edac_mngr)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format)
endif()
set(PROJECT_COMPILE_DEFINES SPDDEVAPP)
set(PROJECT_LINK_LIBRARIES brd brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src/exam_edac.h)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/exam_edac.cpp
    ${PROJECT_SOURCE_DIR}/src/workmode.cpp
)

if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd_full.cpp)
else()
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd.cpp)
endif()

include(executable)
