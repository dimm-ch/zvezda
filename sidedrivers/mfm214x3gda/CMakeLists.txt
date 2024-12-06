#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(mfm214x3gda)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format -Wno-macro-redefined -Wno-tautological-constant-out-of-range-compare)
endif()
set(PROJECT_LINK_LIBRARIES brdlib INIReader)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/AD9176.h
    ${PROJECT_SOURCE_DIR}/src/AD9208.h
    ${PROJECT_SOURCE_DIR}/src/LMX2594.h
    ${PROJECT_SOURCE_DIR}/src/log.h
    ${PROJECT_SOURCE_DIR}/src/mfm214x3gda.h
    ${PROJECT_SOURCE_DIR}/src/sidedriver.h
    ${PROJECT_SOURCE_DIR}/src/SrvADC.h
    ${PROJECT_SOURCE_DIR}/src/SrvDAC.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/AD9176.cpp
    ${PROJECT_SOURCE_DIR}/src/AD9208.cpp
    ${PROJECT_SOURCE_DIR}/src/LMX2594.cpp
    ${PROJECT_SOURCE_DIR}/src/sidedriver.cpp
    ${PROJECT_SOURCE_DIR}/src/sidedriver.SYNC.cpp
    ${PROJECT_SOURCE_DIR}/src/SrvADC.cpp
    ${PROJECT_SOURCE_DIR}/src/SrvADC.exam_adc.cpp
    ${PROJECT_SOURCE_DIR}/src/SrvDAC.cpp
    ${PROJECT_SOURCE_DIR}/src/SrvDAC.exam_edac.cpp

    ${BARDY_ROOT}/brdlib/sidedllini.cpp
    ${BARDY_ROOT}/brdlib/sideentry.cpp
)
set(PROJECT_COMPILE_DEFINES _SIDEINC)

# set(Boost_VERBOSE on)
find_package(Boost REQUIRED)
set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} Boost::headers)

include(lib-shared)
