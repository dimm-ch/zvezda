# -----------------------------------------------------------------------------
# Make Header-Only Library
# -----------------------------------------------------------------------------
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_INCLUDE_DIRS})
target_compile_definitions(${PROJECT_NAME} INTERFACE ${PROJECT_COMPILE_DEFINES})

include(git-version)
get_version_from_git()
