set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/gc5016srv
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/gc5016srv/gc5016srv.h
    ${PROJECT_SOURCE_DIR}/gc5016srv/gc5016regs.h
)
set(PROJECT_SOURCES ${PROJECT_SOURCES}
    ${PROJECT_SOURCE_DIR}/gc5016srv/gc5016ctrl.cpp
    ${PROJECT_SOURCE_DIR}/gc5016srv/gc5016srv.cpp
)
