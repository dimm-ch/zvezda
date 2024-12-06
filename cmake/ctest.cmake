# -----------------------------------------------------------------------------
# Make CTest Application
# -----------------------------------------------------------------------------
add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/test)

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    include(msvc)
endif()

if(NOT DEFINED PROJECT_COMPILE_FEATURES)
    set(PROJECT_COMPILE_FEATURES cxx_std_17)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE_DIRS})

target_compile_features(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_FEATURES})
target_compile_definitions(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_DEFINES})
target_compile_options(${PROJECT_NAME} PUBLIC ${PROJECT_COMPILE_OPTIONS})

target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBRARIES})

if(UNIX)
    target_link_options(${PROJECT_NAME} PUBLIC "$<$<CONFIG:Release>:-s>")
    set(CMAKE_SKIP_RPATH off)
    target_link_options(${PROJECT_NAME} PUBLIC "-Wl,-rpath=./../bin")
endif()

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} )

include(git-version)
get_version_from_git()
