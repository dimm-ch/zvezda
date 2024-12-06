#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(techlib)
set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-delete-abstract-non-virtual-dtor)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-write-strings -Wno-int-to-pointer-cast) # gcc
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(PROJECT_COMPILE_OPTIONS /Zc:strictStrings-)
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/src
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/src/tech.h
    ${PROJECT_SOURCE_DIR}/src/str.h
    ${PROJECT_SOURCE_DIR}/src/io.h
    ${PROJECT_SOURCE_DIR}/src/dict.h
    ${PROJECT_SOURCE_DIR}/src/array.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/src/tech.cpp
    ${PROJECT_SOURCE_DIR}/src/proxy.cpp
    ${PROJECT_SOURCE_DIR}/src/io.cpp
    ${PROJECT_SOURCE_DIR}/src/dict.cpp
)

include(lib-static)
