#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(brd)

set(BARDY_ROOT ${PROJECT_SOURCE_DIR}  CACHE STRING "")

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-parentheses -Wno-switch -Wno-format-security)
endif()
set(PROJECT_LINK_LIBRARIES gipcy)

add_subdirectory(brdlib)
add_subdirectory(brdprx)

set(PROJECT_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/brdinc
    ${PROJECT_SOURCE_DIR}/brdshell
    ${PROJECT_SOURCE_DIR}/brdcpp
)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/brdinc/brd.h
    ${PROJECT_SOURCE_DIR}/brdinc/brdapi.h
    ${PROJECT_SOURCE_DIR}/brdinc/brderr.h
    ${PROJECT_SOURCE_DIR}/brdinc/brdpu.h
    ${PROJECT_SOURCE_DIR}/brdinc/ctrl.h
    ${PROJECT_SOURCE_DIR}/brdinc/ctrlstrm.h
    ${PROJECT_SOURCE_DIR}/brdinc/extn.h
    ${PROJECT_SOURCE_DIR}/brdinc/extn_andor.h
    ${PROJECT_SOURCE_DIR}/brdinc/realdelay.h
    ${PROJECT_SOURCE_DIR}/brdinc/utypes.h
    ${PROJECT_SOURCE_DIR}/brdinc/utypes_linux.h
    ${PROJECT_SOURCE_DIR}/brdshell/brdctrl.h
    ${PROJECT_SOURCE_DIR}/brdshell/brdi.h
    ${PROJECT_SOURCE_DIR}/brdshell/captor.h
    ${PROJECT_SOURCE_DIR}/brdshell/dbprintf.h
    ${PROJECT_SOURCE_DIR}/brdshell/idxbrd.h
    ${PROJECT_SOURCE_DIR}/brdshell/resource.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/brdshell/brd.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/brdinit.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/brdisup.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/brdpu.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/brdreinit.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/brdserv.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/captor.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/dbprintf.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/getregry.cpp
    ${PROJECT_SOURCE_DIR}/brdshell/idxbrd.cpp
)

include(brdcpp/brdcpp.cmake)
include(brdinc/ctrladmpro/ctrladmpro.cmake)
include(drvioctl/drvioctl.cmake)
include(icrinc/icrinc.cmake)

if(MSVC)
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS NOMINMAX)
endif()

include(lib-shared)
