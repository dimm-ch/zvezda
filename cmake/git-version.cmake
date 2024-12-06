# -----------------------------------------------------------------------------
# Get Project Version from Git tag
# -----------------------------------------------------------------------------
macro(get_version_from_git)
    # по умолчанию версия develop
    set(PROJECT_VERSION_MAJOR 0)
    set(PROJECT_VERSION_MINOR 0)
    set(PROJECT_VERSION_PATCH 0)
    set(PROJECT_VERSION 0.0.0)

    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
        # определим версию по тегу, если он присутствует
        execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE ERROR OUTPUT_VARIABLE _HEAD_)
        if(NOT ERROR)
            string(REGEX REPLACE "[ \t\r\n]" "" _HEAD_ ${_HEAD_})
            execute_process(COMMAND ${GIT_EXECUTABLE} name-rev --tags --name-only ${_HEAD_} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE ERROR OUTPUT_VARIABLE _TAG_)
            if(NOT (ERROR OR _TAG_ STREQUAL "undefined" OR _TAG_ MATCHES "^Could not get"))
                if(_TAG_ MATCHES "([0-9]+)\\.([0-9]+)\\.?([0-9]*)")
                    if(CMAKE_MATCH_1 GREATER 0)
                        set(PROJECT_VERSION_MAJOR ${CMAKE_MATCH_1})
                    endif()
                    if(CMAKE_MATCH_2 GREATER 0)
                        set(PROJECT_VERSION_MINOR ${CMAKE_MATCH_2})
                    endif()
                    if(CMAKE_MATCH_3 GREATER 0)
                        set(PROJECT_VERSION_PATCH ${CMAKE_MATCH_3})
                    endif()
                endif()
            endif()
        endif()

        set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
        message(STATUS ">>> Set version ${PROJECT_VERSION} for \"${PROJECT_NAME}\"")
    endif()
endmacro()
