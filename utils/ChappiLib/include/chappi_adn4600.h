///
/// \file adn4600.h
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

struct adn4600_xpt_data {
    uint16_t input {};
    uint16_t output {};
};

namespace detail {
    struct adn4600_counter : chips_counter<adn4600_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint16_t>
class adn4600 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "ADN4600";
    detail::adn4600_counter _counter;

public:
    CHIP_BASE_RESOLVE
    adn4600(bool log_enable)
        : adn4600 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    adn4600(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~adn4600() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.get_num(); }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void reset() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x00, 0x01);
    }
    void reset(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<adn4600, error_type, NoerrorValue, &adn4600::reset>(this, error);
    }
    void xpt_config(const adn4600_xpt_data& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        const auto value = value_type(((data.input << 4) & 0x70) | (data.output & 0x07));
        write(0x40, value);
    }
    void xpt_config(const adn4600_xpt_data& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<adn4600, error_type, NoerrorValue, adn4600_xpt_data, &adn4600::xpt_config>(this, data, error);
    }
    void xpt_update() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x41, 0x01);
    }
    void xpt_update(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<adn4600, error_type, NoerrorValue, &adn4600::xpt_update>(this, error);
    }
};

} // namespace chappi
