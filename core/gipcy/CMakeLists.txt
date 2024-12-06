#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(gipcy)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-parentheses -Wno-switch -Wno-format-security)
endif()
set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/debugipc.h
    ${PROJECT_SOURCE_DIR}/include/devipc.h
    ${PROJECT_SOURCE_DIR}/include/diripc.h
    ${PROJECT_SOURCE_DIR}/include/dllipc.h
    ${PROJECT_SOURCE_DIR}/include/eventipc.h
    ${PROJECT_SOURCE_DIR}/include/fileipc.h
    ${PROJECT_SOURCE_DIR}/include/gipcy.h
    ${PROJECT_SOURCE_DIR}/include/ipcerr.h
    ${PROJECT_SOURCE_DIR}/include/ipclib.h
    ${PROJECT_SOURCE_DIR}/include/ipctype.h
    ${PROJECT_SOURCE_DIR}/include/mutexipc.h
    ${PROJECT_SOURCE_DIR}/include/semipc.h
    ${PROJECT_SOURCE_DIR}/include/shmipc.h
    ${PROJECT_SOURCE_DIR}/include/sockipc.h
    ${PROJECT_SOURCE_DIR}/include/stripc.h
    ${PROJECT_SOURCE_DIR}/include/sysipc.h
    ${PROJECT_SOURCE_DIR}/include/threadipc.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/src/shared/debugipc.cpp)
set(PROJECT_COMPILE_DEFINES _INSYS_IPC_)

if(WIN32)
    set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/src/windows)
    set(PROJECT_INCLUDES ${PROJECT_INCLUDES} ${PROJECT_SOURCE_DIR}/src/windows/winipc.h)
    set(PROJECT_SOURCES ${PROJECT_SOURCES}
        ${PROJECT_SOURCE_DIR}/src/windows/devipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/diripc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/dllipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/eventipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/fileipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/mutexipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/semipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/shmipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/sockipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/sysipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/threadipc.cpp
        ${PROJECT_SOURCE_DIR}/src/windows/winipc.cpp
    )
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} __IPC_WIN__)
    set(PROJECT_LINK_LIBRARIES ws2_32)
else() # UNIX
    set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/
        ${PROJECT_SOURCE_DIR}/src/linux/ipcdrv/
    )
    set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/linipc.h
        ${PROJECT_SOURCE_DIR}/src/linux/ipcdrv/ipcioctl.h
    )
    set(PROJECT_SOURCES ${PROJECT_SOURCES}
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/devipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/diripc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/dllipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/fileipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/sockipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/stripc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/sysipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/threadipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/insys/eventipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/insys/linipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/insys/mutexipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/insys/semipc.cpp
        ${PROJECT_SOURCE_DIR}/src/linux/libsrc/insys/shmipc.cpp
    )
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} __IPC_LINUX__)
    set(PROJECT_LINK_LIBRARIES pthread dl rt)
endif()

if(WIN32)
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} _USRDLL _WINDOWS WIN64 _WIN64 UNICODE _UNICODE)
elseif() # UNIX
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} __LINUX__ __linux__)
endif()

if(MSVC)
    set(PROJECT_COMPILE_DEFINES ${PROJECT_COMPILE_DEFINES} _CRT_NONSTDC_NO_WARNINGS _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS NOMINMAX)
endif()

include(lib-shared)
