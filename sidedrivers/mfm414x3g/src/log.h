///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#pragma once
#include <codecvt>
#include <locale>

///
/// \brief      Конвертирование строки библиотеки BARDY в UTF8
///
/// \param      str   Строка типа BRDCHAR
///
/// \return     Строка типа char* UTF8
///
static std::wstring_convert<std::codecvt_utf8<wchar_t>> wchar_to_utf8_convert;
#ifdef UNICODE
#define BARDY_STR(str) wchar_to_utf8_convert.to_bytes(str).c_str()
#else
#define BARDY_STR(str) str
#endif

///
/// \brief      Вывод отладочного сообщения
///
/// \param      str   Строка в формате `printf`
/// \param      ...   Параметры
///
/// \return     void
///
extern bool debug_info; // признак вывода отладочных сообщений
#ifdef NDEBUG
#define LOG(str, ...) \
    {                 \
        if (debug_info)                                               \
            printf(">>> [%s]\t" str "\n", name_class, ##__VA_ARGS__); \
    }
#define LOG_WITHOUT_NEWLINE(str, ...) \
    {                                 \
        if (debug_info)                                          \
            printf(">>> [%s]\t" str, name_class, ##__VA_ARGS__); \
    }
#define LOG_WITHOUT_NAMECLASS(str, ...) \
    {                                   \
        if (debug_info)                 \
            printf(str, ##__VA_ARGS__); \
    }
#else
#define LOG(str, ...)                                                 \
    {                                                                 \
        if (debug_info)                                               \
            printf(">>> [%s]\t" str "\n", name_class, ##__VA_ARGS__); \
    }
#define LOG_WITHOUT_NEWLINE(str, ...)                            \
    {                                                            \
        if (debug_info)                                          \
            printf(">>> [%s]\t" str, name_class, ##__VA_ARGS__); \
    }
#define LOG_WITHOUT_NAMECLASS(str, ...) \
    {                                   \
        if (debug_info)                 \
            printf(str, ##__VA_ARGS__); \
    }
#endif
