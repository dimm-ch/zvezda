#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

add_subdirectory(isvi-server)
#add_subdirectory(exam-base)
add_subdirectory(xtest)
add_subdirectory(qsfp-test)
add_subdirectory(fpga-load)
add_subdirectory(flash-write)
add_subdirectory(ddr-test)
add_subdirectory(aurora-test)
add_subdirectory(monitor-rc)
add_subdirectory(lvds_test)
add_subdirectory(icr_serial_fix)
add_subdirectory(gpio_test)


