// -----------------------------------------------------------------------------
// logger :: вывод цветных форматированных сообщений в консоль
// author :: Alexander U.Jurankov (heir@mail.ru)
// -----------------------------------------------------------------------------
#pragma once
#include <array>
#include <cctype>
#include <codecvt>
#include <cstdio>
#include <limits>
#include <locale>
#include <map>
#include <stack>
#include <string>
#include <tuple>
#if defined(_WIN32)
#include <windows.h>
#endif

using std::map;
using std::stack;
using std::string;

namespace insys {

using map_str_str = map<string const, string const>;
const map_str_str esc_colors {
    { "black", "\x1B[30m" }, { "red", "\x1B[31m" }, { "green", "\x1B[32m" },
    { "yellow", "\x1B[33m" }, { "blue", "\x1B[34m" }, { "magenta", "\x1B[35m" },
    { "cyan", "\x1B[36m" }, { "white", "\x1B[37m" },

    { "black+", "\x1B[90m" }, { "red+", "\x1B[91m" }, { "green+", "\x1B[92m" },
    { "yellow+", "\x1B[93m" }, { "blue+", "\x1B[94m" }, { "magenta+", "\x1B[95m" },
    { "cyan+", "\x1B[96m" }, { "white+", "\x1B[97m" }
};

#if defined(_WIN32)
using map_str_int = map<string const, uint16_t const>;
const map_str_int win_colors {
    { "31", 0b0100 }, { "32", 0b0010 }, { "33", 0b0110 }, { "34", 0b0001 },
    { "35", 0b0101 }, { "36", 0b0011 }, { "37", 0b0111 }, { "90", 0b1000 },
    { "91", 0b1100 }, { "92", 0b1010 }, { "93", 0b1110 }, { "94", 0b1001 },
    { "95", 0b1101 }, { "96", 0b1011 }, { "97", 0b1111 }
};
#endif

// ----------------------------------------------------------------------------
// Класс вывода логов
// ----------------------------------------------------------------------------
class logger {
private:
    bool enable_print_debug = false; // признак вывода на консоль отладочных сообщений
    string header {}; // header сообщения
    string footer {}; // footer сообщения
    bool begin_output = true; // признак начала вывода строки
#if defined(_WIN32)
    HANDLE win_stdout;
#endif

#if defined(_WIN32)
    void win_output(string const& str)
    {
        auto past_colors = std::stack<uint16_t> {};
        auto info = CONSOLE_SCREEN_BUFFER_INFO {};

        // обработка ESC-последовательностей
        enum state_ops : size_t { SYMBOL,
            ESC_BEGIN,
            ESC_NUMCOLOR };
        state_ops state = SYMBOL;
        string out {};
        string esc_num {};

        for (auto ch : str) {
            switch (state) {
            case ESC_BEGIN:
                esc_num.clear();
                printf("%s", out.c_str());
                out.clear();
                state = ESC_NUMCOLOR;
                break;
            case ESC_NUMCOLOR:
                if (ch == 'm') {
                    state = SYMBOL;
                    if (esc_num[0] == '0') // возврат предыдущего цвета
                        SetConsoleTextAttribute(win_stdout, past_colors.top());
                    else { // установка цвета
                        GetConsoleScreenBufferInfo(win_stdout, &info);
                        past_colors.push(info.wAttributes); // запоминание текущего цвета
                        SetConsoleTextAttribute(win_stdout, win_colors.at(esc_num));
                    }
                } else
                    esc_num += ch;
                break;
            default:
                if (ch == '\x1B')
                    state = ESC_BEGIN;
                else
                    out += ch;
                break;
            }
        }

        printf("%s", out.c_str());
    }
#endif

    // получение параметров форматирования из строки
    // возвращает символ форматирования
    // fill -- значения заполнителя и ширины
    static auto get_argtype(string const& str)
    {
        char type {};
        string fill {};
        for (auto ch : str) {
            if (std::isdigit(ch) || ch == '.')
                fill += ch;
            else
                type = std::tolower(ch);
        }
        return std::make_tuple(type, fill);
    }

    template <typename T>
    using enable_is_integer = std::enable_if_t<std::numeric_limits<T>::is_integer && !std::is_same<T, char>::value, bool>;
    template <typename T>
    using enable_is_float = std::enable_if_t<std::is_floating_point<T>::value, bool>;
    template <typename T>
    using enable_is_string = std::enable_if_t<std::is_same<T, string const>::value || std::is_same<T, string>::value || std::is_same<T, string const&>::value || std::is_same<T, string&>::value, bool>;
    template <typename T>
    using enable_is_char_string = std::enable_if_t<std::is_same<T, char const*>::value || std::is_same<T, char*>::value, bool>;
    template <typename T>
    using enable_is_wchar_string = std::enable_if_t<std::is_same<T, wchar_t const*>::value || std::is_same<T, wchar_t*>::value, bool>;
    template <typename T>
    using enable_is_char = std::enable_if_t<std::is_same<T, char>::value, bool>;
    template <typename T>
    using enable_is_pointer = std::enable_if_t<std::is_pointer<T>::value
            && !std::is_same<T, char const*>::value && !std::is_same<T, char*>::value
            && !std::is_same<T, wchar_t const*>::value && !std::is_same<T, wchar_t*>::value,
        bool>;

