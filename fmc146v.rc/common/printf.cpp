#include <stdarg.h>
#include <stdio.h>

#include "printf.h"

extern int g_isDebugInfo;

void Printf(const char *str, ...)
{
    if(!g_isDebugInfo)
        return;

    char buf[4096];

    // Формирование строки
    va_list marker;
    va_start(marker, str);
    vsprintf(buf, str, marker);
    va_end(marker);
    // -------------------

    fprintf(stderr, "%s", buf);
}
