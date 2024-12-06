///
/// \file ltc2991.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 04.07.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once

#include "chappi_base.h"

namespace chappi {

struct ltc2991_data {
    double Tint {};
    double V1 {};
    double V2 {};
    double V3 {};
    double V4 {};
    double V5 {};
    double V6 {};
    double V7 {};
    double V8 {};
};

enum class ltc2991_channel {
    _1,
    _2,
    _3,
    _4,
    _5,
    _6,
    _7,
    _8
};

struct ltc2991_channel_data {
    ltc2991_channel_data() = default;
    ltc2991_channel channel {};
    double value {};
};

namespace detail {
    struct ltc2991_counter : chips_counter<ltc2991_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint8_t>
class ltc2991 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "LTC2991";
    detail::ltc2991_counter _counter;

public:
    CHIP_BASE_RESOLVE
    ltc2991(bool log_enable)
        : ltc2991 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    ltc2991(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~ltc2991() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.get_num(); }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void enable_all_channels() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type value {};
        read(0x01, value);
        value |= 0b11111000;
        write(0x01, value);
    }
    void enable_all_channels(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<ltc2991, error_type, NoerrorValue, &ltc2991::enable_all_channels>(this, error);
    }
    void repeated_mode(bool enable) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type value {};
        read(0x08, value);
        if (enable) {
            value |= value_type(0b00010000);
        } else {
            value &= value_type(~0b00010000);
        }
        write(0x08, value);
    }
    void repeated_mode(bool enable, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc2991, error_type, NoerrorValue, bool, &ltc2991::repeated_mode>(this, enable, error);
    }
    void get_temperature(double& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type lsb {}, msb {};
        read(0x1A, msb);
        read(0x1B, lsb);
        value = ((msb & 0b00011111) << 8 | lsb) / 16.0;
    }
    double get_temperature() const
    {
        double value {};
        get_temperature(value);
        return value;
    }
    double get_temperature(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<ltc2991, error_type, NoerrorValue, double, &ltc2991::get_temperature>(this, error);
    }
    void get_voltage(ltc2991_channel_data& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _get_voltage(0x0A + (static_cast<int>(data.channel) << 1), data.value);
    }
    double get_voltage(ltc2991_channel channel) const
    {
        ltc2991_channel_data data {};
        data.channel = channel;
        get_voltage(data);
        return data.value;
    }
    double get_voltage(ltc2991_channel channel, error_type& error) const noexcept
    {
        ltc2991_channel_data data {};
        data.channel = channel;
        helpers::noexcept_get_function<ltc2991, error_type, NoerrorValue, ltc2991_channel_data, &ltc2991::get_voltage>(this, data, error);
        return data.value;
    }
    void get_data(ltc2991_data& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value.Tint = get_temperature();
        value.V1 = get_voltage(ltc2991_channel::_1);
        value.V2 = get_voltage(ltc2991_channel::_2);
        value.V3 = get_voltage(ltc2991_channel::_3);
        value.V4 = get_voltage(ltc2991_channel::_4);
        value.V5 = get_voltage(ltc2991_channel::_5);
        value.V6 = get_voltage(ltc2991_channel::_6);
        value.V7 = get_voltage(ltc2991_channel::_7);
        value.V8 = get_voltage(ltc2991_channel::_8);
    }
    ltc2991_data get_data() const
    {
        ltc2991_data data {};
        get_data(data);
        return data;
    }
    ltc2991_data get_data(error_type& error) const
    {
        return helpers::noexcept_get_function<ltc2991, error_type, NoerrorValue, ltc2991_data, &ltc2991::get_data>(this, error);
    }

private:
    void _get_voltage(addr_type addr_msb, double& value) const
    {
        value_type val_lsb {}, val_msb {};
        read(addr_msb, val_msb);
        read(value_type(addr_msb + 1), val_lsb);
        value = ((val_msb & 0b00111111) << 8 | val_lsb) * 0.000305180;
    }
};

} // namespace chappi
