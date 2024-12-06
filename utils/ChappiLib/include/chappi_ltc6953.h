///
/// \file ltc6953.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 25.02.2021
///
/// \copyright InSys Copyright (c) 2021
///

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <thread>

#ifdef _MSC_VER
#if _MSVC_LANG < 201704
#error \    "This file requires compiler and library support for the ISO C++ 2020 standard or later."
#endif
#endif

#include "chappi_base.h"
#include "chappi_register.h"

namespace chappi {

namespace ltc6953_constants {
    const auto output_max_num { 11 };
    struct analog_delay {
        static const auto min { 0u };
        static const auto max { 63u };
    };
    struct digital_delay {
        static const auto min { 0u };
        static const auto max { 4095u };
    };
    struct divider {
        static const auto min { 1u };
        static const auto max { 4096u };
    };

} // namespace ltc6953_constants

namespace ltc6953_registers {
    using register_type = uint8_t;
    using register_addr_type = uint8_t;
    template <typename register_bits_type, register_addr_type register_addr>
    using register_abstract = ::chappi::register_abstract<register_type, register_bits_type, register_addr_type, register_addr>;

#pragma pack(push, 1)
    enum class PDALL_type : register_type {
        normal,
        powerdown
    };
    enum class PD_type : register_type {
        normal,
        muted_0,
        powerdown_output,
        powerdown_divider
    };
    enum class RESET_type : register_type {
        normal,
        reset
    };
    enum class SYSREF_MODE_type : register_type {
        free_run,
        gated_pulses,
        request_pass_through,
        pow2_sysct_pulses
    };
    enum class SRQEN_type : register_type {
        disabled,
        enabled
    };
    enum class SRQMODE_type : register_type {
        sync,
        sysref
    };
    enum class OINV_type : register_type {
        normal,
        inverted
    };
    enum class SYSREF_PULSE_COUNT_type : register_type {
        one_pulse,
        two_pulses,
        four_pulses,
        eight_pulses
    };
    enum class EZSYNC_MODE_type : register_type {
        normal,
        ez_sync_mode
    };
    enum class SSRQ_type : register_type {
        normal,
        synchronization
    };
    enum class FILTV_type : register_type {
        normal,
        slew_rate
    };
    struct register_bits_h00 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        register_type VCOOK : 1;
        register_type nVCOOK : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h01 {
        register_type X0 : 1;
        register_type X1 : 1;
        register_type X2 : 1;
        register_type X3 : 1;
        register_type X4 : 1;
        register_type X5 : 1;
        register_type X6 : 1;
        register_type INVSTAT : 1;
    };
    struct register_bits_h02 {
        RESET_type POR : 1;
        FILTV_type FILTV : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        register_type PDVCOPK : 1;
        const register_type D6 : 1;
        PDALL_type PDALL : 1;
    };
    struct register_bits_h03 {
        PD_type PDO : 2;
        PD_type PD1 : 2;
        PD_type PD2 : 2;
        PD_type PD3 : 2;
    };
    struct register_bits_h04 {
        PD_type PD4 : 2;
        PD_type PD5 : 2;
        PD_type PD6 : 2;
        PD_type PD7 : 2;
    };
    struct register_bits_h05 {
        PD_type PD8 : 2;
        PD_type PD9 : 2;
        PD_type PD10 : 2;
        const register_type D6 : 1;
        register_type TEMPO : 1;
    };
    struct register_bits_h0B {
        SSRQ_type SSRQ : 1;
        SYSREF_PULSE_COUNT_type SYSCT : 2;
        SRQMODE_type SRQMD : 1;
        EZSYNC_MODE_type EZMD : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h0C {
        register_type MD0 : 3;
        register_type MP0 : 5;
    };
    struct register_bits_h0D {
        register_type DDEL0_H : 4;
        OINV_type OINV0 : 1;
        SYSREF_MODE_type MODE0 : 2;
        SRQEN_type SRQEN0 : 1;
    };
    struct register_bits_h0E {
        register_type DDEL0_L;
    };
    struct register_bits_h0F {
        register_type ADEL0 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h10 {
        register_type MD1 : 3;
        register_type MP1 : 5;
    };
    struct register_bits_h11 {
        register_type DDEL1_H : 4;
        OINV_type OINV1 : 1;
        SYSREF_MODE_type MODE1 : 2;
        SRQEN_type SRQEN1 : 1;
    };
    struct register_bits_h12 {
        register_type DDEL1_L;
    };
    struct register_bits_h13 {
        register_type ADEL1 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h14 {
        register_type MD2 : 3;
        register_type MP2 : 5;
    };
    struct register_bits_h15 {
        register_type DDEL2_H : 4;
        OINV_type OINV2 : 1;
        SYSREF_MODE_type MODE2 : 2;
        SRQEN_type SRQEN2 : 1;
    };
    struct register_bits_h16 {
        register_type DDEL2_L;
    };
    struct register_bits_h17 {
        register_type ADEL2 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h18 {
        register_type MD3 : 3;
        register_type MP3 : 5;
    };
    struct register_bits_h19 {
        register_type DDEL3_H : 4;
        OINV_type OINV3 : 1;
        SYSREF_MODE_type MODE3 : 2;
        SRQEN_type SRQEN3 : 1;
    };
    struct register_bits_h1A {
        register_type DDEL3_L;
    };
    struct register_bits_h1B {
        register_type ADEL3 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h1C {
        register_type MD4 : 3;
        register_type MP4 : 5;
    };
    struct register_bits_h1D {
        register_type DDEL4_H : 4;
        OINV_type OINV4 : 1;
        SYSREF_MODE_type MODE4 : 2;
        SRQEN_type SRQEN4 : 1;
    };
    struct register_bits_h1E {
        register_type DDEL4_L;
    };
    struct register_bits_h1F {
        register_type ADEL4 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h20 {
        register_type MD5 : 3;
        register_type MP5 : 5;
    };
    struct register_bits_h21 {
        register_type DDEL5_H : 4;
        OINV_type OINV5 : 1;
        SYSREF_MODE_type MODE5 : 2;
        SRQEN_type SRQEN5 : 1;
    };
    struct register_bits_h22 {
        register_type DDEL5_L;
    };
    struct register_bits_h23 {
        register_type ADEL5 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h24 {
        register_type MD6 : 3;
        register_type MP6 : 5;
    };
    struct register_bits_h25 {
        register_type DDEL6_H : 4;
        OINV_type OINV6 : 1;
        SYSREF_MODE_type MODE6 : 2;
        SRQEN_type SRQEN6 : 1;
    };
    struct register_bits_h26 {
        register_type DDEL6_L;
    };
    struct register_bits_h27 {
        register_type ADEL6 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h28 {
        register_type MD7 : 3;
        register_type MP7 : 5;
    };
    struct register_bits_h29 {
        register_type DDEL7_H : 4;
        OINV_type OINV7 : 1;
        SYSREF_MODE_type MODE7 : 2;
        SRQEN_type SRQEN7 : 1;
    };
    struct register_bits_h2A {
        register_type DDEL7_L;
    };
    struct register_bits_h2B {
        register_type ADEL7 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h2C {
        register_type MD8 : 3;
        register_type MP8 : 5;
    };
    struct register_bits_h2D {
        register_type DDEL8_H : 4;
        OINV_type OINV8 : 1;
        SYSREF_MODE_type MODE8 : 2;
        SRQEN_type SRQEN8 : 1;
    };
    struct register_bits_h2E {
        register_type DDEL8_L;
    };
    struct register_bits_h2F {
        register_type ADEL8 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h30 {
        register_type MD9 : 3;
        register_type MP9 : 5;
    };
    struct register_bits_h31 {
        register_type DDEL9_H : 4;
        OINV_type OINV9 : 1;
        SYSREF_MODE_type MODE9 : 2;
        SRQEN_type SRQEN9 : 1;
    };
    struct register_bits_h32 {
        register_type DDEL9_L;
    };
    struct register_bits_h33 {
        register_type ADEL9 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h34 {
        register_type MD10 : 3;
        register_type MP10 : 5;
    };
    struct register_bits_h35 {
        register_type DDEL10_H : 4;
        OINV_type OINV10 : 1;
        SYSREF_MODE_type MODE10 : 2;
        SRQEN_type SRQEN10 : 1;
    };
    struct register_bits_h36 {
        register_type DDEL10_L;
    };
    struct register_bits_h37 {
        register_type ADEL10 : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
    };
    struct register_bits_h38 {
        const register_type PART : 4;
        const register_type REV : 4;
    };

#pragma pack(pop)
    using register_h00 = register_abstract<register_bits_h00, 0x00>;
    using register_h01 = register_abstract<register_bits_h01, 0x01>;
    using register_h02 = register_abstract<register_bits_h02, 0x02>;
    using register_h03 = register_abstract<register_bits_h03, 0x03>;
    using register_h04 = register_abstract<register_bits_h04, 0x04>;
    using register_h05 = register_abstract<register_bits_h05, 0x05>;
    using register_h06 = register_abstract<register_8bits, 0x06>;
    using register_h07 = register_abstract<register_8bits, 0x07>;
    using register_h08 = register_abstract<register_8bits, 0x08>;
    using register_h09 = register_abstract<register_8bits, 0x09>;
    using register_h0A = register_abstract<register_8bits, 0x0A>;
    using register_h0B = register_abstract<register_bits_h0B, 0x0B>;
    using register_h0C = register_abstract<register_bits_h0C, 0x0C>;
    using register_h0D = register_abstract<register_bits_h0D, 0x0D>;
    using register_h0E = register_abstract<register_bits_h0E, 0x0E>;
    using register_h0F = register_abstract<register_bits_h0F, 0x0F>;
    using register_h10 = register_abstract<register_bits_h10, 0x10>;
    using register_h11 = register_abstract<register_bits_h11, 0x11>;
    using register_h12 = register_abstract<register_bits_h12, 0x12>;
    using register_h13 = register_abstract<register_bits_h13, 0x13>;
    using register_h14 = register_abstract<register_bits_h14, 0x14>;
    using register_h15 = register_abstract<register_bits_h15, 0x15>;
    using register_h16 = register_abstract<register_bits_h16, 0x16>;
    using register_h17 = register_abstract<register_bits_h17, 0x17>;
    using register_h18 = register_abstract<register_bits_h18, 0x18>;
    using register_h19 = register_abstract<register_bits_h19, 0x19>;
    using register_h1A = register_abstract<register_bits_h1A, 0x1A>;
    using register_h1B = register_abstract<register_bits_h1B, 0x1B>;
    using register_h1C = register_abstract<register_bits_h1C, 0x1C>;
    using register_h1D = register_abstract<register_bits_h1D, 0x1D>;
    using register_h1E = register_abstract<register_bits_h1E, 0x1E>;
    using register_h1F = register_abstract<register_bits_h1F, 0x1F>;
    using register_h20 = register_abstract<register_bits_h20, 0x20>;
    using register_h21 = register_abstract<register_bits_h21, 0x21>;
    using register_h22 = register_abstract<register_bits_h22, 0x22>;
    using register_h23 = register_abstract<register_bits_h23, 0x23>;
    using register_h24 = register_abstract<register_bits_h24, 0x24>;
    using register_h25 = register_abstract<register_bits_h25, 0x25>;
    using register_h26 = register_abstract<register_bits_h26, 0x26>;
    using register_h27 = register_abstract<register_bits_h27, 0x27>;
    using register_h28 = register_abstract<register_bits_h28, 0x28>;
    using register_h29 = register_abstract<register_bits_h29, 0x29>;
    using register_h2A = register_abstract<register_bits_h2A, 0x2A>;
    using register_h2B = register_abstract<register_bits_h2B, 0x2B>;
    using register_h2C = register_abstract<register_bits_h2C, 0x2C>;
    using register_h2D = register_abstract<register_bits_h2D, 0x2D>;
    using register_h2E = register_abstract<register_bits_h2E, 0x2E>;
    using register_h2F = register_abstract<register_bits_h2F, 0x2F>;
    using register_h30 = register_abstract<register_bits_h30, 0x30>;
    using register_h31 = register_abstract<register_bits_h31, 0x31>;
    using register_h32 = register_abstract<register_bits_h32, 0x32>;
    using register_h33 = register_abstract<register_bits_h33, 0x33>;
    using register_h34 = register_abstract<register_bits_h34, 0x34>;
    using register_h35 = register_abstract<register_bits_h35, 0x35>;
    using register_h36 = register_abstract<register_bits_h36, 0x36>;
    using register_h37 = register_abstract<register_bits_h37, 0x37>;
    using register_h38 = register_abstract<register_bits_h38, 0x38>;

}; // namespace ltc6953_registers

enum class ltc6953_output {
    out0,
    out1,
    out2,
    out3,
    out4,
    out5,
    out6,
    out7,
    out8,
    out9,
    out10
};

using ltc6953_output_powerdown_mode = ltc6953_registers::PD_type;

struct ltc6953_output_powerdown {
    ltc6953_output output {};
    ltc6953_output_powerdown_mode powerdown {};
};

struct ltc6953_output_inversion {
    ltc6953_output output {};
    bool inverted {};
};

struct ltc6953_digital_delay {
    ltc6953_output output {};
    uint16_t delay : 12;
};

struct ltc6953_analog_delay {
    ltc6953_output output {};
    uint16_t delay : 6;
};

struct ltc6953_divider {
    ltc6953_output output {};
    uint16_t divider : 13;
};

using ltc6953_sysref_mode = ltc6953_registers::SYSREF_MODE_type;

struct ltc6953_output_sync_mode {
    ltc6953_output output {};
    ltc6953_sysref_mode mode {};
    bool enabled {};
};

using ltc6953_srq_mode = ltc6953_registers::SRQMODE_type;
using ltc6953_sysref_pulse_count = ltc6953_registers::SYSREF_PULSE_COUNT_type;
struct ltc6953_sync_mode {
    ltc6953_srq_mode srq_mode {};
    ltc6953_sysref_pulse_count pulse_count {};
    bool ezsync_mode {};
};

namespace detail {
    struct ltc6953_counter : chips_counter<ltc6953_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint8_t>
class ltc6953 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "LTC6953";
    detail::ltc6953_counter _counter;
    mutable bool _is_integer_mode {};

public:
    CHIP_BASE_RESOLVE ltc6953(bool log_enable)
        : ltc6953 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    ltc6953(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~ltc6953() noexcept
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
        using namespace ltc6953_registers;
        register_h02 reg_h02 {};
        _read(reg_h02);
        reg_h02.data.bits.POR = RESET_type::reset;
        _write(reg_h02);
        reg_h02.data.bits.POR = RESET_type::normal;
        _write(reg_h02);
    }
    void reset(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<ltc6953, error_type, NoerrorValue, &ltc6953::reset>(this, error);
    }
    void chip_enable(bool enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        register_h02 reg_h02 {};
        _read(reg_h02);
        reg_h02.data.bits.PDALL = (enabled) ? PDALL_type::normal : PDALL_type::powerdown;
        _write(reg_h02);
    }
    void chip_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::chip_enable>(this, enabled, error);
    }
    void is_enabled(bool& enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        register_h02 reg_h02 {};
        _read(reg_h02);
        enabled = (reg_h02.data.bits.PDALL == PDALL_type::normal) ? true : false;
    }
    bool is_enabled() const
    {
        bool enabled {};
        is_enabled(enabled);
        return enabled;
    }
    bool is_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<ltc6953, error_type, NoerrorValue, bool, &ltc6953::is_enabled>(this, error);
    }
    void is_vco_valid(bool& is_valid) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        register_h00 reg_h00 {};
        _read(reg_h00);
        is_valid = bool(reg_h00.data.bits.VCOOK);
    }
    bool is_vco_valid() const
    {
        bool is_valid {};
        is_vco_valid(is_valid);
        return is_valid;
    }
    bool is_vco_valid(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<ltc6953, error_type, NoerrorValue, bool, &ltc6953::is_vco_valid>(this, error);
    }
    void set_output_inversion(const ltc6953_output_inversion& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        switch (data.output) {
        case ltc6953_output::out0: {
            register_h0D reg_h0D {};
            _read(reg_h0D);
            reg_h0D.data.bits.OINV0 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h0D);
            break;
        }
        case ltc6953_output::out1: {
            register_h11 reg_h11 {};
            _read(reg_h11);
            reg_h11.data.bits.OINV1 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h11);
            break;
        }
        case ltc6953_output::out2: {
            register_h15 reg_h15 {};
            _read(reg_h15);
            reg_h15.data.bits.OINV2 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h15);
            break;
        }
        case ltc6953_output::out3: {
            register_h19 reg_h19 {};
            _read(reg_h19);
            reg_h19.data.bits.OINV3 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h19);
            break;
        }
        case ltc6953_output::out4: {
            register_h1D reg_h1D {};
            _read(reg_h1D);
            reg_h1D.data.bits.OINV4 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h1D);
            break;
        }
        case ltc6953_output::out5: {
            register_h21 reg_h21 {};
            _read(reg_h21);
            reg_h21.data.bits.OINV5 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h21);
            break;
        }
        case ltc6953_output::out6: {
            register_h25 reg_h25 {};
            _read(reg_h25);
            reg_h25.data.bits.OINV6 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h25);
            break;
        }
        case ltc6953_output::out7: {
            register_h29 reg_h29 {};
            _read(reg_h29);
            reg_h29.data.bits.OINV7 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h29);
            break;
        }
        case ltc6953_output::out8: {
            register_h2D reg_h2D {};
            _read(reg_h2D);
            reg_h2D.data.bits.OINV8 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h2D);
            break;
        }
        case ltc6953_output::out9: {
            register_h31 reg_h31 {};
            _read(reg_h31);
            reg_h31.data.bits.OINV9 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h31);
            break;
        }
        case ltc6953_output::out10: {
            register_h35 reg_h35 {};
            _read(reg_h35);
            reg_h35.data.bits.OINV10 = (data.inverted) ? OINV_type::inverted : OINV_type::normal;
            _write(reg_h35);
            break;
        }
        default: {
            throw std::invalid_argument("ltc6953::set_output_inversion: invalid argument for output "
                + std::to_string(register_to_integer(data.output)));
            break;
        }
        }
    }
    void set_output_inversion(const ltc6953_output_inversion& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_output_inversion>(this, data, error);
    }
    void set_output_powerdown(const ltc6953_output_powerdown& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        switch (data.output) {
        case ltc6953_output::out0: {
            register_h03 reg_h03 {};
            _read(reg_h03);
            reg_h03.data.bits.PDO = data.powerdown;
            _write(reg_h03);
            break;
        }
        case ltc6953_output::out1: {
            register_h03 reg_h03 {};
            _read(reg_h03);
            reg_h03.data.bits.PD1 = data.powerdown;
            _write(reg_h03);
            break;
        }
        case ltc6953_output::out2: {
            register_h03 reg_h03 {};
            _read(reg_h03);
            reg_h03.data.bits.PD2 = data.powerdown;
            _write(reg_h03);
            break;
        }
        case ltc6953_output::out3: {
            register_h03 reg_h03 {};
            _read(reg_h03);
            reg_h03.data.bits.PD3 = data.powerdown;
            _write(reg_h03);
            break;
        }
        case ltc6953_output::out4: {
            register_h04 reg_h04 {};
            _read(reg_h04);
            reg_h04.data.bits.PD4 = data.powerdown;
            _write(reg_h04);
            break;
        }
        case ltc6953_output::out5: {
            register_h04 reg_h04 {};
            _read(reg_h04);
            reg_h04.data.bits.PD5 = data.powerdown;
            _write(reg_h04);
            break;
        }
        case ltc6953_output::out6: {
            register_h04 reg_h04 {};
            _read(reg_h04);
            reg_h04.data.bits.PD6 = data.powerdown;
            _write(reg_h04);
            break;
        }
        case ltc6953_output::out7: {
            register_h04 reg_h04 {};
            _read(reg_h04);
            reg_h04.data.bits.PD7 = data.powerdown;
            _write(reg_h04);
            break;
        }
        case ltc6953_output::out8: {
            register_h05 reg_h05 {};
            _read(reg_h05);
            reg_h05.data.bits.PD8 = data.powerdown;
            _write(reg_h05);
            break;
        }
        case ltc6953_output::out9: {
            register_h05 reg_h05 {};
            _read(reg_h05);
            reg_h05.data.bits.PD9 = data.powerdown;
            _write(reg_h05);
            break;
        }
        case ltc6953_output::out10: {
            register_h05 reg_h05 {};
            _read(reg_h05);
            reg_h05.data.bits.PD10 = data.powerdown;
            _write(reg_h05);
            break;
        }
        default: {
            throw std::invalid_argument("ltc6953::set_output_powerdown: invalid argument for output "
                + std::to_string(register_to_integer(data.output)));
            break;
        }
        }
    }
    void set_output_powerdown(const ltc6953_output_powerdown& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_output_powerdown>(this, data, error);
    }
    void set_digital_delay(const ltc6953_digital_delay& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        union {
            uint16_t value;
            uint8_t byte[2];
        } delay;
        delay.value = data.delay;
        switch (data.output) {
        case ltc6953_output::out0: {
            register_h0D reg_h0D {};
            _read(reg_h0D);
            reg_h0D.data.bits.DDEL0_H = delay.byte[1];
            _write(reg_h0D);
            register_h0E reg_h0E {};
            reg_h0E.data.bits.DDEL0_L = delay.byte[0];
            _write(reg_h0E);
            break;
        }
        case ltc6953_output::out1: {
            register_h11 reg_h11 {};
            _read(reg_h11);
            reg_h11.data.bits.DDEL1_H = delay.byte[1];
            _write(reg_h11);
            register_h12 reg_h12 {};
            reg_h12.data.bits.DDEL1_L = delay.byte[0];
            _write(reg_h12);
            break;
        }
        case ltc6953_output::out2: {
            register_h15 reg_h15 {};
            _read(reg_h15);
            reg_h15.data.bits.DDEL2_H = delay.byte[1];
            _write(reg_h15);
            register_h16 reg_h16 {};
            reg_h16.data.bits.DDEL2_L = delay.byte[0];
            _write(reg_h16);
            break;
        }
        case ltc6953_output::out3: {
            register_h19 reg_h19 {};
            _read(reg_h19);
            reg_h19.data.bits.DDEL3_H = delay.byte[1];
            _write(reg_h19);
            register_h1A reg_h1A {};
            reg_h1A.data.bits.DDEL3_L = delay.byte[0];
            _write(reg_h1A);
            break;
        }
        case ltc6953_output::out4: {
            register_h1D reg_h1D {};
            _read(reg_h1D);
            reg_h1D.data.bits.DDEL4_H = delay.byte[1];
            _write(reg_h1D);
            register_h1E reg_h1E {};
            reg_h1E.data.bits.DDEL4_L = delay.byte[0];
            _write(reg_h1E);
            break;
        }
        case ltc6953_output::out5: {
            register_h21 reg_h21 {};
            _read(reg_h21);
            reg_h21.data.bits.DDEL5_H = delay.byte[1];
            _write(reg_h21);
            register_h22 reg_h22 {};
            reg_h22.data.bits.DDEL5_L = delay.byte[0];
            _write(reg_h22);
            break;
        }
        case ltc6953_output::out6: {
            register_h25 reg_h25 {};
            _read(reg_h25);
            reg_h25.data.bits.DDEL6_H = delay.byte[1];
            _write(reg_h25);
            register_h26 reg_h26 {};
            reg_h26.data.bits.DDEL6_L = delay.byte[0];
            _write(reg_h26);
            break;
        }
        case ltc6953_output::out7: {
            register_h29 reg_h29 {};
            _read(reg_h29);
            reg_h29.data.bits.DDEL7_H = delay.byte[1];
            _write(reg_h29);
            register_h2A reg_h2A {};
            reg_h2A.data.bits.DDEL7_L = delay.byte[0];
            _write(reg_h2A);
            break;
        }
        case ltc6953_output::out8: {
            register_h2D reg_h2D {};
            _read(reg_h2D);
            reg_h2D.data.bits.DDEL8_H = delay.byte[1];
            _write(reg_h2D);
            register_h2E reg_h2E {};
            reg_h2E.data.bits.DDEL8_L = delay.byte[0];
            _write(reg_h2E);
            break;
        }
        case ltc6953_output::out9: {
            register_h31 reg_h31 {};
            _read(reg_h31);
            reg_h31.data.bits.DDEL9_H = delay.byte[1];
            _write(reg_h31);
            register_h32 reg_h32 {};
            reg_h32.data.bits.DDEL9_L = delay.byte[0];
            _write(reg_h32);
            break;
        }
        case ltc6953_output::out10: {
            register_h35 reg_h35 {};
            _read(reg_h35);
            reg_h35.data.bits.DDEL10_H = delay.byte[1];
            _write(reg_h35);
            register_h36 reg_h36 {};
            reg_h36.data.bits.DDEL10_L = delay.byte[0];
            _write(reg_h36);
            break;
        }
        default: {
            throw std::invalid_argument("ltc6953::ltc6953_digital_delay: invalid argument for output "
                + std::to_string(register_to_integer(data.output)));
            break;
        }
        }
    }
    void set_digital_delay(const ltc6953_digital_delay& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_digital_delay>(this, data, error);
    }
    void set_analog_delay(const ltc6953_analog_delay& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        if (data.delay > ltc6953_constants::analog_delay::max) {
            throw std::overflow_error("ltc6953::set_analog_delay: overflow argument for output "
                + std::to_string(register_to_integer(data.output)));
        }
        switch (data.output) {
        case ltc6953_output::out0: {
            register_h0F reg_h0F {};
            reg_h0F.data.bits.ADEL0 = data.delay;
            _write(reg_h0F);
            break;
        }
        case ltc6953_output::out1: {
            register_h13 reg_h13 {};
            reg_h13.data.bits.ADEL1 = data.delay;
            _write(reg_h13);
            break;
        }
        case ltc6953_output::out2: {
            register_h17 reg_h17 {};
            reg_h17.data.bits.ADEL2 = data.delay;
            _write(reg_h17);
            break;
        }
        case ltc6953_output::out3: {
            register_h1B reg_h1B {};
            reg_h1B.data.bits.ADEL3 = data.delay;
            _write(reg_h1B);
            break;
        }
        case ltc6953_output::out4: {
            register_h1F reg_h1F {};
            reg_h1F.data.bits.ADEL4 = data.delay;
            _write(reg_h1F);
            break;
        }
        case ltc6953_output::out5: {
            register_h23 reg_h23 {};
            reg_h23.data.bits.ADEL5 = data.delay;
            _write(reg_h23);
            break;
        }
        case ltc6953_output::out6: {
            register_h27 reg_h27 {};
            reg_h27.data.bits.ADEL6 = data.delay;
            _write(reg_h27);
            break;
        }
        case ltc6953_output::out7: {
            register_h2B reg_h2B {};
            reg_h2B.data.bits.ADEL7 = data.delay;
            _write(reg_h2B);
            break;
        }
        case ltc6953_output::out8: {
            register_h2F reg_h2F {};
            reg_h2F.data.bits.ADEL8 = data.delay;
            _write(reg_h2F);
            break;
        }
        case ltc6953_output::out9: {
            register_h33 reg_h33 {};
            reg_h33.data.bits.ADEL9 = data.delay;
            _write(reg_h33);
            break;
        }
        case ltc6953_output::out10: {
            register_h37 reg_h37 {};
            reg_h37.data.bits.ADEL10 = data.delay;
            _write(reg_h37);
            break;
        }
        default: {
            throw std::invalid_argument("ltc6953::set_analog_delay: invalid argument for output "
                + std::to_string(register_to_integer(data.output)));
            break;
        }
        }
    }
    void set_analog_delay(const ltc6953_analog_delay& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_analog_delay>(this, data, error);
    }
    void set_divider(const ltc6953_divider& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        if (data.divider > ltc6953_constants::divider::max || data.divider < ltc6953_constants::divider::min) {
            throw std::overflow_error("ltc6953::set_divider: overflow argument for output "
                + std::to_string(register_to_integer(data.output)));
        }
        uint8_t MDx {};
        const uint8_t MDx_max = 7;
        const uint8_t MPx_steps = 32;
        uint8_t pow2 = (data.divider - 1) / MPx_steps;
        while (pow2 != 0) {
            pow2 >>= 1;
            ++MDx;
            if (MDx == MDx_max) {
                break;
            }
        }
        uint8_t MPx = data.divider / (1 << MDx) - 1;
        if (data.divider % (1 << MDx) != 0) {
            const auto factor { double(data.divider) / (1 << MDx) };
            uint16_t nearest_low = std::floor(factor) * (1 << MDx);
            uint16_t nearest_high = std::ceil(factor) * (1 << MDx);
            throw std::invalid_argument("ltc6953::set_divider: invalid argument for output "
                + std::to_string(register_to_integer(data.output))
                + "\nDivider must be a multiple of " + std::to_string((1 << MDx))
                + "\nNearest value are " + std::to_string(nearest_low) + " or " + std::to_string(nearest_high));
        }
        switch (data.output) {
        case ltc6953_output::out0: {
            register_h0C reg_h0C {};
            reg_h0C.data.bits.MP0 = MPx;
            reg_h0C.data.bits.MD0 = MDx;
            _write(reg_h0C);
            break;
        }
        case ltc6953_output::out1: {
            register_h10 reg_h10 {};
            reg_h10.data.bits.MP1 = MPx;
            reg_h10.data.bits.MD1 = MDx;
            _write(reg_h10);
            break;
        }
        case ltc6953_output::out2: {
            register_h14 reg_h14 {};
            reg_h14.data.bits.MP2 = MPx;
            reg_h14.data.bits.MD2 = MDx;
            _write(reg_h14);
            break;
        }
        case ltc6953_output::out3: {
            register_h18 reg_h18 {};
            reg_h18.data.bits.MP3 = MPx;
            reg_h18.data.bits.MD3 = MDx;
            _write(reg_h18);
            break;
        }
        case ltc6953_output::out4: {
            register_h1C reg_h1C {};
            reg_h1C.data.bits.MP4 = MPx;
            reg_h1C.data.bits.MD4 = MDx;
            _write(reg_h1C);
            break;
        }
        case ltc6953_output::out5: {
            register_h20 reg_h20 {};
            reg_h20.data.bits.MP5 = MPx;
            reg_h20.data.bits.MD5 = MDx;
            _write(reg_h20);
            break;
        }
        case ltc6953_output::out6: {
            register_h24 reg_h24 {};
            reg_h24.data.bits.MP6 = MPx;
            reg_h24.data.bits.MD6 = MDx;
            _write(reg_h24);
            break;
        }
        case ltc6953_output::out7: {
            register_h28 reg_h28 {};
            reg_h28.data.bits.MP7 = MPx;
            reg_h28.data.bits.MD7 = MDx;
            _write(reg_h28);
            break;
        }
        case ltc6953_output::out8: {
            register_h2C reg_h2C {};
            reg_h2C.data.bits.MP8 = MPx;
            reg_h2C.data.bits.MD8 = MDx;
            _write(reg_h2C);
            break;
        }
        case ltc6953_output::out9: {
            register_h30 reg_h30 {};
            reg_h30.data.bits.MP9 = MPx;
            reg_h30.data.bits.MD9 = MDx;
            _write(reg_h30);
            break;
        }
        case ltc6953_output::out10: {
            register_h34 reg_h34 {};
            reg_h34.data.bits.MP10 = MPx;
            reg_h34.data.bits.MD10 = MDx;
            _write(reg_h34);
            break;
        }
        default: {
            throw std::invalid_argument("ltc6953::set_divider: invalid argument for output "
                + std::to_string(register_to_integer(data.output)));
            break;
        }
        }
    }
    void set_divider(const ltc6953_divider& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_divider>(this, data, error);
    }
    void set_output_sync_mode(const ltc6953_output_sync_mode& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        switch (data.output) {
        case ltc6953_output::out0: {
            register_h0D reg_h0D {};
            _read(reg_h0D);
            reg_h0D.data.bits.SRQEN0 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h0D.data.bits.MODE0 = data.mode;
            _write(reg_h0D);
            break;
        }
        case ltc6953_output::out1: {
            register_h11 reg_h11 {};
            _read(reg_h11);
            reg_h11.data.bits.SRQEN1 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h11.data.bits.MODE1 = data.mode;
            _write(reg_h11);
            break;
        }
        case ltc6953_output::out2: {
            register_h15 reg_h15 {};
            _read(reg_h15);
            reg_h15.data.bits.SRQEN2 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h15.data.bits.MODE2 = data.mode;
            _write(reg_h15);
            break;
        }
        case ltc6953_output::out3: {
            register_h19 reg_h19 {};
            _read(reg_h19);
            reg_h19.data.bits.SRQEN3 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h19.data.bits.MODE3 = data.mode;
            _write(reg_h19);
            break;
        }
        case ltc6953_output::out4: {
            register_h1D reg_h1D {};
            _read(reg_h1D);
            reg_h1D.data.bits.SRQEN4 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h1D.data.bits.MODE4 = data.mode;
            _write(reg_h1D);
            break;
        }
        case ltc6953_output::out5: {
            register_h21 reg_h21 {};
            _read(reg_h21);
            reg_h21.data.bits.SRQEN5 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h21.data.bits.MODE5 = data.mode;
            _write(reg_h21);
            break;
        }
        case ltc6953_output::out6: {
            register_h25 reg_h25 {};
            _read(reg_h25);
            reg_h25.data.bits.SRQEN6 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h25.data.bits.MODE6 = data.mode;
            _write(reg_h25);
            break;
        }
        case ltc6953_output::out7: {
            register_h29 reg_h29 {};
            _read(reg_h29);
            reg_h29.data.bits.SRQEN7 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h29.data.bits.MODE7 = data.mode;
            _write(reg_h29);
            break;
        }
        case ltc6953_output::out8: {
            register_h2D reg_h2D {};
            _read(reg_h2D);
            reg_h2D.data.bits.SRQEN8 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h2D.data.bits.MODE8 = data.mode;
            _write(reg_h2D);
            break;
        }
        case ltc6953_output::out9: {
            register_h31 reg_h31 {};
            _read(reg_h31);
            reg_h31.data.bits.SRQEN9 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h31.data.bits.MODE9 = data.mode;
            _write(reg_h31);
            break;
        }
        case ltc6953_output::out10: {
            register_h35 reg_h35 {};
            _read(reg_h35);
            reg_h35.data.bits.SRQEN10 = (data.enabled) ? SRQEN_type::enabled : SRQEN_type::disabled;
            reg_h35.data.bits.MODE10 = data.mode;
            _write(reg_h35);
            break;
        }
        default: {
            throw std::invalid_argument("ltc6953::set_sync_enabled: invalid argument for output "
                + std::to_string(register_to_integer(data.output)));
            break;
        }
        }
    }
    void set_output_sync_mode(const ltc6953_output_sync_mode& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_output_sync_mode>(this, data, error);
    }
    void sync_request() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace ltc6953_registers;
        register_h0B reg_h0B {};
        _read(reg_h0B);
        reg_h0B.data.bits.SSRQ = SSRQ_type::synchronization;
        _write(reg_h0B);
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        reg_h0B.data.bits.SSRQ = SSRQ_type::normal;
        _write(reg_h0B);
    }
    void sync_request(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<ltc6953, error_type, NoerrorValue, &ltc6953::sync_request>(this, error);
    }
    void set_sync_mode(const ltc6953_sync_mode& data)
    {
        using namespace ltc6953_registers;
        register_h0B reg_h0B {};
        _read(reg_h0B);
        reg_h0B.data.bits.SRQMD = data.srq_mode;
        reg_h0B.data.bits.SYSCT = data.pulse_count;
        reg_h0B.data.bits.EZMD = (data.ezsync_mode) ? EZSYNC_MODE_type::ez_sync_mode : EZSYNC_MODE_type::normal;
        _write(reg_h0B);
    }
    void set_sync_mode(const ltc6953_sync_mode& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_sync_mode>(this, data, error);
    }
    void set_input_buffer(bool slew_rate)
    {
        using namespace ltc6953_registers;
        register_h02 reg_h02 {};
        _read(reg_h02);
        reg_h02.data.bits.FILTV = (slew_rate) ? FILTV_type::slew_rate : FILTV_type::normal;
        _write(reg_h02);
    }
    void set_input_buffer(bool slew_rate, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<ltc6953, error_type, NoerrorValue, &ltc6953::set_input_buffer>(this, slew_rate, error);
    }

private:
    template <typename register_bits_type, ltc6953_registers::register_addr_type register_addr>
    void _read(ltc6953_registers::register_abstract<register_bits_type, register_addr>& reg) const
    {
        read(reg.addr, reg.data.value);
    }
    template <typename register_bits_type, ltc6953_registers::register_addr_type register_addr>
    void _write(const ltc6953_registers::register_abstract<register_bits_type, register_addr>& reg) const
    {
        write(reg.addr, reg.data.value);
    }
};

} // namespace chappi
