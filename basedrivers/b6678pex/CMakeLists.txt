#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(b6678pex)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-pointer-bool-conversion)
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/b6678pex.h
    ${PROJECT_SOURCE_DIR}/src/ti6678_pcie.h
    ${PROJECT_SOURCE_DIR}/src/ddw6678pex.h
    ${PROJECT_SOURCE_DIR}/src/extn_6678.h
    ${PROJECT_SOURCE_DIR}/src/ti6678hw.h
    ${PROJECT_SOURCE_DIR}/src/pcieLocalReset_6678.h
    ${PROJECT_SOURCE_DIR}/src/icrWrite.h
    ${PROJECT_SOURCE_DIR}/src/icrCopy.h
    ${PROJECT_SOURCE_DIR}/src/i2cFlashWrite.h
    ${PROJECT_SOURCE_DIR}/src/i2cFlashRead.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/src/b6678pex.cpp
    ${PROJECT_SOURCE_DIR}/src/ti6678_pcie.cpp
    ${PROJECT_SOURCE_DIR}/src/entry.cpp
    ${PROJECT_SOURCE_DIR}/src/lowlev.cpp
)
set(PROJECT_COMPILE_DEFINES _6678PEX)

include(lib-shared)
