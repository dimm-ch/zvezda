# -----------------------------------------------------------------------------
# Make Executable Application
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    include(msvc)
endif()

if(NOT DEFINED PROJECT_COMPILE_FEATURES)
    set(PROJECT_COMPILE_FEATURES cxx_std_17)
endif()

if(WIN32 AND MINGW)
    target_link_options(${PROJECT_NAME} PUBLIC "-municode" )
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINES} UNICODE _UNICODE __USE_MINGW_ANSI_STDIO=0)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBRARIES} "-static-libgcc -Wl,-static,-lpthread")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIRS})

target_compile_features(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_FEATURES})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_DEFINES})
target_compile_options(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBRARIES})

if(UNIX)
    target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:-s>") # удаляем отладочную информацию
    set(CMAKE_SKIP_RPATH on) # отключим установку путей поиска Shared Library's
    target_link_options(${PROJECT_NAME} PUBLIC "-Wl,-rpath=.") # выставим путь поиска '.'
endif()

include(git-version)
get_version_from_git()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    EXEC_MAJOR_VERSION=${PROJECT_VERSION_MAJOR}
    EXEC_MINOR_VERSION=${PROJECT_VERSION_MINOR}
    EXEC_PATCH_VERSION=${PROJECT_VERSION_PATCH}
)
