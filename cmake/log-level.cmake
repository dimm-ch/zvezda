# Уровни вывода сервера и утилит; вывод библиотек и драйверов не меняется.
set(LOG_LEVEL "2" CACHE STRING "Console logs: 0=wrapper off, 1=legacy output, 2=all")
set_property(CACHE LOG_LEVEL PROPERTY STRINGS 0 1 2)
if(NOT "${LOG_LEVEL}" MATCHES "^[012]$")
    message(FATAL_ERROR "LOG_LEVEL must be 0, 1 or 2")
endif()
list(APPEND PROJECT_COMPILE_DEFINES "LOG_LEVEL=${LOG_LEVEL}")
