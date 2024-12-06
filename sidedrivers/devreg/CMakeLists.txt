#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(devreg)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
set(PROJECT_COMPILE_OPTIONS)
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src/devreg.h)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/devreg.cpp

    ${BARDY_ROOT}/brdlib/sidedllini.cpp
    ${BARDY_ROOT}/brdlib/sideentry.cpp
)
set(PROJECT_COMPILE_DEFINES _DEVREG)

include(lib-shared)
