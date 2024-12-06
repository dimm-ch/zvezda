///
/// \file sync_regs.h
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 13.01.2020
///
/// \copyright InSys Copyright (c) 2020
///
///

#pragma once

#include <cstdint>

#include "adm2if.h"
#include "register_type.h"

namespace InSys {

namespace Detail {

    enum REGnr_SPD {
        REGnr_SPD_DEVICE = 0x203,
        REGnr_SPD_CTRL = 0x204,
        REGnr_SPD_ADDR = 0x205,
        REGnr_SPD_DATAL = 0x206
    };

    enum REGnr_START1 : uint32_t {
        REGnr_START1_RISE_HI = 0x210,
        REGnr_START1_RISE_LOW = 0x211,
        REGnr_START1_FALL_HI = 0x212,
        REGnr_START1_FALL_LOW = 0x213,
        REGnr_START1_END_HI = 0x214,
        REGnr_START1_END_LOW = 0x215,
        REGnr_START1_NPULSE = 0x228
    };

    enum REGnr_START2 : uint32_t {
        REGnr_START2_RISE_HI = 0x216,
        REGnr_START2_RISE_LOW = 0x217,
        REGnr_START2_FALL_HI = 0x218,
        REGnr_START2_FALL_LOW = 0x219,
        REGnr_START2_END_HI = 0x21A,
        REGnr_START2_END_LOW = 0x21B,
        REGnr_START2_NPULSE = 0x229

    };

    enum REGnr_START3 : uint32_t {
        REGnr_START3_RISE_HI = 0x21C,
        REGnr_START3_RISE_LOW = 0x21D,
        REGnr_START3_FALL_HI = 0x21E,
        REGnr_START3_FALL_LOW = 0x21F,
        REGnr_START3_END_HI = 0x220,
        REGnr_START3_END_LOW = 0x221,
        REGnr_START3_NPULSE = 0x22A

    };

    enum REGnr_START4 : uint32_t {
        REGnr_START4_RISE_HI = 0x222,
        REGnr_START4_RISE_LOW = 0x223,
        REGnr_START4_FALL_HI = 0x224,
        REGnr_START4_FALL_LOW = 0x225,
        REGnr_START4_END_HI = 0x226,
        REGnr_START4_END_LOW = 0x227,
        REGnr_START4_NPULSE = 0x22B
    };

    enum REGnr_START : uint32_t { REGnr_START_INV_EN = 0x22C };

    enum class Adm2Ver_RegBitMask : Register16BitType { AllBits = UINT16_MAX };

    enum class Adm2Ver_RegBitMaskDefault : Register16BitType { AllBits = 0 };

    enum REGnr_ADM2 : uint32_t {
        REGnr_ADM2_MODE0 = 0x00,
        REGnr_ADM2_STMODE = 0x05
    };

    enum REGnr_CONST : uint32_t { REGnr_CONST_FMC_CLK = 0x108 };

    enum class Adm2Mode0_RegBitMask : Register32BitType {
        RESET = REGISTER_BITMASK_BIT(0),
        MASTER = REGISTER_BITMASK_BIT(4),
        START = REGISTER_BITMASK_BIT(5),
        AllBits = (RESET | MASTER | START)
    };

    enum class Adm2Mode0_RegBitMaskDefault : Register32BitType {
        RESET = REGISTER_BITMASK_DEFAULT_FALSE(0),
        MASTER = REGISTER_BITMASK_DEFAULT_TRUE(4),
        START = REGISTER_BITMASK_DEFAULT_FALSE(5),
        AllBits = (RESET | MASTER | START)
    };

    enum class Adm2StMode_RegBitMask : Register32BitType {
        MSTART = REGISTER_BITMASK_BITS(5, 0),
        START_INV = REGISTER_BITMASK_BIT(6),
        TRIGSTART = REGISTER_BITMASK_BIT(7),
        MSTOP = REGISTER_BITMASK_BITS(5, 8),
        STOP_INV = REGISTER_BITMASK_BIT(14),
        AllBits = (MSTART | START_INV | TRIGSTART | MSTOP | STOP_INV)
    };

