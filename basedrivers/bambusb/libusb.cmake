# -----------------------------------------------------------------------------
# USB Library
# -----------------------------------------------------------------------------
set(LIB_NAME libusb)
set(LIBUSB_SRC_PATH ${PROJECT_SOURCE_DIR}/libusb/libusb)
set(LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/core.c ${LIBUSB_SRC_PATH}/descriptor.c ${LIBUSB_SRC_PATH}/hotplug.c)
list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/io.c ${LIBUSB_SRC_PATH}/strerror.c ${LIBUSB_SRC_PATH}/sync.c)
if(WIN32)
    list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/os/poll_windows.c)
    list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/os/threads_windows.c)
    list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/os/windows_nt_common.c ${LIBUSB_SRC_PATH}/os/windows_usbdk.c ${LIBUSB_SRC_PATH}/os/windows_winusb.c)
endif()
if(UNIX)
    list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/os/poll_posix.c)
    list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/os/threads_posix.c)
    list(APPEND LIBUSB_SOURCES ${LIBUSB_SRC_PATH}/os/linux_usbfs.c ${LIBUSB_SRC_PATH}/os/linux_netlink.c)
endif()

add_library(${LIB_NAME} STATIC ${LIBUSB_SOURCES})
set_target_properties(${LIB_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON) # -fPIC
target_include_directories(${LIB_NAME} PRIVATE ${LIBUSB_SRC_PATH} ${LIBUSB_SRC_PATH}/os)
target_include_directories(${LIB_NAME} INTERFACE $<BUILD_INTERFACE:${LIBUSB_SRC_PATH}>)

# удалим префикс 'lib', т.к. библиотека имеет имя 'libusb'
set_target_properties(${LIB_NAME} PROPERTIES PREFIX "")
set_target_properties(${LIB_NAME} PROPERTIES IMPORT_PREFIX "")

# -----------------------------------------------------------------------------
# Создание конфигурационного файла
# -----------------------------------------------------------------------------
if(WIN32)
    file(READ libusb/msvc/config.h CONFIG_WIN)
	string(REPLACE "#define ENABLE_LOGGING 1" "// #define ENABLE_LOGGING 1" CONFIG_WIN ${CONFIG_WIN})
	# target_compile_definitions(${LIB_NAME} PRIVATE _MSC_VER=1800) # Visual Studio 2013
	string(REPLACE "#ifndef _MSC_VER" "#ifdef _GCC_VER" CONFIG_WIN ${CONFIG_WIN})
    file(WRITE ${CMAKE_BINARY_DIR}/config.h ${CONFIG_WIN})
    target_include_directories(${LIB_NAME} PRIVATE ${CMAKE_BINARY_DIR})
endif()
if(UNIX)
	set(TARGET_NAME libusb_configure)
	set(TARGET_DIRECTORY ${PROJECT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir)
	add_custom_command(
		OUTPUT ${PROJECT_SOURCE_DIR}/libusb/configure
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/libusb
		COMMAND ./bootstrap.sh
	)
	add_custom_command(
		OUTPUT ${TARGET_DIRECTORY}/config.h
		WORKING_DIRECTORY ${TARGET_DIRECTORY}
		COMMAND ${PROJECT_SOURCE_DIR}/libusb/configure --disable-examples-build --disable-tests-build --disable-shared --enable-static --disable-debug-log --disable-system-log --disable-log --disable-udev
	)
	add_custom_target(libusb_configure DEPENDS ${PROJECT_SOURCE_DIR}/libusb/configure ${TARGET_DIRECTORY}/config.h)
	add_dependencies(${LIB_NAME} ${TARGET_NAME})
    target_include_directories(${LIB_NAME} PRIVATE ${TARGET_DIRECTORY})
	unset(TARGET_NAME)
	unset(TARGET_DIRECTORY)
endif()
