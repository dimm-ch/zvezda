#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

option(LIBS_COLLOG "Building CoLLoG static library." ON)
option(LIBS_CHAPPI "Building Chappi header library." ON)
option(LIBS_LOGGER "Building Logger header library." ON)

if(LIBS_COLLOG)
    add_git_submodule(CoLLoG)
    add_subdirectory(CoLLoG)
endif(LIBS_COLLOG)
if(LIBS_CHAPPI)
    add_git_submodule(ChappiLib)
    add_subdirectory(ChappiLib)
endif(LIBS_CHAPPI)
if(LIBS_LOGGER)
    add_git_submodule(Logger)
    add_subdirectory(Logger)
endif(LIBS_LOGGER)
