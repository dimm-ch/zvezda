#pragma once

#include <type_traits>

// #include "../../core/bardy/brdlib/adm2if.h"
#include "brdsafe.h"
#include "chappi.h"

namespace InSys {
namespace Bardy {
    namespace Registers {
        namespace Fmc146v {
            namespace Detail {
                /// @brief  Командные регистры
                enum class REGnr_CMD : uint32_t {
                    MODE0 = 0x00, ///< Регистр управления MODE0
                    FMODE = 0x03, ///< Регистр управления FMODE
                    DAC = 0x04, ///< Регистр управления DAC – Управление ЦАП AD5621
                    MUX = 0x05, ///< Регистр управления MUX
                    FSRC = 0x06, ///< Регистр управления SY58040 – D46
                    FSRC3 = 0x07 ///< Регистр управления SY58040 – D48
                };
                enum class STATUS_RegBitMask : Register16BitType {
                    CMD_RDY = REGISTER_BITMASK_BIT(0),
                    LMX_RDY = REGISTER_BITMASK_BIT(1),
                    DAC_RDY = REGISTER_BITMASK_BIT(2),
                    LTC_RDY = REGISTER_BITMASK_BIT(3),
                    ALARM = REGISTER_BITMASK_BIT(4),
                    OT = REGISTER_BITMASK_BIT(5),
                    OSC_MUX = REGISTER_BITMASK_BIT(6),
                    LTC6953_ST = REGISTER_BITMASK_BIT(7),
                    SY8040_RDY = REGISTER_BITMASK_BIT(8),
                    SY8040_RDY3 = REGISTER_BITMASK_BIT(9),
                    AllBits = (CMD_RDY | LMX_RDY | DAC_RDY | LTC_RDY | ALARM | OT | OSC_MUX
                        | LTC6953_ST | SY8040_RDY | SY8040_RDY3)
                };
                enum class STATUS_RegBitMaskDefault : Register16BitType {
                    CMD_RDY = REGISTER_BITMASK_DEFAULT_FALSE(0),
                    LMX_RDY = REGISTER_BITMASK_DEFAULT_FALSE(1),
                    DAC_RDY = REGISTER_BITMASK_DEFAULT_FALSE(2),
                    LTC_RDY = REGISTER_BITMASK_DEFAULT_FALSE(3),
                    ALARM = REGISTER_BITMASK_DEFAULT_FALSE(4),
                    OT = REGISTER_BITMASK_DEFAULT_FALSE(5),
                    OSC_MUX = REGISTER_BITMASK_DEFAULT_FALSE(6),
                    LTC6953_ST = REGISTER_BITMASK_DEFAULT_FALSE(7),
                    SY8040_RDY = REGISTER_BITMASK_DEFAULT_FALSE(8),
                    SY8040_RDY3 = REGISTER_BITMASK_DEFAULT_FALSE(9),
                    AllBits = (CMD_RDY | LMX_RDY | DAC_RDY | LTC_RDY | ALARM | OT | OSC_MUX | LTC6953_ST
                        | SY8040_RDY | SY8040_RDY3)
                };
                enum class FMODE_RegBitMask : Register16BitType {
                    ZYNQ_PWR = REGISTER_BITMASK_BIT(0),
                    OCS_CE = REGISTER_BITMASK_BIT(1),
                    LTC6953_CE = REGISTER_BITMASK_BIT(2),
                    EXTREF = REGISTER_BITMASK_BIT(3),
                    EXT_EN_N = REGISTER_BITMASK_BIT(4),
                    GEN_EN = REGISTER_BITMASK_BIT(5),
                    AllBits = (ZYNQ_PWR | OCS_CE | LTC6953_CE | EXTREF | EXT_EN_N | GEN_EN)
                };
                enum class FMODE_RegBitMaskDefault : Register16BitType {
                    ZYNQ_PWR = REGISTER_BITMASK_DEFAULT_TRUE(0),
                    OCS_CE = REGISTER_BITMASK_DEFAULT_FALSE(1),
                    LTC6953_CE = REGISTER_BITMASK_DEFAULT_FALSE(2),
                    EXTREF = REGISTER_BITMASK_DEFAULT_FALSE(3),
                    EXT_EN_N = REGISTER_BITMASK_DEFAULT_FALSE(4),
                    GEN_EN = REGISTER_BITMASK_DEFAULT_FALSE(5),
                    AllBits = (ZYNQ_PWR | OCS_CE | LTC6953_CE | EXTREF | EXT_EN_N | GEN_EN)
                };
                enum class DAC_RegBitMask : Register16BitType {
                    DAC_DATA = REGISTER_BITMASK_BITS(12, 2),
                    DAC_PD = REGISTER_BITMASK_BITS(2, 14),
                    AllBits = (DAC_DATA | DAC_PD)
                };
                enum class DAC_RegBitMaskDefault : Register16BitType {
                    DAC_DATA = REGISTER_BITMASK_DEFAULT(0, 14),
                    DAC_PD = REGISTER_BITMASK_DEFAULT(0, 2),
                    AllBits = (DAC_DATA | DAC_PD)
                };
                enum class MUX_RegBitMask : Register16BitType {
                    FMC1_MGTCLK_MUX = REGISTER_BITMASK_BIT(2),
                    FMC2_MGTCLK_MUX = REGISTER_BITMASK_BIT(3),
                    MUX1_LF_CLK = REGISTER_BITMASK_BIT(4),
                    MUX2_LF_CLK = REGISTER_BITMASK_BIT(5),
                    MUX3_LF_CLK = REGISTER_BITMASK_BIT(6),
                    MUX4_LF_CLK = REGISTER_BITMASK_BIT(7),
                    MUX1_HF_CLK = REGISTER_BITMASK_BIT(8),
                    MUX3_HF_CLK = REGISTER_BITMASK_BIT(9),
                    MUX4_HF_CLK = REGISTER_BITMASK_BIT(10),
                    GAIN1_CLK_EN = REGISTER_BITMASK_BIT(11),
                    GAIN2_CLK_EN = REGISTER_BITMASK_BIT(12),
                    GAIN3_CLK_EN = REGISTER_BITMASK_BIT(13),
                    AllBits = (FMC1_MGTCLK_MUX | FMC2_MGTCLK_MUX
                        | MUX1_LF_CLK | MUX2_LF_CLK | MUX3_LF_CLK | MUX4_LF_CLK
                        | MUX1_HF_CLK | MUX3_HF_CLK | MUX4_HF_CLK
                        | GAIN1_CLK_EN | GAIN2_CLK_EN | GAIN3_CLK_EN)
                };
                enum class MUX_RegBitMaskDefault : Register16BitType {
                    FMC1_MGTCLK_MUX = REGISTER_BITMASK_DEFAULT_FALSE(2),
                    FMC2_MGTCLK_MUX = REGISTER_BITMASK_DEFAULT_FALSE(3),
                    MUX1_LF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(4),
                    MUX2_LF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(5),
                    MUX3_LF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(6),
                    MUX4_LF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(7),
                    MUX1_HF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(8),
                    MUX3_HF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(9),
                    MUX4_HF_CLK = REGISTER_BITMASK_DEFAULT_FALSE(10),
                    GAIN1_CLK_EN = REGISTER_BITMASK_DEFAULT_FALSE(11),
                    GAIN2_CLK_EN = REGISTER_BITMASK_DEFAULT_FALSE(12),
                    GAIN3_CLK_EN = REGISTER_BITMASK_DEFAULT_FALSE(13),
                    AllBits = (FMC1_MGTCLK_MUX | FMC2_MGTCLK_MUX
                        | MUX1_LF_CLK | MUX2_LF_CLK | MUX3_LF_CLK | MUX4_LF_CLK
                        | MUX1_HF_CLK | MUX3_HF_CLK | MUX4_HF_CLK
                        | GAIN1_CLK_EN | GAIN2_CLK_EN | GAIN3_CLK_EN)
                };
                enum class FSRC_RegBitMask : Register16BitType {
                    FSRC0 = REGISTER_BITMASK_BIT(0),
                    FSRC1 = REGISTER_BITMASK_BIT(1),
                    FSRC2 = REGISTER_BITMASK_BIT(2),
                    FSRC3 = REGISTER_BITMASK_BIT(3),
                    FSRC4 = REGISTER_BITMASK_BIT(4),
                    FSRC5 = REGISTER_BITMASK_BIT(5),
                    FSRC6 = REGISTER_BITMASK_BIT(6),
                    FSRC7 = REGISTER_BITMASK_BIT(7),
                    AllBits = (FSRC0 | FSRC1 | FSRC2 | FSRC3 | FSRC4 | FSRC5 | FSRC6 | FSRC7)
                };
                enum class FSRC_RegBitMaskDefault : Register16BitType {
                    FSRC0 = REGISTER_BITMASK_DEFAULT_FALSE(0),
                    FSRC1 = REGISTER_BITMASK_DEFAULT_FALSE(1),
                    FSRC2 = REGISTER_BITMASK_DEFAULT_FALSE(2),
                    FSRC3 = REGISTER_BITMASK_DEFAULT_FALSE(3),
                    FSRC4 = REGISTER_BITMASK_DEFAULT_FALSE(4),
                    FSRC5 = REGISTER_BITMASK_DEFAULT_FALSE(5),
                    FSRC6 = REGISTER_BITMASK_DEFAULT_FALSE(6),
                    FSRC7 = REGISTER_BITMASK_DEFAULT_FALSE(7),
                    AllBits = (FSRC0 | FSRC1 | FSRC2 | FSRC3 | FSRC4 | FSRC5 | FSRC6 | FSRC7)
                };
                enum class FSRC3_RegBitMask : Register16BitType {
                    FSRC0 = REGISTER_BITMASK_BIT(0),
                    FSRC1 = REGISTER_BITMASK_BIT(1),
                    FSRC2 = REGISTER_BITMASK_BIT(2),
                    FSRC3 = REGISTER_BITMASK_BIT(3),
                    FSRC4 = REGISTER_BITMASK_BIT(4),
                    FSRC5 = REGISTER_BITMASK_BIT(5),
                    FSRC6 = REGISTER_BITMASK_BIT(6),
                    FSRC7 = REGISTER_BITMASK_BIT(7),
                    AllBits = (FSRC0 | FSRC1 | FSRC2 | FSRC3 | FSRC4 | FSRC5 | FSRC6 | FSRC7)
                };
                enum class FSRC3_RegBitMaskDefault : Register16BitType {
                    FSRC0 = REGISTER_BITMASK_DEFAULT_FALSE(0),
                    FSRC1 = REGISTER_BITMASK_DEFAULT_FALSE(1),
                    FSRC2 = REGISTER_BITMASK_DEFAULT_FALSE(2),
                    FSRC3 = REGISTER_BITMASK_DEFAULT_FALSE(3),
                    FSRC4 = REGISTER_BITMASK_DEFAULT_FALSE(4),
                    FSRC5 = REGISTER_BITMASK_DEFAULT_FALSE(5),
                    FSRC6 = REGISTER_BITMASK_DEFAULT_FALSE(6),
                    FSRC7 = REGISTER_BITMASK_DEFAULT_FALSE(7),
                    AllBits = (FSRC0 | FSRC1 | FSRC2 | FSRC3 | FSRC4 | FSRC5 | FSRC6 | FSRC7)
                };
#pragma pack(push, 1)
                struct STATUS_RegBits {
                    bool CMD_RDY : 1;
                    bool LMX_RDY : 1;
                    bool DAC_RDY : 1;
                    bool LTC_RDY : 1;
                    bool ALARM : 1;
                    bool OT : 1;
                    bool OSC_MUX : 1;
                    bool LTC6953_ST : 1;
                    bool SY8040_RDY : 1;
                    bool SY8040_RDY3 : 1;
                    const uint8_t _NOT_USED_9_16 : 6;
                };
                struct FMODE_RegBits {
                    bool ZYNQ_PWR : 1;
                    bool OCS_CE : 1;
                    bool LTC6953_CE : 1;
                    bool EXTREF : 1;
                    bool EXT_EN_N : 1;
                    bool GEN_EN : 1;
                    const uint8_t _NOT_USED_6_7 : 2;
                    const uint8_t _NOT_USED_8_16 : 8;
                };
                struct DAC_RegBits {
                    const uint16_t _NOT_USED_0_1 : 2;
                    uint16_t DAC_DATA : 12;
                    uint16_t DAC_PD : 2;
                };
                struct MUX_RegBits {
                    bool _NOT_USED_0 : 1;
                    bool _NOT_USED_1 : 1;
                    bool FMC1_MGTCLK_MUX : 1;
                    bool FMC2_MGTCLK_MUX : 1;
                    bool MUX1_LF_CLK : 1;
                    bool MUX2_LF_CLK : 1;
                    bool MUX3_LF_CLK : 1;
                    bool MUX4_LF_CLK : 1;
                    bool MUX1_HF_CLK : 1;
                    bool MUX3_HF_CLK : 1;
                    bool MUX4_HF_CLK : 1;
                    bool GAIN1_CLK_EN : 1;
                    bool GAIN2_CLK_EN : 1;
                    bool GAIN3_CLK_EN : 1;
                    bool _NOT_USED_14 : 1;
                    bool _NOT_USED_15 : 1;
                };
                struct FSRC_RegBits {
                    bool _0 : 1;
                    bool _1 : 1;
                    bool _2 : 1;
                    bool _3 : 1;
                    bool _4 : 1;
                    bool _5 : 1;
                    bool _6 : 1;
                    bool _7 : 1;
                    const uint8_t _NOT_USED_8_16 : 8;
                };
                struct FSRC3_RegBits {
                    bool _0 : 1;
                    bool _1 : 1;
                    bool _2 : 1;
                    bool _3 : 1;
                    bool _4 : 1;
                    bool _5 : 1;
                    bool _6 : 1;
                    bool _7 : 1;
                    const uint8_t _NOT_USED_8_16 : 8;
                };
#pragma pack(pop)
            }
            using FMODE_Reg = IndirectRegister16BitTypeAbstract<
                Detail::FMODE_RegBitMask, Detail::FMODE_RegBitMaskDefault,
                Detail::FMODE_RegBits, std::underlying_type<Detail::REGnr_CMD>::type,
                std::underlying_type<Detail::REGnr_CMD>::type(Detail::REGnr_CMD::FMODE)>;
            using STATUS_Reg = DirectRegister16BitTypeAbstract<
                Detail::STATUS_RegBitMask, Detail::STATUS_RegBitMaskDefault,
                Detail::STATUS_RegBits, std::underlying_type<Detail::REGnr_CMD>::type, ADM2IFnr_STATUS>;
            using DAC_Reg = IndirectRegister16BitTypeAbstract<
                Detail::DAC_RegBitMask, Detail::DAC_RegBitMaskDefault, Detail::DAC_RegBits,
                std::underlying_type<Detail::REGnr_CMD>::type,
                std::underlying_type<Detail::REGnr_CMD>::type(Detail::REGnr_CMD::DAC)>;
            using MUX_Reg = IndirectRegister16BitTypeAbstract<
                Detail::MUX_RegBitMask, Detail::MUX_RegBitMaskDefault, Detail::MUX_RegBits,
                std::underlying_type<Detail::REGnr_CMD>::type,
                std::underlying_type<Detail::REGnr_CMD>::type(Detail::REGnr_CMD::MUX)>;
            using FSRC_Reg = IndirectRegister16BitTypeAbstract<
                Detail::FSRC_RegBitMask, Detail::FSRC_RegBitMaskDefault,
                Detail::FSRC_RegBits, std::underlying_type<Detail::REGnr_CMD>::type,
                std::underlying_type<Detail::REGnr_CMD>::type(Detail::REGnr_CMD::FSRC)>;
            using FSRC3_Reg = IndirectRegister16BitTypeAbstract<
                Detail::FSRC3_RegBitMask, Detail::FSRC3_RegBitMaskDefault,
                Detail::FSRC3_RegBits, std::underlying_type<Detail::REGnr_CMD>::type,
                std::underlying_type<Detail::REGnr_CMD>::type(Detail::REGnr_CMD::FSRC3)>;
        }
    }
    namespace Detail {
        class CFmc146vSyncSpdDevices final {
        public:
            using error_type = int;
            static const error_type no_error_v { BRDerr_OK };
            using si57x = chappi::si57x<error_type, no_error_v>;
            using lmx2594 = chappi::lmx2594<error_type, no_error_v>;
            using ltc6953 = chappi::ltc6953<error_type, no_error_v>;
            CFmc146vSyncSpdDevices(bool logenable = true)
                : Si57x { true && logenable }
                , LMX2594 { true && logenable }
                , LTC6953 { true && logenable }
            {
            }
            si57x Si57x;
            lmx2594 LMX2594;
            ltc6953 LTC6953;
        };
    }
    using CSpdDevices = Detail::CFmc146vSyncSpdDevices;
    class CFmc146vSyncData final {
    public:
        CFmc146vSyncData() = default;
        CFmc146vSyncData(const CRegService& regService)
            : m_RegService { regService }
        {
        }
        CFmc146vSyncData(CRegService&& regService)
            : m_RegService { std::move(regService) }
        {
        }
        CRegService m_RegService {};
        CSpdDevices m_SpdDevices {};
    };
    class CFmc146vSync final {
        static constexpr CSpdDevices::si57x::dev_addr_type k_Si57x_ADDR { 0x55 };
        static constexpr CRegService::SpdDevType k_LMX2594_DEVICE { 1 };
        static constexpr CRegService::SpdDevType k_LTC6953_DEVICE { 2 };
        static constexpr CRegService::SpdDevType k_Si57x_DEVICE { 4 };
        std::shared_ptr<CFmc146vSyncData> d_ptr { std::make_shared<CFmc146vSyncData>() };
        void initIO()
        {
            if (!d_ptr->m_RegService.findTetr(TETR_ID)) {
                throw std::runtime_error { "Can't find tetrade!" };
            };
            CSpdDevices::si57x::reg_read_fn readSi57x =
                [&](CSpdDevices::si57x::dev_addr_type dev, uint8_t addr, uint8_t& value) -> int {
                value = d_ptr->m_RegService.readSpd(k_Si57x_DEVICE, dev, addr);
                return CSpdDevices::no_error_v;
            };
            CSpdDevices::si57x::reg_write_fn writeSi57x =
                [&](CSpdDevices::si57x::dev_addr_type dev, uint8_t addr, uint8_t value) -> int {
                d_ptr->m_RegService.writeSpd(k_Si57x_DEVICE, dev, addr, value);
                return CSpdDevices::no_error_v;
            };
            d_ptr->m_SpdDevices.Si57x.setup_io(readSi57x, writeSi57x, k_Si57x_ADDR);
            CSpdDevices::lmx2594::reg_read_fn readLMX2594 =
                [&](CSpdDevices::lmx2594::dev_addr_type dev, uint8_t addr, uint16_t& value) -> int {
                value = d_ptr->m_RegService.readSpd(k_LMX2594_DEVICE, dev, addr);
                return CSpdDevices::no_error_v;
            };
            CSpdDevices::lmx2594::reg_write_fn writeLMX2594 =
                [&](CSpdDevices::lmx2594::dev_addr_type dev, uint8_t addr, uint16_t value) -> int {
                d_ptr->m_RegService.writeSpd(k_LMX2594_DEVICE, dev, addr, value);
                return CSpdDevices::no_error_v;
            };
            d_ptr->m_SpdDevices.LMX2594.setup_io(readLMX2594, writeLMX2594);
            CSpdDevices::ltc6953::reg_read_fn readLTC6953 =
                [&](CSpdDevices::ltc6953::dev_addr_type dev, uint8_t addr, uint8_t& value) -> int {
                value = d_ptr->m_RegService.readSpd(k_LTC6953_DEVICE, dev, addr);
                return CSpdDevices::no_error_v;
            };
            CSpdDevices::ltc6953::reg_write_fn writeLTC6953 =
                [&](CSpdDevices::ltc6953::dev_addr_type dev, uint8_t addr, uint8_t value) -> int {
                d_ptr->m_RegService.writeSpd(k_LTC6953_DEVICE, dev, addr, value);
                return CSpdDevices::no_error_v;
            };
            d_ptr->m_SpdDevices.LTC6953.setup_io(readLTC6953, writeLTC6953);
        }

    public:
        static const uint32_t TETR_ID { 0x134 };
        CFmc146vSync() = delete;
        CFmc146vSync(const CRegService& regService)
            : d_ptr { std::make_shared<CFmc146vSyncData>(regService) }
        {
            initIO();
        }
        CFmc146vSync(CRegService&& regService)
            : d_ptr { std::make_shared<CFmc146vSyncData>(std::move(regService)) }
        {
            initIO();
        }
        template <typename Register>
        void getReg(Register& reg) const { d_ptr->m_RegService.getReg(reg); }
        template <typename Register>
        void setReg(const Register& reg) const { d_ptr->m_RegService.setReg(reg); }
        CSpdDevices* operator->() noexcept { return &d_ptr->m_SpdDevices; }
        auto getDeviceInfo() const noexcept { return d_ptr->m_RegService->getDeviceInfo(); }
        auto getServiceInfo() const noexcept { return d_ptr->m_RegService->getServiceInfo(); }
    };
}
enum class SyncMode {
    Synchronization,
    SysRef,
    Agregate
};
}

void startFmc(const std::string& inifile, S32 DevNum);
