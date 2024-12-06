set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/ddssrv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/ddssrv/ad9956.h
    ${PROJECT_SOURCE_DIR}/ddssrv/ddsregs.h
    ${PROJECT_SOURCE_DIR}/ddssrv/ddssrv.h
    ${PROJECT_SOURCE_DIR}/ddssrv/ddssrvinfo.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/ddssrv/ddsctrl.cpp
    ${PROJECT_SOURCE_DIR}/ddssrv/ddssrv.cpp
)
