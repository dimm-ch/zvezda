#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(lmfc-adc)

set(PROJECT_COMPILE_FEATURES cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-deprecated-declarations)
endif()
set(PROJECT_COMPILE_DEFINES SPDDEVAPP)
set(PROJECT_LINK_LIBRARIES brdlib INIReader logger)

set(PROJECT_INCLUDE_DIRS)
set(PROJECT_INCLUDES)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)

if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd_full.cpp)
else()
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd.cpp)
endif()

include(executable)
