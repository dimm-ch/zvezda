///
/// \file lmx2594.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 04.07.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <forward_list>
#include <thread>

#ifdef _MSC_VER
#if _MSVC_LANG < 201704
#error \    "This file requires compiler and library support for the ISO C++ 2020 standard or later."
#endif
#endif

#include "chappi_base.h"
#include "chappi_register.h"

namespace chappi {

namespace lmx2594_constants {

    const auto output_power_max { 63 };

    struct vco_frequency {
        static const auto min { 7450000000ull };
    };
    struct osc_frequency {
        static const auto min { 5000000u };
    };
    struct out_frequency {
        static const auto min { 10000000u };
        static const auto max { 15000000000ull };
    };

    struct pre_divider {
        static const auto min { 1u };
        static const auto max { 128u };
    };

    struct divider {
        static const auto min { 1u };
        static const auto max { 255u };
    };

    struct n_divider {
        static const auto min { 28u };
        static const auto max { 524287u };
    };

    struct fraction {
        struct denominator {
            static const auto max { UINT32_MAX };
        };
    };

    const uint32_t actual_channel_divider_array[] {
        2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 72, 96, 128, 192, 256, 384, 512, 768
    };

} // namespace lmx2594_constants

namespace lmx2594_registers {

#pragma pack(push, 1)

    using register_type = uint16_t;

    template <typename register_bits_type>
    union register_basic {
        register_type reg {};
        register_bits_type bits;
    };

    struct register_common {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class POWERDOWN_type : register_type {
        normal,
        powerdown
    };
    enum class RESET_type : register_type {
        normal,
        reset
    };
    enum class MUXOUT_LD_SEL_type : register_type {
        readback,
        lock_detect
    };
    enum class FCAL_HPFD_ADJ_type : register_type {
        lower_100_MHz,
        range_100_150_MHz,
        range_150_200_MHz,
        upper_200_MHz
    };
    enum class FCAL_LPFD_ADJ_type : register_type {
        upper_10_MHz,
        range_5_10_MHz,
        range_2p5_5_MHz,
        lower_2p5_MHz
    };
    enum class FCAL_EN_type : register_type {
        disabled,
        calibrate_vco
    };
    enum class OUT_MUTE_type : register_type {
        disabled,
        mute_vco_output
    };
    enum class VCO_PHASE_SYNC_type : register_type {
        disabled,
        phase_sync_mode
    };
    enum class RAMP_EN_type : register_type {
        disabled,
        freq_ramping_mode
    };

    struct register_R0 {
        POWERDOWN_type POWERDOWN : 1;
        RESET_type RESET : 1;
        MUXOUT_LD_SEL_type MUXOUT_LD_SEL : 1;
        FCAL_EN_type FCAL_EN : 1;
        const register_type D4 : 1;
        FCAL_LPFD_ADJ_type FCAL_LPFD_ADJ : 2;
        FCAL_HPFD_ADJ_type FCAL_HPFD_ADJ : 2;
        OUT_MUTE_type OUT_MUTE : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        VCO_PHASE_SYNC_type VCO_PHASE_SYNC : 1;
        RAMP_EN_type RAMP_EN : 1;
    };

    enum class CAL_CLK_DIV_type : register_type {
        div1,
        div2,
        div4,
        div8
    };

    struct register_R1 {
        CAL_CLK_DIV_type CAL_CLK_DIV : 3;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R4 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        register_type ACAL_CMP_DLY : 8;
    };

    enum class OUT_FORCE_type : register_type {
        disabled,
        forced
    };

    struct register_R7 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        OUT_FORCE_type OUT_FORCE : 1;
        const register_type D15 : 1;
    };

    enum class VCO_CAPCTRL_FORCE_type : register_type {
        disabled,
        forced
    };
    enum class VCO_DACISET_FORCE_type : register_type {
        disabled,
        forced
    };

    struct register_R8 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        VCO_CAPCTRL_FORCE_type VCO_CAPCTRL_FORCE : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        VCO_DACISET_FORCE_type VCO_DACISET_FORCE : 1;
        const register_type D15 : 1;
    };

    enum class OSC_2X_type : register_type {
        disabled,
        low_noise_freq_doubler
    };

    struct register_R9 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        OSC_2X_type OSC_2X : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class MULT_type : register_type {
        bypass = 1,
        mul3 = 3,
        mul4,
        mul5,
        mul6,
        mul7
    };

    struct register_R10 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        MULT_type MULT : 5;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R11 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        register_type PLL_R : 8;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R12 {
        register_type PLL_R_PRE : 12;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class CPG_type : register_type {
        current_0_mA = 0,
        current_6_mA,
        current_12_mA = 3,
        current_3_mA,
        current_9_mA,
        current_15_mA = 7
    };

