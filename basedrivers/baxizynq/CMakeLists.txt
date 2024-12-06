#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(baxizynq)

if(NOT UNIX)
    message(FATAL_ERROR "[${PROJECT_NAME}] Support build only under linux")
endif()

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-mismatched-new-delete)
    set(PROJECT_COMPILE_OPTIONS  ${PROJECT_COMPILE_OPTIONS} -Wno-address-of-packed-member) # gcc
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS  ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/axi_switch.h
    ${PROJECT_SOURCE_DIR}/src/axizynqregs.h
    ${PROJECT_SOURCE_DIR}/src/baxizynq.h
    ${PROJECT_SOURCE_DIR}/src/blk_base.h
    ${PROJECT_SOURCE_DIR}/src/fid_blk.h
    ${PROJECT_SOURCE_DIR}/src/fmcx_blk.h
    ${PROJECT_SOURCE_DIR}/src/fmcx_dev.h
    ${PROJECT_SOURCE_DIR}/src/fmcx_dma.h
    ${PROJECT_SOURCE_DIR}/src/fmcx_trd.h
    ${PROJECT_SOURCE_DIR}/src/main_blk.h
    ${PROJECT_SOURCE_DIR}/src/main_trd.h
    ${PROJECT_SOURCE_DIR}/src/mapper.h
    ${PROJECT_SOURCE_DIR}/src/trd_base.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/axi_switch.cpp
    ${PROJECT_SOURCE_DIR}/src/baxizynq.cpp
    ${PROJECT_SOURCE_DIR}/src/baxizynqll.cpp
    ${PROJECT_SOURCE_DIR}/src/fid_blk.cpp
    ${PROJECT_SOURCE_DIR}/src/fmcx_blk.cpp
    ${PROJECT_SOURCE_DIR}/src/fmcx_dev.cpp
    ${PROJECT_SOURCE_DIR}/src/fmcx_dma.cpp
    ${PROJECT_SOURCE_DIR}/src/fmcx_trd.cpp
    ${PROJECT_SOURCE_DIR}/src/main_blk.cpp
    ${PROJECT_SOURCE_DIR}/src/main_trd.cpp
    ${PROJECT_SOURCE_DIR}/src/mapper.cpp
    ${PROJECT_SOURCE_DIR}/src/trd_base.cpp

    ${BARDY_ROOT}/brdlib/basedllini.cpp
    ${BARDY_ROOT}/brdlib/baseentry.cpp
)
set(PROJECT_COMPILE_DEFINES _AXIZYNQ)

include(lib-shared)
