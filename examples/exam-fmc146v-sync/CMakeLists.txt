#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(exam-fmc146v-sync)

if(UNIX)
    set(PROJECT_COMPILE_FEATURES cxx_std_17)
else()
    set(PROJECT_COMPILE_FEATURES cxx_std_20)
endif()
set(PROJECT_COMPILE_OPTIONS)
set(PROJECT_LINK_LIBRARIES brdlib INIReader concol chappi)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src/fmc146v_sync.h)
set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/exam_fmc146v_sync.cpp)
if(MSVC)
    set(PROJECT_COMPILE_DEFINES _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif()

set(Boost_DEBUG OFF)
find_package(Boost REQUIRED COMPONENTS program_options)
set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} ${Boost_LIBRARIES})

include(executable)
