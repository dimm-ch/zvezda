#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(hwmon-target)

if(NOT UNIX)
    message(FATAL_ERROR "[${PROJECT_NAME}] Support build only under linux")
endif()

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    #set(PROJECT_COMPILE_OPTIONS "-lpthread")
endif()

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/strconv.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/hwmon.cpp
)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
set(PROJECT_LINK_LIBRARIES Threads::Threads)

include(executable)
