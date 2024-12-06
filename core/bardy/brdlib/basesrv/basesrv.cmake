set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/basesrv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/basesrv/baseserv.h
    ${PROJECT_SOURCE_DIR}/basesrv/datainsrv.h
    ${PROJECT_SOURCE_DIR}/basesrv/dataoutsrv.h
    ${PROJECT_SOURCE_DIR}/basesrv/pioregs.h
    ${PROJECT_SOURCE_DIR}/basesrv/piosrv.h
    ${PROJECT_SOURCE_DIR}/basesrv/basefsrv.h
    ${PROJECT_SOURCE_DIR}/basesrv/sysmonsrv.h
    ${PROJECT_SOURCE_DIR}/basesrv/testsrv.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/basesrv/baseserv.cpp
    ${PROJECT_SOURCE_DIR}/basesrv/sysmonsrv.cpp
    ${PROJECT_SOURCE_DIR}/basesrv/testsrv.cpp
    ${PROJECT_SOURCE_DIR}/basesrv/piosrv.cpp
    ${PROJECT_SOURCE_DIR}/basesrv/basefsrv.cpp
    ${PROJECT_SOURCE_DIR}/basesrv/datainsrv.cpp
    ${PROJECT_SOURCE_DIR}/basesrv/dataoutsrv.cpp
)
