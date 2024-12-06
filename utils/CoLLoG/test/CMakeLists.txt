#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(test_collog)

set(PROJECT_COMPILE_FEATURES cxx_std_17)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format-security)
endif()
set(PROJECT_LINK_LIBRARIES concol)

set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/test_collog.cpp)

include(executable)
