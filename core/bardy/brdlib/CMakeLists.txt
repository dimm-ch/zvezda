#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)
project(brdlib)
set(PROJECT_COMPILE_FEATURES cxx_std_11)
set(PROJECT_LINK_LIBRARIES brd)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR})
set(PROJECT_INCLUDES
    ${PROJECT_SOURCE_DIR}/ambinfo.h
    ${PROJECT_SOURCE_DIR}/basedll.h
    ${PROJECT_SOURCE_DIR}/elapsed_time.h
    ${PROJECT_SOURCE_DIR}/mainregs.h
    ${PROJECT_SOURCE_DIR}/pld.h
    ${PROJECT_SOURCE_DIR}/reg_rw_spd.h
    ${PROJECT_SOURCE_DIR}/sidedll.h
    ${PROJECT_SOURCE_DIR}/useful.h
    ${PROJECT_SOURCE_DIR}/service.h
    ${PROJECT_SOURCE_DIR}/streamll.h
    ${PROJECT_SOURCE_DIR}/basestream.h
    ${PROJECT_SOURCE_DIR}/module.h
    ${PROJECT_SOURCE_DIR}/adcdacsubmodule.h
    ${PROJECT_SOURCE_DIR}/dacsubmodule.h
    ${PROJECT_SOURCE_DIR}/adcsubmodule.h
    ${PROJECT_SOURCE_DIR}/basemodule.h
    ${PROJECT_SOURCE_DIR}/submodule.h
    ${PROJECT_SOURCE_DIR}/adm2if.h
    ${PROJECT_SOURCE_DIR}/adm2ifcfg.h
    ${PROJECT_SOURCE_DIR}/regsrv.h
    ${PROJECT_SOURCE_DIR}/streamsrv.h
    ${PROJECT_SOURCE_DIR}/rw_i2c.h
)
set(PROJECT_SOURCES
    ${PROJECT_SOURCE_DIR}/rw_i2c.cpp
    ${PROJECT_SOURCE_DIR}/useful.cpp
    ${PROJECT_SOURCE_DIR}/service.cpp
    ${PROJECT_SOURCE_DIR}/basestream.cpp
    ${PROJECT_SOURCE_DIR}/module.cpp
    ${PROJECT_SOURCE_DIR}/submodule.cpp
    ${PROJECT_SOURCE_DIR}/basemodule.cpp
    ${PROJECT_SOURCE_DIR}/dacsubmodule.cpp
    ${PROJECT_SOURCE_DIR}/adcdacsubmodule.cpp
    ${PROJECT_SOURCE_DIR}/adcsubmodule.cpp
    ${PROJECT_SOURCE_DIR}/adm2if.cpp
    ${PROJECT_SOURCE_DIR}/regsrv.cpp
    ${PROJECT_SOURCE_DIR}/streamsrv.cpp
)
include(adcsrv/adcsrv.cmake)
include(addition/addition.cmake)
include(basesrv/basesrv.cmake)
include(dacsrv/dacsrv.cmake)
include(ddcsrv/ddcsrv.cmake)
include(ddssrv/ddssrv.cmake)
include(dio32srv/dio32srv.cmake)
include(sdramsrv/sdramsrv.cmake)
include(syncsrv/syncsrv.cmake)
include(gc5016srv/gc5016srv.cmake)

include(lib-static)
