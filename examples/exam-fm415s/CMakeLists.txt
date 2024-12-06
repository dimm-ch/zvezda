#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(exam-fm415s)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
set(PROJECT_COMPILE_OPTIONS)
set(PROJECT_LINK_LIBRARIES brdlib brd)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
	${PROJECT_SOURCE_DIR}/src/fm415s_test.h
	${PROJECT_SOURCE_DIR}/../../core/bardy/brdinc/ctrladmpro/ctrlfm415s.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/fm415s_test.cpp)
if(MSVC)
    set(PROJECT_COMPILE_DEFINES _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif(MSVC)

set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} ${Boost_INCLUDE_DIR})
set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} ${Boost_LIBRARIES})

include(executable)
