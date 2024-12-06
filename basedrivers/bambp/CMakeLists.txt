#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(bambp)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format)
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/ddwambp.h
    ${PROJECT_SOURCE_DIR}/src/bambpci.h
    ${PROJECT_SOURCE_DIR}/src/bambpini.h
    ${PROJECT_SOURCE_DIR}/src/bambpMsg.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/bambpci.cpp
    ${PROJECT_SOURCE_DIR}/src/bambpll.cpp
    ${PROJECT_SOURCE_DIR}/src/basedllini.cpp
    ${PROJECT_SOURCE_DIR}/src/baseentry.cpp
)
set(PROJECT_COMPILE_DEFINES _AMBPCX)

include(lib-shared)
