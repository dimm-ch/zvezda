#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(INIReader)

set(PROJECT_COMPILE_FEATURES cxx_std_11)

set(PROJECT_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include/
)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/ini.h
    ${PROJECT_SOURCE_DIR}/include/INIReader.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/ini.c
    ${PROJECT_SOURCE_DIR}/src/INIReader.cpp
)

include(lib-static)
