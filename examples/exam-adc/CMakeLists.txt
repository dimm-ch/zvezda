#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(exam-adc)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-writable-strings -Wno-format -Wno-int-to-void-pointer-cast)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-int-to-pointer-cast -Wno-write-strings) # gcc
endif()
set(PROJECT_COMPILE_DEFINES SPDDEVAPP)
set(PROJECT_LINK_LIBRARIES brd)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/adc_ctrl.h
    ${PROJECT_SOURCE_DIR}/src/mu_ctrl.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/adc_ctrl.cpp
    ${PROJECT_SOURCE_DIR}/src/cont_ctrl.cpp
    ${PROJECT_SOURCE_DIR}/src/exam_adc.cpp
    ${PROJECT_SOURCE_DIR}/src/mem_ctrl.cpp
)

if(WIN32)
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd_full.cpp)
else()
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd.cpp)
endif()

include(executable)
