set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
set(ARCH arm32)

#set path to arm compiler


get_filename_component(ABS_PATH_COMPILER
	"${CMAKE_CURRENT_LIST_DIR}/../../arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin"
	ABSOLUTE)

message( "Current dir: ${ABS_PATH_COMPILER}" )

#set(compiler_path /home/kero/INSYS/SYSTEMZ/FMC141/rootfs.FMC141v.rc/_build_arm32)

set(ROOTFS_ROOT_DIR ${ABS_PATH_COMPILER})



get_filename_component(ROOTFS_ROOT_DIR
	"${CMAKE_CURRENT_LIST_DIR}/../../sysroot"
	ABSOLUTE)

set(CMAKE_SYSROOT ${ROOTFS_ROOT_DIR})


set(CMAKE_C_COMPILER ${ABS_PATH_COMPILER}/arm-none-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER ${ABS_PATH_COMPILER}/arm-none-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
