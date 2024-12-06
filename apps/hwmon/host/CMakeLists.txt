#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(hwmon-host)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
set(PROJECT_LINK_LIBRARIES brdlib)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/src/pcie_lib.h
    ${PROJECT_SOURCE_DIR}/src/strconv.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/src/pcie_lib.cpp
    ${PROJECT_SOURCE_DIR}/src/hwmon_host.cpp
)

include(executable)
