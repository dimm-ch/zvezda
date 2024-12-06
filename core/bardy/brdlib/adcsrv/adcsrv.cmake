set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/adcsrv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/adcsrv/adcregs.h
    ${PROJECT_SOURCE_DIR}/adcsrv/adcsrv.h
    ${PROJECT_SOURCE_DIR}/adcsrv/adcsrvinfo.h
    ${PROJECT_SOURCE_DIR}/adcsrv/adctstsrv.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/adcsrv/adcctrl.cpp
    ${PROJECT_SOURCE_DIR}/adcsrv/adcsrv.cpp
    ${PROJECT_SOURCE_DIR}/adcsrv/adctstsrv.cpp
)
