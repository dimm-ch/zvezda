cmake_minimum_required(VERSION 3.19)
project(extsrv)

set(PROJECT_COMPILE_FEATURES cxx_std_14)

set(PROJECT_LINK_LIBRARIES brdlib INIReader concol chappi)
set(PROJECT_COMPILE_DEFINES SPDDEVAPP)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|GNU)$")
    set(PROJECT_COMPILE_OPTIONS -Wno-writable-strings -Wno-format -Wno-int-to-void-pointer-cast)
    set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS} -Wno-int-to-pointer-cast -Wno-write-strings) # gcc
endif()

#add_definitions(-DCHAPPI_LOG_ENABLE)

set(PROJECT_INCLUDE_DIRS 	${PROJECT_SOURCE_DIR}/
				${PROJECT_SOURCE_DIR}/../contribs/INIReader/include
				${PROJECT_SOURCE_DIR}/dac/
				${PROJECT_SOURCE_DIR}/sync/
				${PROJECT_SOURCE_DIR}/adc/
				${PROJECT_SOURCE_DIR}/../work2/common/
				)

set(PROJECT_SOURCES ${PROJECT_SOURCE_DIR}/server.cpp
					${PROJECT_SOURCE_DIR}/total.cpp 
					${PROJECT_SOURCE_DIR}/../work2/common/dev_util.cpp
					${PROJECT_SOURCE_DIR}/dac/exam_edac.cpp
					${PROJECT_SOURCE_DIR}/dac/workmode.cpp
					${PROJECT_SOURCE_DIR}/sync/exam_fmc146v_sync.cpp
					${PROJECT_SOURCE_DIR}/adc/adc_ctrl.cpp
					${PROJECT_SOURCE_DIR}/adc/exam_adc.cpp
					${PROJECT_SOURCE_DIR}/adc/cont_ctrl.cpp
					${PROJECT_SOURCE_DIR}/adc/mem_ctrl.cpp
					${BARDY_ROOT}/brdlib/reg_rw_spd.cpp
					)

include(executable)