    template <typename T, enable_is_integer<T> = true>
    static string convert(string const& str, T arg)
    {
        string format {};

        char type {};
        string fill {};
        std::tie(type, fill) = get_argtype(str);

        switch (type) {
        case 'x':
            format
                = "0x%0" + fill + "llX";
            break;
        default:
            format = "%" + fill + "lld";
        }

        char buf[512];
        snprintf(buf, 512, format.c_str(), static_cast<long long>(arg));
        return buf;
    }

    template <typename T, enable_is_float<T> = true>
    static string convert(string const& str, T arg)
    {
        char type {};
        string fill {};
        std::tie(type, fill) = get_argtype(str);

        string format = "%" + fill + "f";

        char buf[512];
        snprintf(buf, 512, format.c_str(), arg);
        return buf;
    }

    template <typename T, enable_is_string<T> = true>
    static string convert(string const&, T&& arg)
    {
        return arg;
    }

    template <typename T, enable_is_char_string<T> = true>
    static string convert(string const&, T arg)
    {
        return arg;
    }

    template <typename T, enable_is_wchar_string<T> = true>
    static string convert(string const&, T arg)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> wchar_to_string;
        return wchar_to_string.to_bytes(arg);
    }

    template <typename T, enable_is_char<T> = true>
    static string convert(string const&, T arg)
    {
        return string { arg };
    }

    template <typename T, enable_is_pointer<T> = true>
    static string convert(string const&, T arg)
    {
        string format = "0x%p";
        char buf[512];
        snprintf(buf, 512, format.c_str(), arg);
        return buf;
    }

public:
    logger()
        : logger("") {};
    logger(string const& hdr, string const& ftr = "")
        : header(hdr)
        , footer(ftr.empty() ? "" : ' ' + ftr)
    {
#if defined(_WIN32)
        win_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    }
    void enable_debug() { enable_print_debug = true; }
    void disable_debug() { enable_print_debug = false; }

    template <typename... Types>
    void println(string const& str, Types&&... args)
    {
        print(str + footer + '\n', std::forward<Types>(args)...);
        begin_output = true;
    }

    template <typename... Types>
    void print(string const& str, Types&&... args)
    {
        string output;

        if (begin_output) {
            if (!header.empty()) {
                output = header + ' ' + logger::format(str, std::forward<Types>(args)...);
            } else {
                output = logger::format(str, std::forward<Types>(args)...);
            }
            begin_output = false;
        } else {
            output = ' ' + logger::format(str, std::forward<Types>(args)...);
        }

        if (enable_print_debug == false)
            return;
#if defined(_WIN32)
        win_output(output);
#else
        printf("%s", output.c_str());
#endif
    }

    template <bool = true>
    static string format(string const& str)
    {
        return str;
    }

    template <typename T, typename... Types>
    static string format(string const& str, T&& arg, Types&&... args)
    {
        auto bpos { 0 }; // позиция символа '{'
        auto epos { 0 }; // позиция символа '}'

        auto begin_found = false;
        auto found = false;
        for (auto i = 0u; i < str.size(); i++) {
            auto ch = str[i];
            if (ch == '{') {
                bpos = i;
                begin_found = true;
            }
            if (ch == '}') {
                if (begin_found == false)
                    continue;
                epos = i;
                found = true;
                break;
            }
        }

        if (found) {
            auto format_str = str.substr(bpos + 1, epos - bpos - 1); // получим строку внутри '{...}'
            auto colon_pos = format_str.find(':'); // поиск разделителя ':'
            auto type = (colon_pos != string::npos) ? format_str.substr(colon_pos + 1) : ""; // строка после ':'

            auto color = format_str.substr(0, colon_pos); // строка перед ':'
            color = (esc_colors.find(color) != esc_colors.end()) ? esc_colors.at(color) : "";

            auto result = str.substr(0, bpos); // берём все, перед '{...}'
            result += (color.empty() == false) ? color : ""; // ... переключение на цвет
            result += logger::convert(format_str.substr(colon_pos + 1), std::forward<T>(arg)); // ... вывод параметра
            result += (color.empty() == false) ? "\x1B[0m" : ""; // ... отключение цвета
            result += logger::format(str.substr(epos + 1), std::forward<Types>(args)...); // рекурсионная обработка остальных параметров

            return result;
        } else
            return str;
    }
};

} // namespace insys
