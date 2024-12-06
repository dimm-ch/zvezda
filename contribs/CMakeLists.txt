#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

option(LIBS_INIREADER "Building INIReader static library." ON)

if(LIBS_INIREADER)
    add_git_submodule(INIReader)
    add_subdirectory(INIReader)
endif(LIBS_INIREADER)
