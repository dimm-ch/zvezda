#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(chappi)

set(PROJECT_COMPILE_OPTIONS)
set(PROJECT_COMPILE_DEFINES)
set(PROJECT_LINK_LIBRARIES)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/include/chappi.h
    ${PROJECT_SOURCE_DIR}/include/chappi_base.h
    ${PROJECT_SOURCE_DIR}/include/chappi_except.h
    ${PROJECT_SOURCE_DIR}/include/chappi_register.h
    ${PROJECT_SOURCE_DIR}/include/chappi_adn4600.h
    ${PROJECT_SOURCE_DIR}/include/chappi_hmc987.h
    ${PROJECT_SOURCE_DIR}/include/chappi_ina219.h
    ${PROJECT_SOURCE_DIR}/include/chappi_lmx2594.h
    ${PROJECT_SOURCE_DIR}/include/chappi_ltc2991.h
    ${PROJECT_SOURCE_DIR}/include/chappi_ltc6953.h
    ${PROJECT_SOURCE_DIR}/include/chappi_si57x.h
    ${PROJECT_SOURCE_DIR}/include/chappi_tca6424.h
    ${PROJECT_SOURCE_DIR}/include/chappi_ad5621.h
)

include(lib-header)
