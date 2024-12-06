///
/// \file tca6424.h
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

enum class tca6424_port {
    _0,
    _1,
    _2
};

struct tca6424_port_data {
    tca6424_port port {};
    uint8_t value {};
};

namespace detail {
    struct tca6424_counter : chips_counter<tca6424_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint8_t>
class tca6424 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "TCA6424";
    detail::tca6424_counter _counter;

public:
    CHIP_BASE_RESOLVE
    tca6424(bool log_enable)
        : tca6424 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    tca6424(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~tca6424() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final
    {
        return _counter.get_num();
    }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void configure_port(const tca6424_port_data& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x0c + static_cast<int>(data.port), data.value);
    }
    void configure_port(const tca6424_port_data& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<tca6424, error_type, NoerrorValue, tca6424_port_data, &tca6424::configure_port>(this, data, error);
    }
    void set_port(const tca6424_port_data& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x04 + static_cast<int>(data.port), data.value);
    }
    void set_port(const tca6424_port_data& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<tca6424, error_type, NoerrorValue, tca6424_port_data, &tca6424::set_port>(this, data, error);
    }
    void get_port(tca6424_port_data& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        read(0x04 + static_cast<int>(data.port), data.value);
    }
    value_type get_port(tca6424_port port) const
    {
        tca6424_port_data data {};
        data.port = port;
        get_port(data);
        return data.value;
    }
    value_type get_port(tca6424_port port, error_type& error) const noexcept
    {
        tca6424_port_data data {};
        data.port = port;
        helpers::noexcept_get_function<tca6424, error_type, NoerrorValue, tca6424_port_data, &tca6424::get_port>(this, data, error);
        return data.value;
    }
};

} // namespace chappi