    struct register_R14 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        CPG_type CPG : 3;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R16 {
        register_type VCO_DACISET : 9;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R17 {
        register_type VCO_DACISET_STRT : 9;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R19 {
        register_type VCO_CAPCTRL : 8;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class VCO_SEL_type : register_type {
        not_used,
        vco1,
        vco2,
        vco3,
        vco4,
        vco5,
        vco6,
        vco7
    };

    enum class VCO_SEL_FORCE_type : register_type {
        disabled,
        enabled
    };

    struct register_R20 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        VCO_SEL_FORCE_type VCO_SEL_FORCE : 1;
        VCO_SEL_type VCO_SEL : 3;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class SEG1_EN_type : register_type {
        disabled,
        driver_buffer_enabled
    };

    struct register_R31 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        SEG1_EN_type SEG1_EN : 1;
        const register_type D15 : 1;
    };

    struct register_R34 {
        register_type PLL_N_18_16 : 3;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R36 {
        register_type PLL_N_15_0;
    };

    enum class MASH_SEED_EN_type : register_type {
        disabled,
        enabled
    };

    struct register_R37 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        register_type PFD_DLY_SEL : 6;
        const register_type D14 : 1;
        MASH_SEED_EN_type MASH_SEED_EN : 1;
    };

    struct register_R38 {
        register_type PLL_DEN_31_16;
    };

    struct register_R39 {
        register_type PLL_DEN_15_0;
    };

    struct register_R40 {
        register_type MASH_SEED_31_16;
    };

    struct register_R41 {
        register_type MASH_SEED_15_0;
    };

    struct register_R42 {
        register_type PLL_NUM_31_16;
    };

    struct register_R43 {
        register_type PLL_NUM_15_0;
    };

    enum class MASH_ORDER_type : register_type {
        integer,
        frac1,
        frac2,
        frac3,
        frac4
    };

    enum class MASH_RESET_N_type : register_type {
        disabled,
        frac_mode_enabled
    };
    enum class OUT_PD_type : register_type {
        active,
        powerdown
    };

    struct register_R44 {
        MASH_ORDER_type MASH_ORDER : 3;
        const register_type D3 : 1;
        const register_type D4 : 1;
        MASH_RESET_N_type MASH_RESET_N : 1;
        OUT_PD_type OUTA_PD : 1;
        OUT_PD_type OUTB_PD : 1;
        register_type OUTA_PWR : 6;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class OUTA_MUX_type : register_type {
        chdiv,
        vco,
        high_z = 3
    };
    enum class OUT_ISET_type : register_type {
        boost_max,
        boost_1,
        boost_2,
        boost_off
    };

    struct register_R45 {
        register_type OUTB_PWR : 6;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        OUT_ISET_type OUT_ISET : 2;
        OUTA_MUX_type OUTA_MUX : 2;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class OUTB_MUX_type : register_type {
        chdiv,
        vco,
        sysref,
        high_z
    };

    struct register_R46 {
        OUTB_MUX_type OUTB_MUX : 2;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class INPIN_FMT_type : register_type {
        SYNC_SysRefReq_CMOS,
        SYNC_LVDS__SysRefReq_CMOS,
        SYNC_CMOS__SysRefReq_LVDS,
        SYNC_SysRefReq_LVDS
    };

    enum class INPIN_LVL_type : register_type {
        div4,
        div1,
        div2,
        invalid
    };
    enum class INPIN_HYST_type : register_type {
        disabled,
        hysteresis_enabled
    };
    enum class INPIN_IGNORE_type : register_type {
        SYNC_SysRefReq_used,
        SYNC_SysRefReq_ignored
    };

    struct register_R58 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        INPIN_FMT_type INPIN_FMT : 3;
        INPIN_LVL_type INPIN_LVL : 2;
        INPIN_HYST_type INPIN_HYST : 1;
        INPIN_IGNORE_type INPIN_IGNORE : 1;
    };

    enum class LD_TYPE_type : register_type {
        vco_status,
        vco_vtune_status
    };

    struct register_R59 {
        LD_TYPE_type LD_TYPE : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R60 {
        register_type LD_DLY;
    };

    struct register_R69 {
        register_type MASH_RST_COUNT_31_16;
    };

    struct register_R70 {
        register_type MASH_RST_COUNT_15_0;
    };

    enum class SYSREF_DIV_PRE_type : register_type {
        div1 = 1,
        div2 = 2,
        div4 = 4
    };
    enum class SYSREF_REPEAT_type : register_type {
        master_mode,
        repeater_mode
    };
    enum class SYSREF_EN_type : register_type {
        disabled,
        enabled
    };
    enum class SYSREF_PULSE_type : register_type {
        disabled,
        enabled
    };

    struct register_R71 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        SYSREF_REPEAT_type SYSREF_REPEAT : 1;
        SYSREF_EN_type SYSREF_EN : 1;
        SYSREF_PULSE_type SYSREF_PULSE : 1;
        SYSREF_DIV_PRE_type SYSREF_DIV_PRE : 3;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    inline constexpr register_type get_SYSREF_DIV(const uint16_t div) noexcept
    {
        const uint16_t min_div { 4 };
        const uint16_t max_div { 4098 };
        const register_type min_reg { 0 };
        const register_type max_reg { 2047 };
        if (div < min_div) {
            return min_reg;
        } else if (div > max_div) {
            return max_reg;
        }
        const register_type res = (div - min_div) / 2;
        return res;
    }

    struct register_R72 {
        register_type SYSREF_DIV : 11;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R73 {
        register_type JESD_DAC1_CTRL : 6;
        register_type JESD_DAC2_CTRL : 6;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R74 {
        register_type JESD_DAC3_CTRL : 6;
        register_type JESD_DAC4_CTRL : 6;
        register_type SYSREF_PULSE_CNT : 4;
    };

    enum class CHDIV_type : register_type {
        div2,
        div4,
        div6,
        div8,
        div12,
        div16,
        div24,
        div32,
        div48,
        div64,
        div72,
        div96,
        div128,
        div192,
        div256,
        div384,
        div512,
        div768
    };

    struct register_R75 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        CHDIV_type CHDIV : 5;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class QUICK_RECAL_EN_type : register_type {
        disable,
        enable
    };

    struct register_R78 {
        const register_type D0 : 1;
        register_type VCO_CAPCTRL_STRT : 8;
        QUICK_RECAL_EN_type QUICK_RECAL_EN : 1;
        const register_type D10 : 1;
        register_type RAMP_THRESH_32 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R79 {
        register_type RAMP_THRESH_31_16;
    };

    struct register_R80 {
        register_type RAMP_THRESH_15_0;
    };

    struct register_R81 {
        register_type RAMP_LIMIT_HIGH_32 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R82 {
        register_type RAMP_LIMIT_HIGH_31_16;
    };

    struct register_R83 {
        register_type RAMP_LIMIT_HIGH_15_0;
    };

    struct register_R84 {
        register_type RAMP_LIMIT_LOW_32 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R85 {
        register_type RAMP_LIMIT_LOW_31_16;
    };

    struct register_R86 {
        register_type RAMP_LIMIT_LOW_15_0;
    };

    enum class RAMP_BURST_EN_type : register_type {
        disable,
        boost_ramping_mode_enable
    };

    struct register_R96 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        register_type RAMP_BURST_COUNT : 13;
        RAMP_BURST_EN_type RAMP_BURST_EN : 1;
    };

    enum class RAMP_TRIG_type : register_type {
        disabled,
        clk_rising_edge,
        dir_rising_edge,
        always_triggered = 4,
        clk_falling_edge = 9,
        dir_falling_edge = 10
    };
    enum class RAMP0_RST_type : register_type {
        disabled,
        enabled
    };
    enum class RAMP_BURST_TRIG_type : register_type {
        ramp_transition,
        trigger_a,
        trigger_b
    };

    struct register_R97 {
        RAMP_BURST_TRIG_type RAMP_BURST_TRIG : 2;
        const register_type D2 : 1;
        RAMP_TRIG_type RAMP_TRIGA : 4;
        RAMP_TRIG_type RAMP_TRIGB : 4;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        RAMP0_RST_type RAMP0_RST : 1;
    };

    struct register_R98 {
        register_type RAMP0_DLY : 1;
        const register_type D1 : 1;
        register_type RAMP0_INC_29_16 : 14;
    };

    struct register_R99 {
        register_type RAMP0_INC_15_0;
    };

    struct register_R100 {
        register_type RAMP0_LEN;
    };

    struct register_R101 {
        register_type RAMP0_NEXT_TRIG : 2;
        const register_type D2 : 1;
        const register_type D3 : 1;
        register_type RAMP0_NEXT : 1;
        register_type RAMP1_RST : 1;
        register_type RAMP1_DLY : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R102 {
        register_type RAMP1_INC_29_16 : 14;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R103 {
        register_type RAMP1_INC_15_0;
    };

    struct register_R104 {
        register_type RAMP1_LEN;
    };

    struct register_R105 {
        register_type RAMP1_NEXT_TRIG : 2;
        const register_type D2 : 1;
        const register_type D3 : 1;
        register_type RAMP1_NEXT : 1;
        register_type RAMP_MANUAL : 1;
        register_type RAMP_DLY_CNT : 10;
    };

    struct register_R106 {
        register_type RAMP_SCALE_COUNT : 3;
        const register_type D3 : 1;
        register_type RAMP_TRIG_CAL : 1;
        const register_type D5 : 1;
        const register_type D6 : 1;
        const register_type D7 : 1;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    enum class rb_VCO_SEL_type : register_type {
        invalid,
        vco1,
        vco2,
        vco3,
        vco4,
        vco5,
        vco6,
        vco7
    };

    enum class rb_LD_VTUNE_type : register_type {
        vtune_low,
        invalid_state,
        locked,
        vtune_high
    };

    struct register_R110 {
        const register_type D0 : 1;
        const register_type D1 : 1;
        const register_type D2 : 1;
        const register_type D3 : 1;
        const register_type D4 : 1;
        rb_VCO_SEL_type rb_VCO_SEL : 3;
        const register_type D8 : 1;
        rb_LD_VTUNE_type rb_LD_VTUNE : 2;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R111 {
        register_type rb_VCO_CAPCTRL : 8;
        const register_type D8 : 1;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    struct register_R112 {
        register_type rb_VCO_DACISET : 9;
        const register_type D9 : 1;
        const register_type D10 : 1;
        const register_type D11 : 1;
        const register_type D12 : 1;
        const register_type D13 : 1;
        const register_type D14 : 1;
        const register_type D15 : 1;
    };

    namespace detail {

        struct registers_map {
            register_basic<register_R0> reg_R0 {};
            register_basic<register_R1> reg_R1 {};
            register_basic<register_common> reg_R2 {};
            register_basic<register_common> reg_R3 {};
            register_basic<register_R4> reg_R4 {};
            register_basic<register_common> reg_R5 {};
            register_basic<register_common> reg_R6 {};
            register_basic<register_R7> reg_R7 {};
            register_basic<register_R8> reg_R8 {};
            register_basic<register_R9> reg_R9 {};
            register_basic<register_R10> reg_R10 {};
            register_basic<register_R11> reg_R11 {};
            register_basic<register_R12> reg_R12 {};
            register_basic<register_common> reg_R13 {};
            register_basic<register_R14> reg_R14 {};
            register_basic<register_common> reg_R15 {};
            register_basic<register_R16> reg_R16 {};
            register_basic<register_R17> reg_R17 {};
            register_basic<register_common> reg_R18 {};
            register_basic<register_R19> reg_R19 {};
            register_basic<register_R20> reg_R20 {};
            register_basic<register_common> reg_R21 {};
            register_basic<register_common> reg_R22 {};
            register_basic<register_common> reg_R23 {};
            register_basic<register_common> reg_R24 {};
            register_basic<register_common> reg_R25 {};
            register_basic<register_common> reg_R26 {};
            register_basic<register_common> reg_R27 {};
            register_basic<register_common> reg_R28 {};
            register_basic<register_common> reg_R29 {};
            register_basic<register_common> reg_R30 {};
            register_basic<register_R31> reg_R31 {};
            register_basic<register_common> reg_R32 {};
            register_basic<register_common> reg_R33 {};
            register_basic<register_R34> reg_R34 {};
            register_basic<register_common> reg_R35 {};
            register_basic<register_R36> reg_R36 {};
            register_basic<register_R37> reg_R37 {};
            register_basic<register_R38> reg_R38 {};
            register_basic<register_R39> reg_R39 {};
            register_basic<register_R40> reg_R40 {};
            register_basic<register_R41> reg_R41 {};
            register_basic<register_R42> reg_R42 {};
            register_basic<register_R43> reg_R43 {};
            register_basic<register_R44> reg_R44 {};
            register_basic<register_R45> reg_R45 {};
            register_basic<register_R46> reg_R46 {};
            register_basic<register_common> reg_R47 {};
            register_basic<register_common> reg_R48 {};
            register_basic<register_common> reg_R49 {};
            register_basic<register_common> reg_R50 {};
            register_basic<register_common> reg_R51 {};
            register_basic<register_common> reg_R52 {};
            register_basic<register_common> reg_R53 {};
            register_basic<register_common> reg_R54 {};
            register_basic<register_common> reg_R55 {};
            register_basic<register_common> reg_R56 {};
            register_basic<register_common> reg_R57 {};
            register_basic<register_R58> reg_R58 {};
            register_basic<register_R59> reg_R59 {};
            register_basic<register_R60> reg_R60 {};
            register_basic<register_common> reg_R61 {};
            register_basic<register_common> reg_R62 {};
            register_basic<register_common> reg_R63 {};
            register_basic<register_common> reg_R64 {};
            register_basic<register_common> reg_R65 {};
            register_basic<register_common> reg_R66 {};
            register_basic<register_common> reg_R67 {};
            register_basic<register_common> reg_R68 {};
            register_basic<register_R69> reg_R69 {};
            register_basic<register_R70> reg_R70 {};
            register_basic<register_R71> reg_R71 {};
            register_basic<register_R72> reg_R72 {};
            register_basic<register_R73> reg_R73 {};
            register_basic<register_R74> reg_R74 {};
            register_basic<register_R75> reg_R75 {};
            register_basic<register_common> reg_R76 {};
            register_basic<register_common> reg_R77 {};
            register_basic<register_R78> reg_R78 {};
            register_basic<register_R79> reg_R79 {};
            register_basic<register_R80> reg_R80 {};
            register_basic<register_R81> reg_R81 {};
            register_basic<register_R82> reg_R82 {};
            register_basic<register_R83> reg_R83 {};
            register_basic<register_R84> reg_R84 {};
            register_basic<register_R85> reg_R85 {};
            register_basic<register_R86> reg_R86 {};
            register_basic<register_common> reg_R87 {};
            register_basic<register_common> reg_R88 {};
            register_basic<register_common> reg_R89 {};
            register_basic<register_common> reg_R90 {};
            register_basic<register_common> reg_R91 {};
            register_basic<register_common> reg_R92 {};
            register_basic<register_common> reg_R93 {};
            register_basic<register_common> reg_R94 {};
            register_basic<register_common> reg_R95 {};
            register_basic<register_R96> reg_R96 {};
            register_basic<register_R97> reg_R97 {};
            register_basic<register_R98> reg_R98 {};
            register_basic<register_R99> reg_R99 {};
            register_basic<register_R100> reg_R100 {};
            register_basic<register_R101> reg_R101 {};
            register_basic<register_R102> reg_R102 {};
            register_basic<register_R103> reg_R103 {};
            register_basic<register_R104> reg_R104 {};
            register_basic<register_R105> reg_R105 {};
            register_basic<register_R106> reg_R106 {};
            register_basic<register_common> reg_R107 {};
            register_basic<register_common> reg_R108 {};
            register_basic<register_common> reg_R109 {};
            register_basic<register_R110> reg_R110 {};
            register_basic<register_R111> reg_R111 {};
            register_basic<register_R112> reg_R112 {};
        };

        struct registers_range {
            int begin {};
            int end {};
        };

    } // namespace detail

    const int register_max_num { sizeof(detail::registers_map) / sizeof(register_type) };
    const detail::registers_range registers_range_common { 0, 78 };
    const detail::registers_range registers_range_ramping { 79, 106 };
    const detail::registers_range registers_range_readback { 107, 112 };

    namespace detail {

        union { // SNAS696C-MARCH 2017 - REVISED APRIL 2019
            const registers_map regs {
                .reg_R0 { .bits {
                    .POWERDOWN = POWERDOWN_type::normal,
                    .RESET = RESET_type::normal,
                    .MUXOUT_LD_SEL = MUXOUT_LD_SEL_type::readback,
                    .FCAL_EN = FCAL_EN_type::disabled,
                    .D4 = 1,
                    .FCAL_LPFD_ADJ = FCAL_LPFD_ADJ_type::upper_10_MHz,
                    .FCAL_HPFD_ADJ = FCAL_HPFD_ADJ_type::lower_100_MHz,
                    .OUT_MUTE = OUT_MUTE_type::disabled,
                    .D10 = 1,
                    .D13 = 1,
                    .VCO_PHASE_SYNC = VCO_PHASE_SYNC_type::disabled,
                    .RAMP_EN = RAMP_EN_type::disabled } },
                .reg_R1 { .bits { .CAL_CLK_DIV = CAL_CLK_DIV_type::div8, .D3 = 1, .D11 = 1 } },
                .reg_R2 { .bits { .D8 = 1, .D10 = 1 } },
                .reg_R3 { .bits { .D1 = 1, .D6 = 1, .D9 = 1, .D10 = 1 } },
                .reg_R4 { .bits { .D0 = 1, .D1 = 1, .D6 = 1, .ACAL_CMP_DLY = 10 } },
                .reg_R5 { .bits { .D3 = 1, .D6 = 1, .D7 = 1 } },
                .reg_R6 { .bits { .D1 = 1, .D11 = 1, .D14 = 1, .D15 = 1 } },
                .reg_R7 { .bits { .D1 = 1, .D4 = 1, .D5 = 1, .D7 = 1, .OUT_FORCE = OUT_FORCE_type::disabled } },
                .reg_R8 { .bits {
                    .VCO_CAPCTRL_FORCE = VCO_CAPCTRL_FORCE_type::disabled,
                    .D13 = 1,
                    .VCO_DACISET_FORCE = VCO_DACISET_FORCE_type::disabled } },
                .reg_R9 { .bits { .D2 = 1, .D9 = 1, .D10 = 1, .OSC_2X = OSC_2X_type::disabled } },
                .reg_R10 { .bits { .D3 = 1, .D4 = 1, .D6 = 1, .MULT = MULT_type::bypass, .D12 = 1 } },
                .reg_R11 { .bits { .D3 = 1, .PLL_R = 1 } },
                .reg_R12 { .bits { .PLL_R_PRE = 1, .D12 = 1, .D14 = 1 } },
                .reg_R13 { .bits { .D14 = 1 } },
                .reg_R14 { .bits { .CPG = CPG_type::current_15_mA, .D9 = 1, .D10 = 1, .D11 = 1, .D12 = 1 } },
                .reg_R15 { .bits { .D0 = 1, .D1 = 1, .D2 = 1, .D3 = 1, .D6 = 1, .D9 = 1, .D10 = 1 } },
                .reg_R16 { .bits { .VCO_DACISET = 128 } },
                .reg_R17 { .bits { .VCO_DACISET_STRT = 250 } },
                .reg_R18 { .bits { .D2 = 1, .D5 = 1, .D6 = 1 } },
                .reg_R19 { .bits { .VCO_CAPCTRL = 183, .D8 = 1, .D9 = 1, .D10 = 1, .D13 = 1 } },
                .reg_R20 { .bits { .D3 = 1,
                    .D6 = 1,
                    .VCO_SEL_FORCE = VCO_SEL_FORCE_type::disabled,
                    .VCO_SEL = VCO_SEL_type::vco7,
                    .D14 = 1,
                    .D15 = 1 } },
                .reg_R21 { .bits { .D0 = 1, .D10 = 1 } },
                .reg_R22 { .bits { .D0 = 1 } },
                .reg_R23 { .bits { .D2 = 1, .D3 = 1, .D4 = 1, .D5 = 1, .D6 = 1 } },
                .reg_R24 { .bits { .D1 = 1, .D3 = 1, .D4 = 1, .D8 = 1, .D9 = 1, .D10 = 1 } },
                .reg_R25 { .bits { .D0 = 1, .D1 = 1, .D3 = 1, .D5 = 1, .D10 = 1, .D11 = 1 } },
                .reg_R26 { .bits { .D4 = 1, .D5 = 1, .D7 = 1, .D8 = 1, .D10 = 1, .D11 = 1 } },
                .reg_R27 { .bits { .D1 = 1 } },
                .reg_R28 { .bits { .D3 = 1, .D7 = 1, .D10 = 1 } },
                .reg_R29 { .bits { .D2 = 1, .D3 = 1, .D7 = 1, .D8 = 1, .D12 = 1, .D13 = 1 } },
                .reg_R30 { .bits { .D2 = 1, .D3 = 1, .D7 = 1, .D8 = 1, .D12 = 1, .D13 = 1 } },
                .reg_R31 { .bits { .D2 = 1, .D3 = 1, .D5 = 1, .D6 = 1, .D7 = 1, .D8 = 1, .D9 = 1, .SEG1_EN = SEG1_EN_type::disabled } },
                .reg_R32 { .bits { .D0 = 1, .D1 = 1, .D4 = 1, .D7 = 1, .D8 = 1, .D9 = 1 } },
                .reg_R33 { .bits { .D0 = 1, .D5 = 1, .D9 = 1, .D10 = 1, .D11 = 1, .D12 = 1 } },
                .reg_R34 { .reg {} },
                .reg_R35 { .bits { .D2 = 1 } },
                .reg_R36 { .bits { .PLL_N_15_0 = 100 } },
                .reg_R37 { .bits { .D2 = 1, .PFD_DLY_SEL = 2, .MASH_SEED_EN = MASH_SEED_EN_type::disabled } },
                .reg_R38 { .reg {} },
                .reg_R39 { .reg {} },
                .reg_R40 { .reg {} },
                .reg_R41 { .reg {} },
                .reg_R42 { .reg {} },
                .reg_R43 { .reg {} },
                .reg_R44 { .bits {
                    .MASH_ORDER = MASH_ORDER_type::integer,
                    .MASH_RESET_N = MASH_RESET_N_type::frac_mode_enabled,
                    .OUTA_PD = OUT_PD_type::active,
                    .OUTB_PD = OUT_PD_type::powerdown,
                    .OUTA_PWR = 31 } },
                .reg_R45 { .bits { .D6 = 1, .D7 = 1, .OUT_ISET = OUT_ISET_type::boost_max, .OUTA_MUX = OUTA_MUX_type::vco, .D14 = 1, .D15 = 1 } },
                .reg_R46 { .bits {
                    .OUTB_MUX = OUTB_MUX_type::vco, .D2 = 1, .D3 = 1, .D4 = 1, .D5 = 1, .D6 = 1, .D7 = 1, .D8 = 1, .D9 = 1, .D10 = 1 } },
                .reg_R47 { .bits { .D8 = 1, .D9 = 1 } },
                .reg_R48 { .bits { .D8 = 1, .D9 = 1 } },
                .reg_R49 { .bits { .D7 = 1, .D8 = 1, .D14 = 1 } },
                .reg_R50 { .reg {} },
                .reg_R51 { .bits { .D7 = 1 } },
                .reg_R52 { .bits { .D5 = 1, .D11 = 1 } },
                .reg_R53 { .reg {} },
                .reg_R54 { .reg {} },
                .reg_R55 { .reg {} },
                .reg_R56 { .reg {} },
                .reg_R57 { .bits { .D5 = 1 } },
                .reg_R58 { .bits {
                    .D0 = 1,
                    .INPIN_FMT = INPIN_FMT_type::SYNC_SysRefReq_CMOS,
                    .INPIN_LVL = INPIN_LVL_type::div4,
                    .INPIN_HYST = INPIN_HYST_type::disabled,
                    .INPIN_IGNORE = INPIN_IGNORE_type::SYNC_SysRefReq_ignored } },
                .reg_R59 { .bits { .LD_TYPE = LD_TYPE_type::vco_vtune_status } },
                .reg_R60 { .reg {} },
                .reg_R61 { .bits { .D3 = 1, .D5 = 1, .D7 = 1 } },
                .reg_R62 { .bits { .D1 = 1, .D5 = 1, .D8 = 1, .D9 = 1 } },
                .reg_R63 { .reg {} },
                .reg_R64 { .bits { .D3 = 1, .D7 = 1, .D8 = 1, .D9 = 1, .D12 = 1 } },
                .reg_R65 { .reg {} },
                .reg_R66 { .bits { .D2 = 1, .D4 = 1, .D5 = 1, .D6 = 1, .D7 = 1, .D8 = 1 } },
                .reg_R67 { .reg {} },
                .reg_R68 { .bits { .D3 = 1, .D5 = 1, .D6 = 1, .D7 = 1, .D8 = 1, .D9 = 1 } },
                .reg_R69 { .reg {} },
                .reg_R70 { .reg {} },
                .reg_R71 { .bits {
                    .D0 = 1,
                    .SYSREF_REPEAT = SYSREF_REPEAT_type::master_mode,
                    .SYSREF_EN = SYSREF_EN_type::disabled,
                    .SYSREF_PULSE = SYSREF_PULSE_type::disabled,
                    .SYSREF_DIV_PRE = SYSREF_DIV_PRE_type::div4 } },
                .reg_R72 { .bits { .SYSREF_DIV = get_SYSREF_DIV(4) } },
                .reg_R73 { .bits { .JESD_DAC1_CTRL = 63 } },
                .reg_R74 { .reg {} },
                .reg_R75 { .bits { .CHDIV = CHDIV_type::div2, .D11 = 1 } },
                .reg_R76 { .bits { .D2 = 1, .D3 = 1 } },
                .reg_R77 { .reg {} },
                .reg_R78 { .bits { .D0 = 1, .QUICK_RECAL_EN = QUICK_RECAL_EN_type::disable } },
                .reg_R79 { .reg {} },
                .reg_R80 { .reg {} },
                .reg_R81 { .reg {} },
                .reg_R82 { .reg {} },
                .reg_R83 { .reg {} },
                .reg_R84 { .reg {} },
                .reg_R85 { .reg {} },
                .reg_R86 { .reg {} },
                .reg_R87 { .reg {} },
                .reg_R88 { .reg {} },
                .reg_R89 { .reg {} },
                .reg_R90 { .reg {} },
                .reg_R91 { .reg {} },
                .reg_R92 { .reg {} },
                .reg_R93 { .reg {} },
                .reg_R94 { .reg {} },
                .reg_R95 { .reg {} },
                .reg_R96 { .bits { .RAMP_BURST_EN = RAMP_BURST_EN_type::disable } },
                .reg_R97 { .bits {
                    .RAMP_BURST_TRIG = RAMP_BURST_TRIG_type::ramp_transition,
                    .RAMP_TRIGA = RAMP_TRIG_type::disabled,
                    .RAMP_TRIGB = RAMP_TRIG_type::disabled,
                    .D11 = 1,
                    .RAMP0_RST = RAMP0_RST_type::disabled } },
                .reg_R98 { .reg {} },
                .reg_R99 { .reg {} },
                .reg_R100 { .reg {} },
                .reg_R101 { .reg {} },
                .reg_R102 { .reg {} },
                .reg_R103 { .reg {} },
                .reg_R104 { .reg {} },
                .reg_R105 { .reg {} },
                .reg_R106 { .reg {} },
                .reg_R107 { .reg {} },
                .reg_R108 { .reg {} },
                .reg_R109 { .reg {} },
                .reg_R110 { .reg {} },
                .reg_R111 { .reg {} },
                .reg_R112 { .reg {} }
            };
            const register_type array[register_max_num];
        } const registers_map_defaults {};

    } // namespace detail

    union registers_map {
        detail::registers_map regs { detail::registers_map_defaults.regs };
        register_type array[register_max_num];
    };

    using registers_update = ::chappi::registers_update<lmx2594_registers::register_max_num>;

#pragma pack(pop)

} // namespace lmx2594_registers

enum class lmx2594_output {
    outa,
    outb
};

struct lmx2594_output_enable {
    lmx2594_output output {};
    bool enabled {};
};

struct lmx2594_output_power {
    lmx2594_output output {};
    int power {};
};

struct lmx2594_output_frequency {
    lmx2594_output output {};
    double reference {};
    double frequency {};
};

using lmx2594_output_a_mux = lmx2594_registers::OUTA_MUX_type;
using lmx2594_output_b_mux = lmx2594_registers::OUTB_MUX_type;
using lmx2594_channel_divider = lmx2594_registers::CHDIV_type;
using lmx2594_charge_pump_gain = lmx2594_registers::CPG_type;
using lmx2594_doubler = lmx2594_registers::OSC_2X_type;
using lmx2594_pre_divider = lmx2594_registers::register_type;
using lmx2594_multiplier = lmx2594_registers::MULT_type;
using lmx2594_divider = lmx2594_registers::register_type;
using lmx2594_n_divider = uint32_t;
using lmx2594_fractional_numerator = uint32_t;
using lmx2594_fractional_denomerator = uint32_t;
using lmx2594_lock_detect = lmx2594_registers::LD_TYPE_type;
using lmx2594_lock_detect_mux = lmx2594_registers::MUXOUT_LD_SEL_type;
using lmx2594_mash_order = lmx2594_registers::MASH_ORDER_type;

namespace detail {
    struct lmx2594_counter : chips_counter<lmx2594_counter> {
    };
} // namespace detail

template <typename ErrorType = int, ErrorType NoerrorValue = 0,
    typename DevAddrType = uint8_t, typename AddrType = uint8_t, typename ValueType = uint16_t>
class lmx2594 final : public chip_base<ErrorType, NoerrorValue, DevAddrType, AddrType, ValueType> {
    static constexpr auto _chip_name = "LMX2594";
    detail::lmx2594_counter _counter;
    mutable lmx2594_registers::registers_map _registers_map {};
    mutable lmx2594_registers::registers_update _registers_update {};
    mutable bool _is_integer_mode {};

public:
    CHIP_BASE_RESOLVE
    lmx2594(bool log_enable)
        : lmx2594 { (log_enable) ? std::clog.rdbuf() : nullptr }
    {
    }
    lmx2594(std::streambuf* buf_ptr = {}, reg_read_fn reg_read = {}, reg_write_fn reg_write = {}, dev_addr_type dev_addr = {})
        : chip_base<error_type, NoerrorValue, dev_addr_type, addr_type, value_type> { buf_ptr, reg_read, reg_write, dev_addr }
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    ~lmx2594() noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
    }
    int get_num() const noexcept final { return _counter.get_num(); }
    int get_counts() const noexcept final { return _counter.get_counts(); }
    std::string get_name() const noexcept final { return get_name(_chip_name, get_num()); }
    void update_changes() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        while (_registers_update.is_changed()) {
            const auto registers_num = _registers_update.get_changed();
            write(registers_num, _registers_map.array[registers_num]);
            _registers_update.clear_changed(registers_num);
        };
    }
    void reset() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R0.bits.RESET = RESET_type::reset;
        write(0, _registers_map.regs.reg_R0.reg);
        _registers_map.regs.reg_R0.bits.RESET = RESET_type::normal;
        write(0, _registers_map.regs.reg_R0.reg);
        auto register_count { register_max_num - 1 };
        do {
            write(register_count, _registers_map.array[register_count]);
        } while (register_count--);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    void reset(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<lmx2594, error_type, NoerrorValue, &lmx2594::reset>(this, error);
    }
    void chip_enable(bool enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R0.bits.POWERDOWN = (enabled) ? POWERDOWN_type::normal : POWERDOWN_type::powerdown;
        write(0, _registers_map.regs.reg_R0.reg);
    }
    void chip_enable(bool enabled, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::chip_enable>(this, enabled, error);
    }
    void is_enabled(bool& enabled) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        read(0, _registers_map.regs.reg_R0.reg);
        enabled = (_registers_map.regs.reg_R0.bits.POWERDOWN == POWERDOWN_type::normal) ? true : false;
    }
    bool is_enabled() const
    {
        bool enabled {};
        is_enabled(enabled);
        return enabled;
    }
    bool is_enabled(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<lmx2594, error_type, NoerrorValue, bool, &lmx2594::is_enabled>(this, error);
    }
    void is_locked(bool& locked) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        locked = _is_locked();
    }
    bool is_locked() const
    {
        bool locked {};
        is_locked(locked);
        return locked;
    }
    bool is_locked(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<lmx2594, error_type, NoerrorValue, bool, &lmx2594::is_locked>(this, error);
    }
    void wait_lock_detect(bool& locked) const
    { // FIXME:
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        const int timeout_us { 50 * 5000 };
        const int cycle_us { 10 };
        auto cycles = timeout_us / cycle_us;
        do {
            locked = _is_locked();
            if (locked) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(cycle_us));
        } while (--cycles);
    }
    bool wait_lock_detect() const
    {
        bool locked {};
        wait_lock_detect(locked);
        return locked;
    }
    bool wait_lock_detect(error_type& error) const noexcept
    {
        return helpers::noexcept_get_function<lmx2594, error_type, NoerrorValue, bool, &lmx2594::wait_lock_detect>(this, error);
    }
    void set_output_enabled(const lmx2594_output_enable& data) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_enabled(data);
        _registers_update.set_changed(44);
    }
    void update_output_enabled(const lmx2594_output_enable& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_enabled(data);
        _update_registers(44);
    }
    void update_output_enabled(const lmx2594_output_enable& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_output_enabled>(this, data, error);
    }
    void is_output_enabled(lmx2594_output_enable& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        read(44, _registers_map.regs.reg_R44.reg);
        if (data.output == lmx2594_output::outa) {
            data.enabled = (_registers_map.regs.reg_R44.bits.OUTA_PD == OUT_PD_type::active) ? true : false;
        } else if (data.output == lmx2594_output::outb) {
            data.enabled = (_registers_map.regs.reg_R44.bits.OUTB_PD == OUT_PD_type::active) ? true : false;
        }
    }
    bool is_output_enabled(lmx2594_output output) const
    {
        lmx2594_output_enable data {};
        data.output = output;
        is_output_enabled(data);
        return data.enabled;
    }
    bool is_output_enabled(lmx2594_output output, error_type& error) const noexcept
    {
        lmx2594_output_enable data {};
        data.output = output;
        helpers::noexcept_get_function<lmx2594, error_type, NoerrorValue, lmx2594_output, &lmx2594::is_output_enabled>(this, data, error);
        return data.enabled;
    }
    void set_output_power(const lmx2594_output_power& data) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_power(data);
        _registers_update.set_changed(45, 44);
    }
    void update_output_power(const lmx2594_output_power& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_power(data);
        _update_registers(45, 44);
    }
    void update_output_power(const lmx2594_output_power& data, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_output_power>(this, data, error);
    }
    void set_output_mux(const lmx2594_output_a_mux& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_mux(value);
        _registers_update.set_changed(45);
    }
    void update_output_mux(const lmx2594_output_a_mux& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_mux(value);
        _update_registers(45);
    }
    void update_output_mux(const lmx2594_output_a_mux& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_output_mux>(this, value, error);
    }
    void set_output_mux(const lmx2594_output_b_mux& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_mux(value);
        _registers_update.set_changed(46);
    }
    void update_output_mux(const lmx2594_output_b_mux& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_output_mux(value);
        _update_registers(46);
    }
    void update_output_mux(const lmx2594_output_b_mux& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_output_mux>(this, value, error);
    }
    void set_channel_divider(const lmx2594_channel_divider& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_channel_divider(value);
        _registers_update.set_changed(75, 31);
    }
    void update_channel_divider(const lmx2594_channel_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_channel_divider(value);
        _update_registers(75, 31);
    }
    void update_channel_divider(const lmx2594_channel_divider& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_channel_divider>(this, value, error);
    }
    void set_charge_pump_gain(const lmx2594_charge_pump_gain& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_charge_pump_gain(value);
        _registers_update.set_changed(14);
    }
    void update_charge_pump_gain(const lmx2594_charge_pump_gain& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_charge_pump_gain(value);
        _update_registers(14);
    }
    void update_charge_pump_gain(const lmx2594_charge_pump_gain& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_charge_pump_gain>(this, value, error);
    }
    void set_doubler(const lmx2594_doubler& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_doubler(value);
        _registers_update.set_changed(9);
    }
    void update_doubler(const lmx2594_doubler& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_doubler(value);
        _update_registers(9);
    }
    void update_doubler(const lmx2594_doubler& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_doubler>(this, value, error);
    }
    void set_pre_divider(const lmx2594_pre_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_pre_divider(value);
        _registers_update.set_changed(12);
    }
    void update_pre_divider(const lmx2594_pre_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_pre_divider(value);
        _update_registers(12);
    }
    void update_pre_divider(const lmx2594_pre_divider& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_pre_divider>(this, value, error);
    }
    void set_multiplier(const lmx2594_multiplier& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_multiplier(value);
        _registers_update.set_changed(10);
    }
    void update_multiplier(const lmx2594_multiplier& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_multiplier(value);
        _update_registers(10);
    }
    void update_multiplier(const lmx2594_multiplier& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_multiplier>(this, value, error);
    }
    void set_divider(const lmx2594_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_divider(value);
        _registers_update.set_changed(11);
    }
    void update_divider(const lmx2594_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_divider(value);
        _update_registers(11);
    }
    void update_divider(const lmx2594_divider& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_divider>(this, value, error);
    }
    void set_n_divider(const lmx2594_n_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_n_divider(value);
        _registers_update.set_changed(36, 34);
    }
    void update_n_divider(const lmx2594_n_divider& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_n_divider(value);
        _update_registers(36, 34);
    }
    void update_n_divider(const lmx2594_n_divider& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_n_divider>(this, value, error);
    }
    void set_fractional_numerator(const lmx2594_fractional_numerator& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_fractional_numerator(value);
        _registers_update.set_changed(43, 42);
    }
    void update_fractional_numerator(const lmx2594_fractional_numerator& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_fractional_numerator(value);
        _update_registers(43, 42);
    }
    void update_fractional_numerator(const lmx2594_fractional_numerator& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_fractional_numerator>(this, value, error);
    }
    void set_fractional_denomerator(const lmx2594_fractional_denomerator& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_fractional_denomerator(value);
        _registers_update.set_changed(39, 38);
    }
    void update_fractional_denomerator(const lmx2594_fractional_denomerator& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_fractional_denomerator(value);
        _update_registers(39, 38);
    }
    void update_fractional_denomerator(const lmx2594_fractional_denomerator& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_fractional_denomerator>(this, value, error);
    }
    void vco_calibrate() const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R0.bits.FCAL_EN = FCAL_EN_type::calibrate_vco;
        write(0, _registers_map.regs.reg_R0.reg);
        _registers_map.regs.reg_R0.bits.FCAL_EN = FCAL_EN_type::disabled;
    }
    void vco_calibrate(error_type& error) const noexcept
    {
        helpers::noexcept_void_function<lmx2594, error_type, NoerrorValue, &lmx2594::vco_calibrate>(this, error);
    }
    void set_lock_detect(const lmx2594_lock_detect& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_lock_detect(value);
        _registers_update.set_changed(59);
    }
    void update_lock_detect(const lmx2594_lock_detect& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_lock_detect(value);
        _update_registers(59);
    }
    void update_lock_detect(const lmx2594_lock_detect& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_lock_detect>(this, value, error);
    }
    void set_lock_detect_mux(const lmx2594_lock_detect_mux& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_lock_detect_mux(value);
        _registers_update.set_changed(0);
    }
    void update_lock_detect_mux(const lmx2594_lock_detect_mux& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_lock_detect_mux(value);
        _update_registers(0);
    }
    void update_lock_detect_mux(const lmx2594_lock_detect_mux& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_lock_detect_mux>(this, value, error);
    }
    void set_phase_detector_delay(uint64_t vco_frequency) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_phase_detector_delay(vco_frequency);
        _registers_update.set_changed(37);
    }
    void update_phase_detector_delay(uint64_t vco_frequency) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_phase_detector_delay(vco_frequency);
        _update_registers(37);
    }
    void update_phase_detector_delay(uint64_t vco_frequency, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_phase_detector_delay>(this, vco_frequency, error);
    }
    void set_vco_calibration_divider(uint64_t osc_frequency) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_vco_calibration_divider(osc_frequency);
        _registers_update.set_changed(1, 4);
    }
    void update_vco_calibration_divider(uint64_t osc_frequency) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_vco_calibration_divider(osc_frequency);
        _update_registers(1, 4);
    }
    void update_vco_calibration_divider(uint64_t osc_frequency, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_vco_calibration_divider>(this, osc_frequency, error);
    }
    void set_mash_order(const lmx2594_mash_order& value) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_mash_order(value);
        _registers_update.set_changed(44);
    }
    void update_mash_order(const lmx2594_mash_order& value) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_mash_order(value);
        _update_registers(44);
    }
    void update_mash_order(const lmx2594_mash_order& value, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_mash_order>(this, value, error);
    }
    void set_high_pd_frequency_calibration(uint32_t pd_frequency) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_high_pd_frequency_calibration(pd_frequency);
        _registers_update.set_changed(0);
    }
    void update_high_pd_frequency_calibration(uint32_t pd_frequency) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_high_pd_frequency_calibration(pd_frequency);
        _update_registers(0);
    }
    void update_high_pd_frequency_calibration(uint32_t pd_frequency, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_high_pd_frequency_calibration>(this, pd_frequency, error);
    }
    void set_low_pd_frequency_calibration(uint32_t pd_frequency) const noexcept
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_low_pd_frequency_calibration(pd_frequency);
        _registers_update.set_changed(0);
    }
    void update_low_pd_frequency_calibration(uint32_t pd_frequency) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        _set_low_pd_frequency_calibration(pd_frequency);
        _update_registers(0);
    }
    void update_low_pd_frequency_calibration(uint32_t pd_frequency, error_type& error) const noexcept
    {
        helpers::noexcept_set_function<lmx2594, error_type, NoerrorValue, &lmx2594::update_low_pd_frequency_calibration>(this, pd_frequency, error);
    }
    auto get_n_divider_min(uint64_t vco_frequency) const
    {
        using namespace lmx2594_registers;
        if (vco_frequency < lmx2594_constants::vco_frequency::min || vco_frequency > get_vco_frequency_max()) {
            throw std::invalid_argument("lmx2594::get_n_divider_min: invalid argument");
        }
        uint32_t n_divider_min {};
        switch (_registers_map.regs.reg_R44.bits.MASH_ORDER) {
        case MASH_ORDER_type::integer:
            if (vco_frequency > 12500000000ull) {
                n_divider_min = 32;
            } else {
                n_divider_min = 28;
            }
            break;
        case MASH_ORDER_type::frac1:
            if (vco_frequency > 12500000000ull) {
                n_divider_min = 36;
            } else if (vco_frequency > 10000000000ull) {
                n_divider_min = 32;
            } else {
                n_divider_min = 28;
            }
            break;
        case MASH_ORDER_type::frac2:
            if (vco_frequency > 10000000000ull) {
                n_divider_min = 36;
            } else {
                n_divider_min = 32;
            }
            break;
        case MASH_ORDER_type::frac3:
            if (vco_frequency > 10000000000ull) {
                n_divider_min = 40;
            } else {
                n_divider_min = 36;
            }
            break;
        case MASH_ORDER_type::frac4:
            if (vco_frequency > 10000000000ull) {
                n_divider_min = 48;
            } else {
                n_divider_min = 44;
            }
            break;
        default:
            throw std::invalid_argument("lmx2594::get_n_divider_min: invalid argument");
            break;
        }
        return n_divider_min;
    }
    static auto get_actual_channel_divider(lmx2594_channel_divider register_value) noexcept
    {
        return lmx2594_constants::actual_channel_divider_array[std::underlying_type_t<lmx2594_channel_divider>(register_value)];
    }
    auto get_osc_frequency_max() const noexcept
    {
        using namespace lmx2594_registers;
        if (_registers_map.regs.reg_R9.bits.OSC_2X != OSC_2X_type::disabled) {
            return 200000000ull;
        }
        return 1400000000ull;
    }
    auto get_pd_frequency_max() const noexcept
    {
        using namespace lmx2594_registers;
        if (_registers_map.regs.reg_R44.bits.MASH_ORDER == MASH_ORDER_type::integer) {
            return 400000000u;
        } else if (_registers_map.regs.reg_R44.bits.MASH_ORDER == MASH_ORDER_type::frac4) {
            return 240000000u;
        }
        return 300000000u;
    }
    auto get_pd_frequency_min() const noexcept
    {
        using namespace lmx2594_registers;
        if (_registers_map.regs.reg_R44.bits.MASH_ORDER == MASH_ORDER_type::integer) {
            return 125u;
        }
        return 5000u;
    }
    auto get_vco_frequency_max() const noexcept
    {
        using namespace lmx2594_registers;
        if (_registers_map.regs.reg_R75.bits.CHDIV >= CHDIV_type::div8) {
            return 11500000000ull;
        }
        return 15000000000ull;
    }
    auto get_channel_divider(uint64_t out_frequency, double pd_frequency) const
    {
        std::underlying_type_t<lmx2594_channel_divider> chdiv {};
        for (const auto channel_divider : lmx2594_constants::actual_channel_divider_array) {
            const auto vco_frequency { out_frequency * channel_divider };
            if (vco_frequency < lmx2594_constants::vco_frequency::min) {
                ++chdiv;
                continue;
            }
            if (vco_frequency > get_vco_frequency_max()) {
                throw std::out_of_range("lmx2594::find_channel_divider: vco_frequency out of range");
            }
            if (vco_frequency / pd_frequency >= get_n_divider_min(vco_frequency)) {
                break;
            }
            ++chdiv;
        }
        return lmx2594_channel_divider(chdiv);
    }
    void set_vco_assist(uint64_t vco_frequency) const
    {
        // TODO:
    }
    auto find_num_denom(double alfa, uint32_t& numerator, uint32_t& denomerator) const
    {
        if (alfa < 1. && alfa > 0.) {
            uint64_t numerator_prev {};
            uint64_t denomerator_prev { 1 };
            uint64_t numerator_last { 1 };
            uint64_t denomerator_last { uint64_t(1. / alfa) };
            double epsilon { 1 / alfa - denomerator_last };
            const double zero { 1e-11 };
            while (epsilon > zero) {
                const auto a = uint64_t(1 / epsilon);
                const auto denomerator_tmp = denomerator_last * a + denomerator_prev;
                if (denomerator_tmp >= lmx2594_constants::fraction::denominator::max) {
                    break;
                }
                const auto numerator_tmp = numerator_last * a + numerator_prev;
                epsilon = 1. / epsilon - a;
                numerator_prev = numerator_last;
                numerator_last = numerator_tmp;
                denomerator_prev = denomerator_last;
                denomerator_last = denomerator_tmp;
            }
            numerator = uint32_t(numerator_last);
            denomerator = uint32_t(denomerator_last);
            return true;
        }
        return false;
    }
    void set_frequency(const lmx2594_output_frequency& data) const
    {
#if defined(CHAPPI_LOG_ENABLE)
        log_info(__func__);
#endif
        using namespace lmx2594_registers;
        const auto out_frequency { static_cast<uint64_t>(data.frequency + 0.5) };
        const auto osc_frequency { static_cast<uint64_t>(data.reference + 0.5) };
        set_doubler(lmx2594_doubler::disabled);
        set_pre_divider(1);
        set_multiplier(lmx2594_multiplier::bypass);
        set_divider(1);
        set_mash_order(lmx2594_mash_order::frac3);
        _is_integer_mode = false;
        set_vco_calibration_divider(uint64_t(data.reference));
        set_channel_divider(lmx2594_channel_divider::div2);
        if (out_frequency > lmx2594_constants::out_frequency::max || out_frequency < lmx2594_constants::out_frequency::min) {
            throw std::out_of_range("lmx2594::set_frequency: out_frequency out of range");
        }
        if (osc_frequency < lmx2594_constants::osc_frequency::min || osc_frequency > get_osc_frequency_max()) {
            throw std::out_of_range("lmx2594::set_frequency: osc_frequency out of range");
        }
        double osc_frequency_ratio = register_to_integer<register_type>(_registers_map.regs.reg_R10.bits.MULT)
            * (register_to_integer<register_type>(_registers_map.regs.reg_R9.bits.OSC_2X) + 1)
            / double(_registers_map.regs.reg_R11.bits.PLL_R)
            / double(_registers_map.regs.reg_R12.bits.PLL_R_PRE);
        double pd_frequency = osc_frequency * osc_frequency_ratio;
        if (pd_frequency > get_pd_frequency_max() || pd_frequency < get_pd_frequency_min()) {
            throw std::out_of_range("lmx2594::set_frequency: pd_frequency out of range");
        }
        auto out_a_mux { OUTA_MUX_type::chdiv };
        auto out_b_mux { OUTB_MUX_type::chdiv };
        decltype(get_channel_divider({}, {})) channel_divider {};
        decltype(get_actual_channel_divider({})) actual_channel_divider {};
        try {
            channel_divider = get_channel_divider(out_frequency, uint64_t(pd_frequency));
            actual_channel_divider = get_actual_channel_divider(channel_divider);
            set_channel_divider(channel_divider);
        } catch (const std::out_of_range& e) {
            out_a_mux = OUTA_MUX_type::vco;
            out_b_mux = OUTB_MUX_type::vco;
            actual_channel_divider = 1;
        }
        const uint64_t vco_frequency = out_frequency * actual_channel_divider;
        if (vco_frequency < lmx2594_constants::vco_frequency::min || vco_frequency > get_vco_frequency_max()) {
            throw std::out_of_range("lmx2594::set_frequency: vco_frequency out of range");
        }
        const double divider = double(vco_frequency) / pd_frequency;
        const uint32_t n_divider = uint32_t(divider);
        uint32_t numerator {};
        uint32_t denomerator { lmx2594_constants::fraction::denominator::max };
        if (n_divider < get_n_divider_min(vco_frequency)) {
            throw std::out_of_range("lmx2594::set_frequency: n_divider out of range ");
        }
        const double num_denom_min = 1. / pd_frequency;
        if (((divider - n_divider) < num_denom_min)) {
            _is_integer_mode = true;
            set_mash_order(lmx2594_mash_order::integer);
            numerator = 0;
        } else if (!find_num_denom(divider - n_divider, numerator, denomerator)) {
            numerator = uint32_t((divider - n_divider) * denomerator);
            throw std::out_of_range("lmx2594::set_frequency: find_num_denom out of range ");
        }
        numerator *= lmx2594_constants::fraction::denominator::max / denomerator;
        denomerator = denomerator * (lmx2594_constants::fraction::denominator::max / denomerator);
        set_n_divider(n_divider);
        set_fractional_numerator(numerator);
        set_fractional_denomerator(denomerator);
        if (data.output == lmx2594_output::outa) {
            set_output_mux(out_a_mux);
        }
        if (data.output == lmx2594_output::outb) {
            set_output_mux(out_b_mux);
        }
        set_phase_detector_delay(vco_frequency);
        set_high_pd_frequency_calibration(uint32_t(pd_frequency));
        set_low_pd_frequency_calibration(uint32_t(pd_frequency));
        set_charge_pump_gain(lmx2594_charge_pump_gain::current_15_mA);
        update_changes();
        vco_calibrate();
        if (!wait_lock_detect()) {
            throw std::runtime_error("lmx2594::set_frequency: not locked!");
        }
        update_charge_pump_gain(lmx2594_charge_pump_gain::current_6_mA);
#if defined(CHAPPI_LOG_ENABLE)
        auto actual_out_frequency = uint64_t(std::round(pd_frequency * (n_divider + double(numerator) / double(denomerator))
            / double(actual_channel_divider)));
        log_info(std::string(32, '-'));
        log_info(std::string(13, '-') + " PLL " + std::string(14, '-'));
        log_info(std::string(32, '-'));
        if (_is_integer_mode) {
            log_info("integer mode");
        } else {
            log_info("fractional mode");
        }
        log_info("out_frequency (Hz) = " + std::to_string(out_frequency));
        log_info("actual_out_frequency (Hz) = " + std::to_string(actual_out_frequency));
        log_info("osc_frequency (Hz) = " + std::to_string(osc_frequency));
        log_info("vco_frequency (Hz) = " + std::to_string(vco_frequency));
        log_info("pd_frequency (Hz) = " + std::to_string(pd_frequency));
        if (actual_channel_divider > 1) {
            log_info("channel_divider = " + std::to_string(actual_channel_divider));
        }
        log_info("n_divider = " + std::to_string(n_divider));
        log_info("numerator = " + std::to_string(numerator));
        log_info("denomerator = " + std::to_string(denomerator));
        log_info(std::string(32, '-'));
#endif
    }
    bool is_integer_mode() const noexcept { return _is_integer_mode; }

