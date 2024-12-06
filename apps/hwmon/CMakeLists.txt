#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

option(BUILD_HWMON_HOST "Build hardware monitor for host." ON)

if(BUILD_HWMON_HOST)
    add_subdirectory(host)
endif()

option(BUILD_HWMON_TARGET "Build hardware monitor for remote target." ON)

if(BUILD_HWMON_TARGET AND WIN32)
    message(STATUS "[hwmon-target] Supported build only for linux/arm")
else()
    add_subdirectory(target)
endif()
