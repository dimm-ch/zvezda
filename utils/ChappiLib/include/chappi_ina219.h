///
/// \file ina219.h
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

namespace detail {
    struct ina219_counter {
        chips_counter<ina219_counter> data;
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint16_t>
class ina219 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "INA219";
    detail::ina219_counter _counter;

public:
    CHIP_BASE_RESOLVE
    ina219(bool log_enable)
        : ina219 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    ina219(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~ina219() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.data.get_num(); }
    int get_counts() const noexcept final { return _counter.data.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void configure(value_type value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x00, value);
    }
    void configure(value_type value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ina219, error_type, NoerrorValue, value_type, &ina219::configure>(this, value, error);
    }
    void reset() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type value {};
        read(0x00, value);
        value |= (0x8000);
        write(0x00, value);
    }
    void reset(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<ina219, error_type, NoerrorValue, &ina219::reset>(this, error);
    }
    void get_shunt_voltage(double& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type retval {};
        read(0x01, retval);
        value = (retval & 0x3FFF);
    }
    double get_shunt_voltage(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<ina219, error_type, NoerrorValue, double, &ina219::get_shunt_voltage>(this, error);
    }
    double get_shunt_voltage() const
    {
        double value {};
        get_shunt_voltage(value);
        return value;
    }
    void get_bus_voltage(double& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type retval {};
        read(0x02, retval);
        value = ((retval >> 3) & 0x1FFF);
    }
    double get_bus_voltage(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<ina219, error_type, NoerrorValue, double, &ina219::get_bus_voltage>(this, error);
    }
    double get_bus_voltage() const
    {
        double value {};
        get_bus_voltage(value);
        return value;
    }
};

} // namespace chappi
