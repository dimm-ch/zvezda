
cmake_minimum_required(VERSION 3.19)
project(regio)

set(PROJECT_COMPILE_FEATURES cxx_std_11)
if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-writable-strings -Wno-format -Wno-int-to-void-pointer-cast)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-int-to-pointer-cast -Wno-write-strings) # gcc
endif()

set(PROJECT_COMPILE_DEFINES SPDDEVAPP)
set(PROJECT_LINK_LIBRARIES brd)

set(PROJECT_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/src)

#set(PROJECT_INCLUDES
#    ${PROJECT_SOURCE_DIR}/../common/dev_util.h
#)
 
 include_directories(../common/dev_util.h)
 
 set(PROJECT_SOURCES
     ${PROJECT_SOURCE_DIR}/src/main.cpp
     ${PROJECT_SOURCE_DIR}/../common/dev_util.cpp
 )

set(PROJECT_SOURCES ${PROJECT_SOURCES} ${BARDY_ROOT}/brdlib/reg_rw_spd.cpp)


include(executable)
