#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

add_git_submodule(bardy)
add_subdirectory(bardy)
add_git_submodule(gipcy)
add_subdirectory(gipcy)
