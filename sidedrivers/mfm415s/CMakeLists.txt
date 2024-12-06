#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(mfm415s)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-deprecated-declarations)
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/fm415sRegs.h
    ${PROJECT_SOURCE_DIR}/src/fm415sSrv.h
    ${PROJECT_SOURCE_DIR}/src/mfm415s.h
    ${PROJECT_SOURCE_DIR}/src/sidedriver.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/fm415sSrv.cpp
    ${PROJECT_SOURCE_DIR}/src/mfm415s.cpp

    ${BARDY_ROOT}/brdlib/sidedllini.cpp
    ${BARDY_ROOT}/brdlib/sideentry.cpp
)
set(PROJECT_COMPILE_DEFINES _SIDEINC)

include(lib-shared)