private:
    template <typename Arg = int, typename... Args>
    void _update_registers(Arg register_num, Args... other_registers) const
    {
        write(register_num, _registers_map.array[register_num]);
        _update_registers(other_registers...);
    }
    void _update_registers() const { }
    auto _is_locked() const
    {
        using namespace lmx2594_registers;
        read(110, _registers_map.regs.reg_R110.reg);
        const auto locked = (_registers_map.regs.reg_R110.bits.rb_LD_VTUNE == rb_LD_VTUNE_type::locked) ? true : false;
        return locked;
    }
    void _set_output_enabled(const lmx2594_output_enable& data) const noexcept
    {
        using namespace lmx2594_registers;
        auto enabled { (data.enabled) ? OUT_PD_type::active : OUT_PD_type::powerdown };
        if (data.output == lmx2594_output::outa) {
            _registers_map.regs.reg_R44.bits.OUTA_PD = enabled;
        }
        if (data.output == lmx2594_output::outb) {
            _registers_map.regs.reg_R44.bits.OUTB_PD = enabled;
        }
    }
    void _set_output_power(const lmx2594_output_power& data) const noexcept
    {
        using namespace lmx2594_registers;
        if (data.output == lmx2594_output::outa) {
            _registers_map.regs.reg_R44.bits.OUTA_PWR = data.power;
        }
        if (data.output == lmx2594_output::outb) {
            _registers_map.regs.reg_R45.bits.OUTB_PWR = data.power;
        }
    }
    void _set_output_mux(const lmx2594_output_a_mux& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R45.bits.OUTA_MUX = value;
    }
    void _set_output_mux(const lmx2594_output_b_mux& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R46.bits.OUTB_MUX = value;
    }
    void _set_channel_divider(const lmx2594_channel_divider& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R75.bits.CHDIV = value;
        if (_registers_map.regs.reg_R75.bits.CHDIV > CHDIV_type::div2) {
            _registers_map.regs.reg_R31.bits.SEG1_EN = SEG1_EN_type::driver_buffer_enabled;
        } else {
            _registers_map.regs.reg_R31.bits.SEG1_EN = SEG1_EN_type::disabled;
        }
    }
    void _set_charge_pump_gain(const lmx2594_charge_pump_gain& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R14.bits.CPG = value;
    }
    void _set_doubler(const lmx2594_doubler& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R9.bits.OSC_2X = value;
    }
    void _set_pre_divider(const lmx2594_pre_divider& value) const
    {
        using namespace lmx2594_registers;
        using namespace lmx2594_constants;
        if (value < pre_divider::min || value > pre_divider::max) {
            throw std::invalid_argument("lmx2594::set_pre_divider: invalid argument");
        }
        _registers_map.regs.reg_R12.bits.PLL_R_PRE = value;
    }
    void _set_multiplier(const lmx2594_multiplier& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R10.bits.MULT = value;
    }
    void _set_divider(const lmx2594_divider& value) const
    {
        using namespace lmx2594_registers;
        using namespace lmx2594_constants;
        if (value < divider::min || value > divider::max) {
            throw std::invalid_argument("lmx2594::set_divider: invalid argument");
        }
        _registers_map.regs.reg_R11.bits.PLL_R = value;
    }
    void _set_n_divider(const lmx2594_n_divider& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R36.bits.PLL_N_15_0 = value & 0xFFFF;
        _registers_map.regs.reg_R34.bits.PLL_N_18_16 = (value >> 16) & 0x0007;
    }
    void _set_fractional_numerator(const lmx2594_fractional_numerator& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R43.bits.PLL_NUM_15_0 = value & 0xFFFF;
        _registers_map.regs.reg_R42.bits.PLL_NUM_31_16 = (value >> 16) & 0xFFFF;
    }
    void _set_fractional_denomerator(const lmx2594_fractional_denomerator& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R39.bits.PLL_DEN_15_0 = value & 0xFFFF;
        _registers_map.regs.reg_R38.bits.PLL_DEN_31_16 = (value >> 16) & 0xFFFF;
    }
    void _set_lock_detect(const lmx2594_lock_detect& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R59.bits.LD_TYPE = value;
    }
    void _set_lock_detect_mux(const lmx2594_lock_detect_mux& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R0.bits.MUXOUT_LD_SEL = value;
    }
    void _set_phase_detector_delay(uint64_t vco_frequency) const
    {
        using namespace lmx2594_registers;
        if (vco_frequency < lmx2594_constants::vco_frequency::min || vco_frequency > get_vco_frequency_max()) {
            throw std::invalid_argument("lmx2594::set_phase_detector_delay: invalid argument");
        }
        switch (_registers_map.regs.reg_R44.bits.MASH_ORDER) {
        case MASH_ORDER_type::integer:
            if (vco_frequency > 12500000000ull) {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 2;
            } else {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 1;
            }
            break;
        case MASH_ORDER_type::frac1:
            if (vco_frequency > 12500000000ull) {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 3;
            } else if (vco_frequency > 10000000000ull) {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 2;
            } else {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 1;
            }
            break;
        case MASH_ORDER_type::frac2:
            if (vco_frequency > 10000000000ull) {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 3;
            } else {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 2;
            }
            break;
        case MASH_ORDER_type::frac3:
            if (vco_frequency > 10000000000ull) {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 4;
            } else {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 3;
            }
            break;
        case MASH_ORDER_type::frac4:
            if (vco_frequency > 10000000000ull) {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 6;
            } else {
                _registers_map.regs.reg_R37.bits.PFD_DLY_SEL = 5;
            }
            break;
        default:
            throw std::invalid_argument("lmx2594::set_phase_detector_delay: invalid argument");
        }
    }
    void _set_vco_calibration_divider(uint64_t osc_frequency) const noexcept
    {
        using namespace lmx2594_registers;
        CAL_CLK_DIV_type divider { CAL_CLK_DIV_type::div1 };
        if (auto force_div8 = true) {
            divider = CAL_CLK_DIV_type::div8;
        } else if (osc_frequency > 800000000u) {
            divider = CAL_CLK_DIV_type::div8;
        } else if (osc_frequency > 400000000u) {
            divider = CAL_CLK_DIV_type::div4;
        } else if (osc_frequency > 200000000u) {
            divider = CAL_CLK_DIV_type::div2;
        }
        _registers_map.regs.reg_R1.bits.CAL_CLK_DIV = divider;
        const double smclk_frequency = osc_frequency / std::pow(2, register_to_integer<register_type>(divider));
        _registers_map.regs.reg_R4.bits.ACAL_CMP_DLY = register_type(std::ceil(smclk_frequency / 10000000.)) + 1;
    }
    void _set_mash_order(const lmx2594_mash_order& value) const noexcept
    {
        using namespace lmx2594_registers;
        _registers_map.regs.reg_R44.bits.MASH_ORDER = value;
    }
    void _set_high_pd_frequency_calibration(uint32_t pd_frequency) const noexcept
    {
        using namespace lmx2594_registers;
        if (pd_frequency > 200000000ul) {
            _registers_map.regs.reg_R0.bits.FCAL_HPFD_ADJ = FCAL_HPFD_ADJ_type::upper_200_MHz;
        } else if (pd_frequency > 150000000ul) {
            _registers_map.regs.reg_R0.bits.FCAL_HPFD_ADJ = FCAL_HPFD_ADJ_type::range_150_200_MHz;
        } else if (pd_frequency > 100000000ul) {
            _registers_map.regs.reg_R0.bits.FCAL_HPFD_ADJ = FCAL_HPFD_ADJ_type::range_100_150_MHz;
        } else {
            _registers_map.regs.reg_R0.bits.FCAL_HPFD_ADJ = FCAL_HPFD_ADJ_type::lower_100_MHz;
        }
    }
    void _set_low_pd_frequency_calibration(uint32_t pd_frequency) const noexcept
    {
        using namespace lmx2594_registers;
        if (pd_frequency < 2500000ul) {
            _registers_map.regs.reg_R0.bits.FCAL_LPFD_ADJ = FCAL_LPFD_ADJ_type::lower_2p5_MHz;
        } else if (pd_frequency < 5000000ul) {
            _registers_map.regs.reg_R0.bits.FCAL_LPFD_ADJ = FCAL_LPFD_ADJ_type::range_2p5_5_MHz;
        } else if (pd_frequency < 10000000ul) {
            _registers_map.regs.reg_R0.bits.FCAL_LPFD_ADJ = FCAL_LPFD_ADJ_type::range_5_10_MHz;
        } else {
            _registers_map.regs.reg_R0.bits.FCAL_LPFD_ADJ = FCAL_LPFD_ADJ_type::upper_10_MHz;
        }
    }
};

} // namespace chappi
