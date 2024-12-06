#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(isagent)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format-extra-args -Wno-unused-result -Wno-mismatched-new-delete)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} )
endif()
set(PROJECT_LINK_LIBRARIES ctplib techlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/_brd.h
    ${PROJECT_SOURCE_DIR}/src/host.h
    ${PROJECT_SOURCE_DIR}/src/winnet.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/_brd.cpp
    ${PROJECT_SOURCE_DIR}/src/brdhost.cpp
    ${PROJECT_SOURCE_DIR}/src/client.cpp

    ${PROJECT_SOURCE_DIR}/src/winnet.cpp
    ${PROJECT_SOURCE_DIR}/src/main.cpp
)

include(executable)
