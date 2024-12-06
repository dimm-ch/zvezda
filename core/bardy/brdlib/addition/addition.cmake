set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/addition
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/addition/v6mmcm.h
    ${PROJECT_SOURCE_DIR}/addition/v7mmcm.h
    ${PROJECT_SOURCE_DIR}/addition/si571.h
    ${PROJECT_SOURCE_DIR}/addition/ad9512.h
    ${PROJECT_SOURCE_DIR}/addition/dac34sh84.h
    ${PROJECT_SOURCE_DIR}/addition/lmk61e2.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/addition/si571.cpp
    ${PROJECT_SOURCE_DIR}/addition/ad9512.cpp
    ${PROJECT_SOURCE_DIR}/addition/v6mmcm.cpp
    ${PROJECT_SOURCE_DIR}/addition/dac34sh84.cpp
    ${PROJECT_SOURCE_DIR}/addition/lmk61e2.cpp
)
