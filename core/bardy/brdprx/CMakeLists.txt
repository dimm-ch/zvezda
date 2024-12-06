#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

option(LIBS_BRDPRX "Building Bardy Proxy static libraries." ON)

if(LIBS_BRDPRX)
    add_subdirectory(ctplib)
    add_subdirectory(techlib)
endif(LIBS_BRDPRX)
