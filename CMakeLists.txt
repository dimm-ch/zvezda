#
# This file is part of InSys SDK
#
# Copyright (c) 1989-2021 Instrumental Systems, JSC (devlab@insys.ru)
#

cmake_minimum_required(VERSION 3.19)

project(insys-projects)

include(git-submodule.cmake)
add_git_submodule(cmake)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

option(BUILD_ALL "Building ALL." OFF)

option(TEST_ALL "Build all tests." OFF)

set(CPACK_ENABLE OFF)
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CPACK_ENABLE ON)

    set(BASE_ALL ON)
    set(SIDE_ALL OFF)
    set(EXAM_ADC ON)
    set(EXAM_EDAC ON)
    set(EXAM_SYNC ON)
    set(EXAM_FDDS ON)
endif()

add_subdirectory(core)
add_subdirectory(utils)
add_subdirectory(contribs)
add_subdirectory(basedrivers)
add_subdirectory(sidedrivers)
add_subdirectory(examples)
add_subdirectory(apps)
add_subdirectory(server)
add_subdirectory(work)
add_subdirectory(work2)
add_subdirectory(work2/temp)

message(STATUS ${Var1})
if(DEFINED WINCOMPILE)
  message("COMPILING FOR WINDOWS, SKIPPING ROOT COMPLEX")
else()
  message("compiling for linux")
  add_subdirectory(fmc146v.rc)
endif()

# Обойти все обработанные поддиректории, начиная с корня билда,
# и собрать все таргеты
function(get_all_targets out_var root_bin_dir)
    set(_queue "${root_bin_dir}")
    set(_seen "")
    set(_all "")

    while(_queue)
        list(POP_BACK _queue _dir)

        # пропускаем, если уже были тут
        list(FIND _seen "${_dir}" _idx)
        if(_idx GREATER -1)
            continue()
        endif()
        list(APPEND _seen "${_dir}")

        # таргеты в этой директории
        get_property(_tgts DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
        if(_tgts)
            list(APPEND _all ${_tgts})
        endif()

        # добавить обработанные поддиректории в очередь
        get_property(_subs DIRECTORY "${_dir}" PROPERTY SUBDIRECTORIES)
        if(_subs)
            list(APPEND _queue ${_subs})
        endif()
    endwhile()

    if(_all)
        list(REMOVE_DUPLICATES _all)
    endif()
    set(${out_var} "${_all}" PARENT_SCOPE)
endfunction()

if (${CPACK_ENABLE})
    get_all_targets(ALL_TARGETS "${CMAKE_BINARY_DIR}")

    # Specify some params for target OS
    if(WIN32)
        set(CPACK_GENERATOR "NSIS")
        set(CPACK_NSIS_INSTALL_ROOT "$Profile")
        set(CPACK_PACKAGE_INSTALL_DIRECTORY "insys\\bardy")
        set(CPACK_DESTINATION ".")
    else()
        set(CPACK_GENERATOR "DEB")
        set(CPACK_DESTINATION "/opt/insys/bardy")
    endif()

    # install targets
    foreach(TARGET_NAME IN LISTS ALL_TARGETS)
        install(
            TARGETS ${TARGET_NAME}
            DESTINATION ${CPACK_DESTINATION}
        )
    endforeach()

    get_version_from_git()

    # General info about all packages
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_PACKAGE_VENDOR "JSC InSys")
    set(CPACK_PACKAGE_CONTACT "devlab@insys.ru")
    set(CPACK_VERBATIM_VARIABLES YES)

    set(CPACK_OUTPUT_FILE_PREFIX "${CMAKE_SOURCE_DIR}/_distr")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

    # for DEB generator
    set(CPACK_DEB_COMPONENT_INSTALL YES)
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "JSC InSys")
    set(CPACK_DEBIAN_PACKAGE_SECTION "libs")
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)

    set(CPACK_DEBIAN_PACKAGE_DEPENDS "")
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "JSC InSys Projects distribution package")

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
        "${CMAKE_CURRENT_SOURCE_DIR}/scripts/prerm;${CMAKE_CURRENT_SOURCE_DIR}/scripts/postinst")

    # for NSIS generator
    set(CPACK_NSIS_PACKAGE_NAME "insys-projects")
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_IGNORE_LICENSE_PAGE ON)

    set(NSIS_INSTALL_FOLDER "${CPACK_NSIS_INSTALL_ROOT}\\${CPACK_PACKAGE_INSTALL_DIRECTORY}")
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        CreateShortcut \"$smprograms\\${CPACK_NSIS_PACKAGE_NAME}\\Insys Projects.lnk\" \"${NSIS_INSTALL_FOLDER}\"
        EnVar::SetHKCU
        EnVar::AddValue PATH \"${CPACK_NSIS_INSTALL_ROOT}\\${CPACK_PACKAGE_INSTALL_DIRECTORY}\"
    ")

    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \"$smprograms\\${CPACK_NSIS_PACKAGE_NAME}\\Insys Projects.lnk\"
        EnVar::SetHKCU
        EnVar::DeleteValue PATH \"${CPACK_NSIS_INSTALL_ROOT}\\${CPACK_PACKAGE_INSTALL_DIRECTORY}\"
    ")

    include(CPack)
endif()
