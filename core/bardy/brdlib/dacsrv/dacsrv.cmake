set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/dacsrv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/dacsrv/dacregs.h
    ${PROJECT_SOURCE_DIR}/dacsrv/dacsrv.h
    ${PROJECT_SOURCE_DIR}/dacsrv/dacsrvinfo.h
    ${PROJECT_SOURCE_DIR}/dacsrv/dac1624x192regs.h
    ${PROJECT_SOURCE_DIR}/dacsrv/dac1624x192srv.h
    ${PROJECT_SOURCE_DIR}/dacsrv/dac1624x192srvinfo.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/dacsrv/dacctrl.cpp
    ${PROJECT_SOURCE_DIR}/dacsrv/dacsrv.cpp
    ${PROJECT_SOURCE_DIR}/dacsrv/dac1624x192srv.cpp
)
