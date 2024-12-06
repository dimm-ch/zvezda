#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(buni)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-write-strings -Wno-delete-abstract-non-virtual-dtor -Wno-deprecated-declarations)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(PROJECT_COMPILE_OPTIONS /Zc:strictStrings-)
endif()
set(PROJECT_LINK_LIBRARIES techlib ctplib)

set(PROJECT_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/src
)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/buni.h
    ${PROJECT_SOURCE_DIR}/src/basedriver.h
    ${PROJECT_SOURCE_DIR}/src/servstrm.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/buni.cpp
    ${PROJECT_SOURCE_DIR}/src/basedriver.cpp
    ${PROJECT_SOURCE_DIR}/src/bentry.cpp
    ${PROJECT_SOURCE_DIR}/src/servstrm.cpp
)

include(lib-shared)
