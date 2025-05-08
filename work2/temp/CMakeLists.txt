#
# Мой тест
#

cmake_minimum_required(VERSION 3.19)
project(temp)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-writable-strings -Wno-format -Wno-int-to-void-pointer-cast)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-int-to-pointer-cast -Wno-write-strings) # gcc
endif()
set(PROJECT_COMPILE_DEFINES SPDDEVAPP)

set(PROJECT_SOURCES
     ${PROJECT_SOURCE_DIR}/main.cpp
 )

set(PROJECT_SOURCES ${PROJECT_SOURCES})


include(executable)
