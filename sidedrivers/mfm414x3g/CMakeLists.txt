#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(mfm414x3g)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format -Wno-deprecated-declarations)
endif()
set(PROJECT_LINK_LIBRARIES brdlib INIReader)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/AD9208.h
    ${PROJECT_SOURCE_DIR}/src/LMX2594.h
    ${PROJECT_SOURCE_DIR}/src/log.h
    ${PROJECT_SOURCE_DIR}/src/mfm414x3g.h
    ${PROJECT_SOURCE_DIR}/src/mfm414x3g_brdsafe.h
    ${PROJECT_SOURCE_DIR}/src/sidedriver.h
    ${PROJECT_SOURCE_DIR}/src/SrvADC.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/AD9208.cpp
    ${PROJECT_SOURCE_DIR}/src/LMX2594.cpp
    ${PROJECT_SOURCE_DIR}/src/LTC2991.cpp
    ${PROJECT_SOURCE_DIR}/src/sidedriver.cpp
    ${PROJECT_SOURCE_DIR}/src/sidedriver.SYNC.cpp
    ${PROJECT_SOURCE_DIR}/src/SrvADC.cpp
    ${PROJECT_SOURCE_DIR}/src/SrvADC.exam_adc.cpp

    ${BARDY_ROOT}/brdlib/sidedllini.cpp
    ${BARDY_ROOT}/brdlib/sideentry.cpp
)
set(PROJECT_COMPILE_DEFINES _SIDEINC)

include(lib-shared)
