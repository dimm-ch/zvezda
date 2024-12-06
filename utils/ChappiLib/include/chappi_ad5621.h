///
/// \file ad5621.h
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
    struct ad5621_counter : chips_counter<ad5621_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint16_t>
class ad5621 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "AD5621";
    detail::ad5621_counter _counter;

public:
    CHIP_BASE_RESOLVE
    ad5621(bool log_enable)
        : ad5621 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    ad5621(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~ad5621() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.get_num(); }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void set_value(value_type value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x00, value_type(value << 2));
    }
    void set_value(value_type value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ad5621, error_type, NoerrorValue, value_type, &ad5621::set_value>(this, value, error);
    }
};

} // namespace chappi
