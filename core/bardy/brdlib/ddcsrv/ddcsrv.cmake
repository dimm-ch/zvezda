set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/ddcsrv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddcregs.h
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddcsrv.h
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddcsrvinfo.h
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddc4x16srv.h
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddc4x16srvinfo.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddcctrl.cpp
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddcsrv.cpp
    ${PROJECT_SOURCE_DIR}/ddcsrv/ddc4x16srv.cpp
)