    enum class Adm2StMode_RegBitMaskDefault : Register32BitType {
        MSTART = REGISTER_BITMASK_DEFAULT(0, 0),
        START_INV = REGISTER_BITMASK_DEFAULT_FALSE(6),
        TRIGSTART = REGISTER_BITMASK_DEFAULT_FALSE(7),
        MSTOP = REGISTER_BITMASK_DEFAULT(8, 0),
        STOP_INV = REGISTER_BITMASK_DEFAULT_FALSE(14),
        AllBits = (MSTART | START_INV | TRIGSTART | MSTOP | STOP_INV)
    };

    enum class StartInvEn_RegBitMask : Register16BitType {
        START1_EN = REGISTER_BITMASK_BIT(0),
        START2_EN = REGISTER_BITMASK_BIT(1),
        START3_EN = REGISTER_BITMASK_BIT(2),
        START4_EN = REGISTER_BITMASK_BIT(3),
        START1_INV = REGISTER_BITMASK_BIT(8),
        START2_INV = REGISTER_BITMASK_BIT(9),
        START3_INV = REGISTER_BITMASK_BIT(10),
        START4_INV = REGISTER_BITMASK_BIT(11),
        AllBits = (START1_EN | START2_EN | START3_EN | START4_EN | START1_INV | START2_INV | START3_INV | START4_INV)
    };

    enum class StartInvEn_RegBitMaskDefault : Register16BitType {
        START1_EN = REGISTER_BITMASK_DEFAULT_FALSE(0),
        START2_EN = REGISTER_BITMASK_DEFAULT_FALSE(1),
        START3_EN = REGISTER_BITMASK_DEFAULT_FALSE(2),
        START4_EN = REGISTER_BITMASK_DEFAULT_FALSE(3),
        START1_INV = REGISTER_BITMASK_DEFAULT_FALSE(8),
        START2_INV = REGISTER_BITMASK_DEFAULT_FALSE(9),
        START3_INV = REGISTER_BITMASK_DEFAULT_FALSE(10),
        START4_INV = REGISTER_BITMASK_DEFAULT_FALSE(11),
        AllBits = (START1_EN | START2_EN | START3_EN | START4_EN | START1_INV | START2_INV | START3_INV | START4_INV)
    };

    enum class Start_RegBitMask : Register16BitType {
        AllBits = REGISTER_BITMASK_BITS(16, 0)
    };

    enum class Start_RegBitMaskDefault : Register16BitType { AllBits = 0 };

    enum class FmcClk_RegBitMask : Register16BitType {
        AllBits = REGISTER_BITMASK_BITS(16, 0)
    };

    enum class FmcClk_RegBitMaskDefault : Register16BitType { AllBits = 0 };

#pragma pack(push, 1)

    struct Adm2Ver_RegBits {
        uint8_t MAJOR;
        uint8_t MINOR;
    };

    struct Adm2Mode0_RegBits {
        bool RESET : 1;
        const bool _NOT_USED_1 : 1;
        const bool _NOT_USED_2 : 1;
        const bool _NOT_USED_3 : 1;
        bool MASTER : 1;
        bool START : 1;
        const bool _NOT_USED_6 : 1;
        const bool _NOT_USED_7 : 1;
        const uint8_t _NOT_USED_8_15 : 8;
        const uint16_t _NOT_USED_16_31 : 16;
    };

    struct Adm2StMode_RegBits {
        uint8_t MSTART : 5;
        const bool _NOT_USED_5 : 1;
        bool START_INV : 1;
        bool TRIGSTART : 1;
        uint8_t MSTOP : 5;
        const bool _NOT_USED_13 : 1;
        bool STOP_INV : 1;
        const bool _NOT_USED_15 : 1;
        const uint16_t _NOT_USED_16_31 : 16;
    };

    struct StartInvEn_RegBits {
        bool START1_EN : 1;
        bool START2_EN : 1;
        bool START3_EN : 1;
        bool START4_EN : 1;
        const uint8_t _NOT_USED_4_7 : 4;
        bool START1_INV : 1;
        bool START2_INV : 1;
        bool START3_INV : 1;
        bool START4_INV : 1;
        const uint8_t _NOT_USED_12_15 : 4;
    };

