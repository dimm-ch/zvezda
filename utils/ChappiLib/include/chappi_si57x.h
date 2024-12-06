///
/// \file si57x.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 04.07.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once

#include <array>
#include <cmath>

#include "chappi_base.h"

namespace chappi {

namespace detail {
    struct si57x_counter : chips_counter<si57x_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint8_t>
class si57x final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "Si57x";
    detail::si57x_counter _counter;
    static const int _freq_regs_num { 12 };
    static constexpr double _fxtal_default { 114.285e6 };
    double _fxtal { _fxtal_default };
    constexpr int reg_addr_to_idx(int addr) const noexcept { return addr - start_addr; }

public:
    CHIP_BASE_RESOLVE
    si57x(bool log_enable)
        : si57x { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    si57x(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~si57x() noexcept
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
        write(135, 0x80);
    }
    void reset(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<si57x, error_type, NoerrorValue, &si57x::reset>(this, error);
    }
    void freeze_dco(bool enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(137, (enabled) ? 0x10 : 0x00);
    }
    void freeze_dco(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<si57x, error_type, NoerrorValue, bool, &si57x::freeze_dco>(this, enabled, error);
    }
    void apply_freq() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        write(135, 0x40);
    }
    void apply_freq(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<si57x, error_type, NoerrorValue, &si57x::apply_freq>(this, error);
    }
    void set_freq(double value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        freq_regs_type freq_regs {};
        if (_make_freq_regs(value, _fxtal, freq_regs) != true) {
            auto error_msg = std::string("can't calulate parameters for ") + get_name();
            throw std::runtime_error(error_msg);
        }
        addr_type addr { start_addr };
        for (auto& reg : freq_regs) {
            write(addr, reg);
            ++addr;
        }
    }
    void
    set_freq(double value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<si57x, error_type, NoerrorValue, double, &si57x::set_freq>(this, value, error);
    }
    void
    get_freq(double& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        freq_regs_type freq_regs {};
        addr_type addr { start_addr };
        for (auto& reg : freq_regs) {
            read(addr, reg);
            ++addr;
        }
        value = _calculate_freq(_fxtal, freq_regs);
    }
    double get_freq(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<si57x, error_type, NoerrorValue, double, &si57x::get_freq>(this, error);
    }
    double
    get_freq() const
    {
        double value {};
        get_freq(value);
        return value;
    }
    void
    set_fxtal(double fxtal) noexcept { _fxtal = fxtal; }
    double
    get_fxtal() const noexcept { return _fxtal; }
    void
    calib_fxtal(double freq_gen) noexcept
    {
        freq_regs_type freq_regs {};
        addr_type addr { start_addr };
        for (auto& reg : freq_regs) {
            read(addr, reg);
            ++addr;
        }
        _fxtal = _calculate_fxtal(freq_gen, freq_regs);
#if defined(CHAPPI_LOG_ENABLE)
        log << '[' << get_name() << ']' << " Fxtal = " << std::setprecision(12) << _fxtal << '\n';
#endif
    }
    void calib_fxtal(double freq_gen, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<si57x, error_type, NoerrorValue, double, &si57x::freeze_dco>(this, freq_gen, error);
    }

private:
    using freq_regs_type = std::array<value_type, _freq_regs_num>;
    static const addr_type start_addr { 7 };
    double _calculate_fxtal(double freq_gen, freq_regs_type& reg)
    {
        uint32_t rfreq_lo = 0xFF & reg[reg_addr_to_idx(12)];
        rfreq_lo |= (0xFF & reg[reg_addr_to_idx(11)]) << 8;
        rfreq_lo |= (0xFF & reg[reg_addr_to_idx(10)]) << 16;
        rfreq_lo |= (0xF & reg[reg_addr_to_idx(9)]) << 24;
        uint32_t rfreq_hi = (0xF0 & reg[reg_addr_to_idx(9)]) >> 4;
        rfreq_hi |= (0x3F & reg[reg_addr_to_idx(8)]) << 4;
        uint32_t hs_div = (0xE0 & reg[reg_addr_to_idx(7)]) >> 5;
        uint32_t n1 = (0xC0 & reg[reg_addr_to_idx(8)]) >> 6;
        n1 |= (0x1F & reg[reg_addr_to_idx(7)]) << 2;
        auto rfreq = double(rfreq_lo);
        rfreq /= 1024.0 * 1024.0 * 256.0;
        rfreq += double(rfreq_hi);
        auto hs_div_tmp = double(hs_div + 4);
        auto n1_tmp = (n1 == 1) ? 1.0 : double(0xFE & (n1 + 1));
        double fxtal = freq_gen;
        fxtal /= rfreq;
        fxtal *= hs_div_tmp * n1_tmp;
        return fxtal;
    }
    bool _make_freq_regs(double freq, double fxtal, freq_regs_type& reg) const noexcept
    {
        if (freq < 10000000.0) {
            freq = 10000000.0;
        }
        uint32_t rfreq_lo {};
        uint32_t rfreq_hi {};
        uint32_t hs_div {};
        uint32_t n1 {};
        auto success = _calculate_divider(freq, fxtal, rfreq_lo, rfreq_hi, hs_div, n1);
        if (!success) {
            return false;
        }
        reg[reg_addr_to_idx(7)] = reg[reg_addr_to_idx(13)] = value_type((hs_div << 5) | (n1 >> 2));
        reg[reg_addr_to_idx(8)] = reg[reg_addr_to_idx(14)] = value_type((n1 << 6) | (rfreq_hi >> 4));
        reg[reg_addr_to_idx(9)] = reg[reg_addr_to_idx(15)] = value_type((rfreq_hi << 4) | (rfreq_lo >> 24));
        reg[reg_addr_to_idx(10)] = reg[reg_addr_to_idx(16)] = value_type((rfreq_lo >> 16) & 0xFF);
        reg[reg_addr_to_idx(11)] = reg[reg_addr_to_idx(17)] = value_type((rfreq_lo >> 8) & 0xFF);
        reg[reg_addr_to_idx(12)] = reg[reg_addr_to_idx(18)] = rfreq_lo & 0xFF;
        return true;
    }
    bool _calculate_divider(double freq, double fxtal, uint32_t& rfreq_lo, uint32_t& rfreq_hi, uint32_t& hs_div, uint32_t& n1) const noexcept
    {
        double hs_div_tmp {};
        double n1_tmp {};

        double dco_min { 4850000000.0 };
        double dco_max { 5670000000.0 };
        double hs_div_valid[] { 4.0, 5.0, 6.0, 7.0, 9.0, 11.0 };
        double freq_dco {};
        double freq_tmp {};
        freq_dco = dco_max;
        for (int n1_count { 1 }; n1_count <= 128; ++n1_count) {
            if (n1_count > 1 && n1_count & 0x1) {
                continue;
            }
            for (int hs_div_count {}; hs_div_count < int(sizeof(hs_div_valid) / sizeof(hs_div_valid[0])); ++hs_div_count) {
                freq_tmp = freq * hs_div_valid[hs_div_count] * double(n1_count);
                if ((freq_tmp >= dco_min) && (freq_tmp <= freq_dco)) {
                    freq_dco = freq_tmp;
                    hs_div_tmp = hs_div_valid[hs_div_count];
                    n1_tmp = double(n1_count);
                }
            }
        }

        if ((hs_div_tmp == 0.0) || (n1_tmp == 0.0)) {
            return false;
        }
        double rfreq = freq * hs_div_tmp * n1_tmp;
        rfreq /= fxtal;
        rfreq_hi = uint32_t(rfreq);
        rfreq_lo = uint32_t((rfreq - double(rfreq_hi)) * 1024.0 * 1024.0 * 256.0);
        hs_div = uint32_t(hs_div_tmp - 4.0);
        n1 = uint32_t(n1_tmp - 1.0);
        return true;
    }
    double _calculate_freq(double fxtal, freq_regs_type& reg) const noexcept
    {
        uint32_t rfreq_lo = 0xFF & reg[reg_addr_to_idx(12)];
        rfreq_lo |= (0xFF & reg[reg_addr_to_idx(11)]) << 8;
        rfreq_lo |= (0xFF & reg[reg_addr_to_idx(10)]) << 16;
        rfreq_lo |= (0xF & reg[reg_addr_to_idx(9)]) << 24;
        uint32_t rfreq_ho = (0xF0 & reg[reg_addr_to_idx(9)]) >> 4;
        rfreq_ho |= (0x3F & reg[reg_addr_to_idx(8)]) << 4;
        uint32_t hs_div = (0xE0 & reg[reg_addr_to_idx(7)]) >> 5;
        uint32_t n1 = (0xC0 & reg[reg_addr_to_idx(8)]) >> 6;
        n1 |= (0x1F & reg[reg_addr_to_idx(7)]) << 2;
        auto rfreq = double(rfreq_lo);
        rfreq /= 1024.0 * 1024.0 * 256.0;
        rfreq += double(rfreq_ho);
        auto hs_div_tmp = double(hs_div + 4);
        auto n1_tmp = (n1 == 0) ? 1.0 : double(0xFE & (n1 + 1));
        double freq = fxtal;
        freq /= hs_div_tmp * n1_tmp;
        freq *= rfreq;
        return freq;
    }
};

} // namespace chappi
