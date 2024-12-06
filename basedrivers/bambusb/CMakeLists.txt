#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(bambusb)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-format -Wno-c++11-narrowing -Wno-pragma-pack -Wno-sizeof-pointer-memaccess -Wno-int-to-void-pointer-cast )
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-constant-conversion -Wno-exceptions -Wno-delete-abstract-non-virtual-dtor)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-void-pointer-to-int-cast -Wno-unknown-warning-option)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-int-to-pointer-cast -Wno-overflow -Wno-pragmas -Wno-pointer-arith) # gcc
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-conversion-null -Wno-aggressive-loop-optimizations) #gcc
endif()
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES ${PROJECT_SOURCE_DIR}/src/bambusb.h)

set(PROJECT_SOURCES
    ${BARDY_ROOT}/brdlib/basedllini.cpp
    ${BARDY_ROOT}/brdlib/baseentry.cpp

    ${PROJECT_SOURCE_DIR}/src/bambusb.cpp
    ${PROJECT_SOURCE_DIR}/src/bambusbll.cpp
)
set(PROJECT_COMPILE_DEFINES _AMBUSB)

if(WIN32)
    set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src/windows)
    set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
        ${PROJECT_SOURCE_DIR}/src/windows/CyAPI.h
        ${PROJECT_SOURCE_DIR}/src/windows/cyioctl.h
        ${PROJECT_SOURCE_DIR}/src/windows/CyUSB30_def.h
        ${PROJECT_SOURCE_DIR}/src/windows/usb100.h
        ${PROJECT_SOURCE_DIR}/src/windows/usb200.h
        ${PROJECT_SOURCE_DIR}/src/windows/UsbdStatus.h
        ${PROJECT_SOURCE_DIR}/src/windows/VersionNo.h
    )
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} __CYPRESS_API)
    # set(PROJECT_SOURCES ${PROJECT_SOURCES} ${PROJECT_SOURCE_DIR}/src/windows/CyAPI.cpp)
    set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} ${PROJECT_SOURCE_DIR}/src/windows/CyAPIx64.lib setupapi)
else()
    set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src/linux)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm")
        set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} ${PROJECT_SOURCE_DIR}/src/linux/libusb-1.0.24.armhf.a)
    else()
        set(PROJECT_LINK_LIBRARIES ${PROJECT_LINK_LIBRARIES} usb-1.0)
    endif()
    set(PROJECT_SOURCES ${PROJECT_SOURCES} ${PROJECT_SOURCE_DIR}/src/linux/cyapi.cpp)
endif()

include(lib-shared)
