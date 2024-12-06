if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    return()
endif()

set(PROJECT_COMPILE_FEATURES cxx_std_20)
set(PROJECT_COMPILE_OPTIONS ${PROJECT_COMPILE_OPTIONS}
    /Zc:__cplusplus
    $<$<CONFIG:Debug>:/MTd /Od>
    $<$<CONFIG:Release>:/MT>
    $<$<CONFIG:RelWithDebInfo>:/MTd /Od>
    $<$<CONFIG:MinSizeRel>:/MT>
)
