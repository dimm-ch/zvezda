# Apply only to extsrv and ADC/DAC/SYNC managers, not to legacy library logging.
set(LOG_LEVEL "2" CACHE STRING "New shutdown logs: 0=off, 1=basic, 2=detailed")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS 0 1 2)
if(NOT "${LOG_LEVEL}" MATCHES "^[012]$")
    message(FATAL_ERROR "LOG_LEVEL must be 0, 1 or 2")
endif()
list(APPEND PROJECT_COMPILE_DEFINES "LOG_LEVEL=${LOG_LEVEL}")
