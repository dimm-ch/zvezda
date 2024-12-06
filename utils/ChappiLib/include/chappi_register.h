///
/// \file chappi_register.h
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 26.02.2021
///
/// \copyright InSys Copyright (c) 2021
///
///

#pragma once

#include <algorithm>
#include <cstdint>
#include <forward_list>

namespace chappi {

#pragma pack(push, 1)

struct register_8bits {
    const uint8_t D0 : 1;
    const uint8_t D1 : 1;
    const uint8_t D2 : 1;
    const uint8_t D3 : 1;
    const uint8_t D4 : 1;
    const uint8_t D5 : 1;
    const uint8_t D6 : 1;
    const uint8_t D7 : 1;
};

struct register_16bits {
    const uint16_t D0 : 1;
    const uint16_t D1 : 1;
    const uint16_t D2 : 1;
    const uint16_t D3 : 1;
    const uint16_t D4 : 1;
    const uint16_t D5 : 1;
    const uint16_t D6 : 1;
    const uint16_t D7 : 1;
    const uint16_t D8 : 1;
    const uint16_t D9 : 1;
    const uint16_t D10 : 1;
    const uint16_t D11 : 1;
    const uint16_t D12 : 1;
    const uint16_t D13 : 1;
    const uint16_t D14 : 1;
    const uint16_t D15 : 1;
};

#pragma pack(pop)

template <typename register_type, typename register_bits_type,
    typename register_addr_type, register_addr_type register_addr>
struct register_abstract {
    union register_data_type {
        register_type value {};
        register_bits_type bits;
    } data {};
    const register_addr_type addr { register_addr };
};

template <typename register_type = std::size_t, typename value_type>
constexpr register_type register_to_integer(value_type value) noexcept
{
    return static_cast<std::underlying_type_t<value_type>>(value);
}

template <std::size_t register_max_num>
class registers_update {
    std::forward_list<std::size_t> _changed_registers {};

public:
    registers_update() = default;
    auto is_valid(std::size_t register_num) const noexcept
    {
        if (register_num < 0 || register_num > register_max_num - 1) {
            return false;
        }
        return true;
    }
    auto is_changed(std::size_t register_num) const noexcept
    {
        if (!is_valid(register_num)) {
            return false;
        }
        auto result = std::find(_changed_registers.cbegin(), _changed_registers.cend(), register_num);
        if (result != _changed_registers.cend()) {
            return true;
        }
        return false;
    }
    auto is_changed() const noexcept { return !_changed_registers.empty(); }
    template <typename Arg = std::size_t, typename... Args>
    auto set_changed(Arg register_num, Args... other_registers) noexcept
    {
        if (!is_valid(register_num)) {
            return false;
        }
        _changed_registers.push_front(register_num);
        return set_changed(other_registers...);
    }
    auto set_changed() noexcept { return true; }
    auto get_changed() const noexcept { return _changed_registers.front(); }
    auto clear_changed(std::size_t register_num) noexcept
    {
        if (!is_valid(register_num)) {
            return false;
        }
        _changed_registers.remove(register_num);
        return true;
    }
};

} // namespace chappi
