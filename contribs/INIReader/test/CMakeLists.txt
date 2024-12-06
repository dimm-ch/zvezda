# -----------------------------------------------------------------------------
# Project
# -----------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.10)

if(MSVC AND POLICY CMP0091)
	cmake_minimum_required(VERSION 3.14)
	cmake_policy(SET CMP0091 NEW)
endif()

project(test LANGUAGES CXX)
# set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Headers
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})
set(SOURCES ${SOURCE_DIR}/test.cpp)

# -----------------------------------------------------------------------------
# Defines
# -----------------------------------------------------------------------------
list (APPEND DEFINES INI_PATH="${CMAKE_CURRENT_LIST_DIR}/test.ini")

# -----------------------------------------------------------------------------
# Flags
# -----------------------------------------------------------------------------
if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
	set(COMPILE_FLAGS "-Wno-writable-strings -Wno-format -Wno-int-to-pointer-cast -Wno-write-strings")
	if(WIN32)
		string(APPEND LINK_FLAGS " -static-libgcc -Wl,-static,-lpthread")
	endif()
endif()

# -----------------------------------------------------------------------------
# Execute
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON) # -fPIC
set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH ".")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "${DEFINES}")
set_target_properties(${PROJECT_NAME} PROPERTIES INCLUDE_DIRECTORIES "${HEADER_DIR}")
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${COMPILE_FLAGS}")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "${LINK_FLAGS}")

# -----------------------------------------------------------------------------
# Library's
# -----------------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} INIReader)
