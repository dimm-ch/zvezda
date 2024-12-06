#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(bambpex)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS )
    set(PROJECT_COMPILE_OPTIONS  ${PROJECT_COMPILE_OPTIONS} -Wno-address-of-packed-member)
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/resource.h
    ${PROJECT_SOURCE_DIR}/src/bambpex.h

    ${BARDY_ROOT}/drvioctl/ambpex/ddwambpex.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/bambpex.cpp
    ${PROJECT_SOURCE_DIR}/src/bambpexll.cpp
    ${PROJECT_SOURCE_DIR}/src/basedllini.cpp
    ${PROJECT_SOURCE_DIR}/src/baseentry.cpp
)
set(PROJECT_COMPILE_DEFINES _AMBPEX)

include(lib-shared)
