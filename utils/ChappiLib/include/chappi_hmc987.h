///
/// \file hmc987.h
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

union hmc987_outputs {
    enum class outs_bitmask : uint8_t {
        Out1 = (1 << 0),
        Out2 = (1 << 1),
        Out3 = (1 << 2),
        Out4 = (1 << 3),
        Out5 = (1 << 4),
        Out6 = (1 << 5),
        Out7 = (1 << 6),
        Out8 = (1 << 7),
        All = 0xFF,
        None = 0
    } bitmask;
#pragma pack(push, 1)
    struct {
        bool out1 : 1;
        bool out2 : 1;
        bool out3 : 1;
        bool out4 : 1;
        bool out5 : 1;
        bool out6 : 1;
        bool out7 : 1;
        bool out8 : 1;
    } bits;
#pragma pack(pop)
};

inline constexpr auto hmc987_make_outs_bitmask(hmc987_outputs::outs_bitmask bitmask)
{
    return bitmask;
}

template <typename arg = hmc987_outputs::outs_bitmask, typename... args>
constexpr auto hmc987_make_outs_bitmask(arg bitmask_arg, args... bitmask_args)
{
    std::underlying_type<hmc987_outputs::outs_bitmask>::type bitmask_init {};
    bitmask_init |= std::underlying_type<hmc987_outputs::outs_bitmask>::type(bitmask_arg);
    return hmc987_make_outs_bitmask(static_cast<hmc987_outputs::outs_bitmask>(bitmask_init), bitmask_args...);
}

template <typename arg = hmc987_outputs::outs_bitmask, typename... args>
constexpr auto hmc987_make_outs_bitmask(arg bitmask, arg bitmask_arg, args... bitmask_args)
{
    auto bitmask_init = std::underlying_type<hmc987_outputs::outs_bitmask>::type(bitmask);
    bitmask_init |= std::underlying_type<hmc987_outputs::outs_bitmask>::type(bitmask_arg);
    return hmc987_make_outs_bitmask(static_cast<hmc987_outputs::outs_bitmask>(bitmask_init), bitmask_args...);
}

enum class hmc987_gain : uint8_t {
    Disable = 0,
    neg_9_dBm,
    neg_6_dBm,
    neg_3_dBm,
    zero_dBm,
    pos_3_dBm
};

namespace detail {
    struct hmc987_counter : chips_counter<hmc987_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint8_t>
class hmc987 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "HMC987";
    detail::hmc987_counter _counter;

public:
    CHIP_BASE_RESOLVE
    hmc987(bool log_enable)
        : hmc987 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    hmc987(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~hmc987() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.get_num(); }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void init() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x00, 0x00);
    }
    void init(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<hmc987, error_type, NoerrorValue, &hmc987::init>(this, error);
    }
    void read_id(value_type& id) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        read(0x00, id);
    }
    value_type read_id() const
    {
        value_type id {};
        read_id(id);
        return id;
    }
    value_type read_id(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc987, error_type, NoerrorValue, value_type, &hmc987::read_id>(this, error);
    }
    void chip_enable(bool enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x01, (enabled) ? 0x01 : 0x00);
    }
    void chip_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc987, error_type, NoerrorValue, bool, &hmc987::chip_enable>(this, enabled, error);
    }
    void is_enabled(bool& enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type value;
        write(0x00, 0x01);
        read(0x00, value);
        enabled = (value != 0) ? true : false;
    }
    bool is_enabled() const
    {
        bool enabled {};
        is_enabled(enabled);
        return enabled;
    }
    bool is_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc987, error_type, NoerrorValue, bool, &hmc987::is_enabled>(this, error);
    }
    void enable_buffers(hmc987_outputs::outs_bitmask bitmask) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x02, static_cast<value_type>(bitmask));
    }
    void enable_buffers(hmc987_outputs::outs_bitmask bitmask, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc987, error_type, NoerrorValue, hmc987_outputs::outs_bitmask, &hmc987::enable_buffers>(this, bitmask, error);
    }
    void state_buffers(hmc987_outputs::outs_bitmask& bitmask) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type value;
        write(0x00, 0x02);
        read(0x00, value);
        bitmask = static_cast<hmc987_outputs::outs_bitmask>(value);
    }
    hmc987_outputs::outs_bitmask state_buffers() const
    {
        hmc987_outputs::outs_bitmask bitmask {};
        state_buffers(bitmask);
        return bitmask;
    }
    hmc987_outputs::outs_bitmask state_buffers(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc987, error_type, NoerrorValue, hmc987_outputs::outs_bitmask, &hmc987::state_buffers>(this, error);
    }
    void set_gain(hmc987_gain gain) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(0x04, static_cast<value_type>(gain) & 0x07);
    }
    void set_gain(hmc987_gain gain, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<hmc987, error_type, NoerrorValue, hmc987_gain, &hmc987::set_gain>(this, gain, error);
    }
    void get_gain(hmc987_gain& gain) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        value_type value;
        write(0x00, 0x04);
        read(0x00, value);
        gain = static_cast<hmc987_gain>(value);
    }
    hmc987_gain get_gain() const
    {
        hmc987_gain gain {};
        get_gain(gain);
        return gain;
    }
    hmc987_gain get_gain(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<hmc987, error_type, NoerrorValue, hmc987_gain, &hmc987::get_gain>(this, error);
    }
};

} // namespace chappi
