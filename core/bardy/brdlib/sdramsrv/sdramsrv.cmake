set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/sdramsrv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramambpcdsrv.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramambpcdsrvinfo.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramambpcxsrv.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramregs.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramsrv.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramsrvinfo.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/ddr3sdramsrv.h
    ${PROJECT_SOURCE_DIR}/sdramsrv/ddr4sdramsrv.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramambpcdsrv.cpp
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramambpcxsrv.cpp
    ${PROJECT_SOURCE_DIR}/sdramsrv/sdramsrv.cpp
    ${PROJECT_SOURCE_DIR}/sdramsrv/ddr3sdramsrv.cpp
    ${PROJECT_SOURCE_DIR}/sdramsrv/ddr4sdramsrv.cpp
)
