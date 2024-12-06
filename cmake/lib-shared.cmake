# -----------------------------------------------------------------------------
# Make Shared Library
# -----------------------------------------------------------------------------
add_library(${PROJECT_NAME} SHARED)
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    include(msvc)
endif()

if(WIN32 AND MINGW)
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    target_link_options(${PROJECT_NAME} PUBLIC "-municode" )
    target_compile_definitions(${PROJECT_NAME} PRIVATE ${PROJECT_COMPILE_DEFINES} UNICODE _UNICODE __USE_MINGW_ANSI_STDIO=0)
    target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBRARIES} "-static-libgcc -Wl,-static,-lpthread")
endif()

if(NOT DEFINED PROJECT_COMPILE_FEATURES)
    set(PROJECT_COMPILE_FEATURES cxx_std_17)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE on) # -fPIC

target_include_directories(${PROJECT_NAME} PUBLIC "$<BUILD_INTERFACE:${PROJECT_INCLUDE_DIRS}>")

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