    struct Start_RegBits {
        uint16_t value;
    };

    struct FmcClk_RegBits {
        uint16_t value;
    };

#pragma pack(pop)

} // namespace Detail

using Adm2Ver_Reg = Register16BitTypeAbstract<
    Detail::Adm2Ver_RegBitMask, Detail::Adm2Ver_RegBitMaskDefault,
    Detail::Adm2Ver_RegBits, Register16BitType, ADM2IFnr_VER>;

using Adm2Mode0_Reg = Register32BitTypeAbstract<
    Detail::Adm2Mode0_RegBitMask, Detail::Adm2Mode0_RegBitMaskDefault,
    Detail::Adm2Mode0_RegBits, std::underlying_type<Detail::REGnr_ADM2>::type,
    Detail::REGnr_ADM2_MODE0>;

using Adm2StMode_Reg = Register32BitTypeAbstract<
    Detail::Adm2StMode_RegBitMask, Detail::Adm2StMode_RegBitMaskDefault,
    Detail::Adm2StMode_RegBits, std::underlying_type<Detail::REGnr_ADM2>::type,
    Detail::REGnr_ADM2_STMODE>;

using StartInvEn_Reg = Register16BitTypeAbstract<
    Detail::StartInvEn_RegBitMask, Detail::StartInvEn_RegBitMaskDefault,
    Detail::StartInvEn_RegBits, std::underlying_type<Detail::REGnr_START>::type,
    Detail::REGnr_START_INV_EN>;

template <typename AddrType, AddrType AddrValue>
using Start_Reg = Register16BitTypeAbstract<
    Detail::Start_RegBitMask, Detail::Start_RegBitMaskDefault,
    Detail::Start_RegBits, typename std::underlying_type<AddrType>::type,
    AddrValue>;

using Start1RiseLow_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_RISE_LOW>;
using Start1RiseHi_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_RISE_HI>;
using Start1FallLow_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_FALL_LOW>;
using Start1FallHi_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_FALL_HI>;
using Start1EndLow_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_END_LOW>;
using Start1EndHi_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_END_HI>;
using Start1PulseCnt_Reg = Start_Reg<Detail::REGnr_START1, Detail::REGnr_START1_NPULSE>;

using Start2RiseLow_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_RISE_LOW>;
using Start2RiseHi_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_RISE_HI>;
using Start2FallLow_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_FALL_LOW>;
using Start2FallHi_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_FALL_HI>;
using Start2EndLow_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_END_LOW>;
using Start2EndHi_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_END_HI>;
using Start2PulseCnt_Reg = Start_Reg<Detail::REGnr_START2, Detail::REGnr_START2_NPULSE>;

using Start3RiseLow_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_RISE_LOW>;
using Start3RiseHi_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_RISE_HI>;
using Start3FallLow_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_FALL_LOW>;
using Start3FallHi_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_FALL_HI>;
using Start3EndLow_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_END_LOW>;
using Start3EndHi_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_END_HI>;
using Start3PulseCnt_Reg = Start_Reg<Detail::REGnr_START3, Detail::REGnr_START3_NPULSE>;

using Start4RiseLow_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_RISE_LOW>;
using Start4RiseHi_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_RISE_HI>;
using Start4FallLow_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_FALL_LOW>;
using Start4FallHi_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_FALL_HI>;
using Start4EndLow_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_END_LOW>;
using Start4EndHi_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_END_HI>;
using Start4PulseCnt_Reg = Start_Reg<Detail::REGnr_START4, Detail::REGnr_START4_NPULSE>;

using FmcClk_Reg = Register16BitTypeAbstract<
    Detail::FmcClk_RegBitMask, Detail::FmcClk_RegBitMaskDefault,
    Detail::FmcClk_RegBits, std::underlying_type<Detail::REGnr_CONST>::type,
    Detail::REGnr_CONST_FMC_CLK>;

} // namespace InSys
