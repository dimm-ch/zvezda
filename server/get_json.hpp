#pragma once

#include <json.hpp>

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

using json = nlohmann::json;

namespace detail
{
    inline std::string trim_copy(std::string_view sv)
    {
        size_t begin = 0;
        size_t end = sv.size();

        while (begin < end && std::isspace(static_cast<unsigned char>(sv[begin])))
            ++begin;

        while (end > begin && std::isspace(static_cast<unsigned char>(sv[end - 1])))
            --end;

        return std::string(sv.substr(begin, end - begin));
    }

    inline bool iequals(std::string_view a, std::string_view b)
    {
        if (a.size() != b.size())
            return false;

        for (size_t i = 0; i < a.size(); ++i)
        {
            const unsigned char ca = static_cast<unsigned char>(a[i]);
            const unsigned char cb = static_cast<unsigned char>(b[i]);
            if (std::tolower(ca) != std::tolower(cb))
                return false;
        }
        return true;
    }

    template <typename T>
    std::optional<T> parse_integral_from_string(std::string_view sv)
    {
        static_assert(std::is_integral_v<T>, "T must be integral");

        const std::string s = trim_copy(sv);
        if (s.empty())
            return std::nullopt;

        T value{};
        const char* first = s.data();
        const char* last = s.data() + s.size();

        auto [ptr, ec] = std::from_chars(first, last, value, 0); // base 0: поддержка 123, 077, 0xFF
        if (ec != std::errc{} || ptr != last)
            return std::nullopt;

        return value;
    }

    template <typename T>
    std::optional<T> parse_floating_from_string(std::string_view sv)
    {
        static_assert(std::is_floating_point_v<T>, "T must be floating point");

        const std::string s = trim_copy(sv);
        if (s.empty())
            return std::nullopt;

        char* end = nullptr;
        errno = 0;

        const long double value = std::strtold(s.c_str(), &end);

        if (end != s.c_str() + s.size())
            return std::nullopt;

        if (errno == ERANGE)
            return std::nullopt;

        if (!std::isfinite(static_cast<double>(value)))
            return std::nullopt;

        if (value < static_cast<long double>(std::numeric_limits<T>::lowest()) ||
            value > static_cast<long double>(std::numeric_limits<T>::max()))
        {
            return std::nullopt;
        }

        return static_cast<T>(value);
    }

    inline std::optional<bool> parse_bool_from_string(std::string_view sv)
    {
        const std::string s = trim_copy(sv);
        if (s.empty())
            return std::nullopt;

        if (iequals(s, "true") || s == "1")
            return true;

        if (iequals(s, "false") || s == "0")
            return false;

        return std::nullopt;
    }

    template <typename T>
    std::optional<T> convert_json_number(const json& j)
    {
        if constexpr (std::is_same_v<T, int>)
        {
            if (j.is_number_integer())
            {
                const auto v = j.get<int64_t>();
                if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max())
                    return std::nullopt;
                return static_cast<int>(v);
            }

            if (j.is_number_unsigned())
            {
                const auto v = j.get<uint64_t>();
                if (v > static_cast<uint64_t>(std::numeric_limits<int>::max()))
                    return std::nullopt;
                return static_cast<int>(v);
            }

            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, unsigned>)
        {
            if (j.is_number_unsigned())
            {
                const auto v = j.get<uint64_t>();
                if (v > static_cast<uint64_t>(std::numeric_limits<unsigned>::max()))
                    return std::nullopt;
                return static_cast<unsigned>(v);
            }

            if (j.is_number_integer())
            {
                const auto v = j.get<int64_t>();
                if (v < 0 || static_cast<uint64_t>(v) > std::numeric_limits<unsigned>::max())
                    return std::nullopt;
                return static_cast<unsigned>(v);
            }

            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, int64_t>)
        {
            if (j.is_number_integer())
                return j.get<int64_t>();

            if (j.is_number_unsigned())
            {
                const auto v = j.get<uint64_t>();
                if (v > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
                    return std::nullopt;
                return static_cast<int64_t>(v);
            }

            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, uint64_t>)
        {
            if (j.is_number_unsigned())
                return j.get<uint64_t>();

            if (j.is_number_integer())
            {
                const auto v = j.get<int64_t>();
                if (v < 0)
                    return std::nullopt;
                return static_cast<uint64_t>(v);
            }

            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            if (!j.is_number())
                return std::nullopt;

            const double v = j.get<double>();
            if (!std::isfinite(v))
                return std::nullopt;
            return v;
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            if (!j.is_number())
                return std::nullopt;

            const double v = j.get<double>();
            if (!std::isfinite(v))
                return std::nullopt;

            if (v < static_cast<double>(std::numeric_limits<float>::lowest()) ||
                v > static_cast<double>(std::numeric_limits<float>::max()))
            {
                return std::nullopt;
            }

            return static_cast<float>(v);
        }
        else
        {
            static_assert(!sizeof(T*), "Unsupported numeric type");
        }
    }

    template <typename T>
    std::optional<T> from_json_value(const json& j)
    {
        if constexpr (std::is_same_v<T, bool>)
        {
            if (j.is_boolean())
                return j.get<bool>();

            if (j.is_string())
                return parse_bool_from_string(j.get_ref<const std::string&>());

            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            if (!j.is_string())
                return std::nullopt;

            return j.get<std::string>();
        }
        else if constexpr (std::is_same_v<T, int> ||
                            std::is_same_v<T, unsigned> ||
                            std::is_same_v<T, int64_t> ||
                            std::is_same_v<T, uint64_t>)
        {
            if (j.is_number_integer() || j.is_number_unsigned())
                return convert_json_number<T>(j);

            if (j.is_string())
                return parse_integral_from_string<T>(j.get_ref<const std::string&>());

            return std::nullopt;
        }
        else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>)
        {
            if (j.is_number())
                return convert_json_number<T>(j);

            if (j.is_string())
                return parse_floating_from_string<T>(j.get_ref<const std::string&>());

            return std::nullopt;
        }
        else
        {
            static_assert(!sizeof(T*), "Unsupported type");
        }
    }
} // namespace detail

template <typename T>
std::optional<T> get_json_value(const json& j, std::string_view key)
{
    static_assert(
        std::is_same_v<T, bool> ||
        std::is_same_v<T, int> ||
        std::is_same_v<T, unsigned> ||
        std::is_same_v<T, int64_t> ||
        std::is_same_v<T, uint64_t> ||
        std::is_same_v<T, double> ||
        std::is_same_v<T, float> ||
        std::is_same_v<T, std::string>,
        "Unsupported type for get_json_value"
    );

    if (!j.is_object())
        return std::nullopt;

    const auto it = j.find(std::string(key));
    if (it == j.end() || it->is_null())
        return std::nullopt;

    return detail::from_json_value<T>(*it);
}

template <typename T>
T get_json_value_or(const json& j, std::string_view key, T default_value)
{
    auto v = get_json_value<T>(j, key);
    return v ? *v : default_value;
}