///
/// \file AD9176.h
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
/// \version 0.1
/// \date 20.05.2019
///
/// \copyright InSys Copyright (c) 2019
///

#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include <utility>

class AD9176 {
    static constexpr auto name_class = "AD9176"; // имя класса для отладочных сообщений

    using spi_read_t = std::function<uint8_t(std::size_t)>;
    using spi_write_t = std::function<void(std::size_t, uint8_t)>;

    // функции SPI чтения/записи регистров
    spi_read_t spi_read;
    spi_write_t spi_write;

    double freq_clk = 0.0; // частота тактирования ЦАП в МГц
    std::size_t freq_mult = 0; // коэффициент умножения входной частоты ЦАП
    double sample_rate = 0.0; // частота оцифрованного потока в МГц, с учётом интерполяции
    double lane_rate = 0.0; // частота потока на линиях JESD'а

public:
    AD9176();
    ~AD9176() = default;

    auto setup_spi(spi_read_t r, spi_write_t w) -> void
    {
        spi_read = r;
        spi_write = w;
    };

    auto reset() -> void;

    using ChipID = struct {
        uint8_t chip_type; ///< Chip Type Code
        uint16_t prod_id; ///< Product ID Code
        uint8_t prod_grade; ///< Product Grade Code
        uint8_t dev_revision; ///< Device Revision
        uint32_t sn; ///< Serial Number
    };
    auto get_chip_id() -> ChipID;

    auto pll_setup() -> bool;
    auto set_fclk(double value, std::size_t mult) -> void
    {
        freq_mult = mult;
        freq_clk = value * mult;
    }
    auto get_fclk() -> double { return freq_clk; }
    auto get_sample_rate() -> double { return sample_rate; }
    auto get_lane_rate() -> double { return lane_rate; }

    auto jesd_mode_setup(bool dual, uint8_t mode, uint8_t ch_mode, uint8_t dp_mode) -> bool;
    auto jesd_serdes_setup(uint8_t lanes, bool calibration) -> bool;
    auto jesd_crossbar_setup() -> void;
    auto jesd_links_enable() -> bool;
    auto jesd_get_links_status() -> std::pair<std::array<uint8_t, 4>, std::array<uint8_t, 4>>;
    auto jesd_get_links_status_print() -> void;
    auto jesd_set_lmfc(std::size_t delay, std::size_t variable) -> void;
    auto jesd_get_lmfc() -> std::size_t;

    using JESD_Info = struct {
        std::size_t L; ///< Количество используемых линий
        std::size_t F; ///< Количество октетов в фрейме на линию
        std::size_t K; ///< Количество фреймов в мултифрейме
    };
    auto jesd_get_info() -> JESD_Info;

    auto digital_gain_setup() -> void;
    auto nco_channel_setup(std::size_t chan, double freq, double phase) -> void;
    auto nco_main_setup(std::size_t chan, double freq, double phase) -> void;

    auto dds_enable(bool enable) -> void;
    auto dds_amplitude(std::size_t chan, uint16_t ampl) -> void;

    auto enable_calibration() -> void;
    auto enable_tx() -> void;

    auto set_digital_gain(std::size_t chan, double gain) -> void;

    auto blob() -> void;
    auto blob_work() -> void;

// -------------------------------------------------------------------------
// Регистры
// -------------------------------------------------------------------------
#define AD9176_REG(name, addr, ...)    \
    static constexpr auto name = addr; \
    using name##_VALUE = union {       \
        uint8_t all;                   \
        struct {                       \
            uint8_t __VA_ARGS__;       \
        };                             \
    }

    // Reset
    static constexpr auto SPI_INTFCONFA = 0x000;
    static constexpr auto DAC_POWERDOWN = 0x090;
    static constexpr auto ACLK_CTRL = 0x091;
    static constexpr auto CDR_RESET = 0x206;
    AD9176_REG(NVM_LOADER_EN, 0x705, NVM_BLR_EN : 1, NVM_BLR_DONE : 1);

    // Chip ID, SN
    static constexpr auto SPI_CHIPTYPE = 0x003;
    static constexpr auto SPI_PRODIDL = 0x004;
    static constexpr auto SPI_PRODIDH = 0x005;
    AD9176_REG(SPI_CHIPGRADE, 0x006, DEV_REVISION : 4, PROD_GRADE : 2);
    static constexpr auto CHIP_ID_L = 0x010;
    static constexpr auto CHIP_ID_M1 = 0x011;
    static constexpr auto CHIP_ID_M2 = 0x012;
    static constexpr auto CHIP_ID_H = 0x013;

    // JESD's
    AD9176_REG(SYSREF_MODE, 0x03A, : 1, SYSREF_MODE_ONESHOT : 1, : 2, SYNC_ROTATION_DONE : 1);
    AD9176_REG(ROTATION_MODE, 0x03B, ROTATION_MODE : 2, : 2, NCORST_AFTER_ROT_EN : 1, PERIODIC_RST_EN : 1, WRITE_1 : 1, SYNCLOGIC_EN : 1);
    // AD9176_REG(SYSREF_CTRL,        0x084, SYSREF_PD:1, :5, SYSREF_INPUTMODE:1);
    using SYSREF_CTRL_0x084 = union {
        uint8_t all;
        struct {
            uint8_t SYSREF_PD : 1, : 5, SYSREF_INPUTMODE : 1;
        };
    };
    AD9176_REG(DIG_RESET, 0x100, DIG_DATAPATH_PD : 1);
    AD9176_REG(JESD_MODE, 0x110, JESD_MODE : 5, JESD_MODE_DUAL : 1, COM_SYNC : 1, MODE_NOT_IN_TABLE : 1);
    AD9176_REG(INTRP_MODE, 0x111, CH_INTERP_MODE : 4, DP_INTERP_MODE : 4);
    AD9176_REG(GENERAL_JRX_CTRL_0, 0x300, LINK_EN_0 : 1, LINK_EN_1 : 1, LINK_PAGE : 1, LINK_MODE : 1);
    AD9176_REG(ILS_DID, 0x450, DID : 8);
    AD9176_REG(ILS_BID, 0x451, BID : 8);
    AD9176_REG(ILS_LID0, 0x452, LID0 : 5, PHADJ : 1, ADJDIR : 1);
    AD9176_REG(ILS_SCR_L, 0x453, L_1 : 5, : 2, SCR : 1);
    AD9176_REG(ILS_F, 0x454, F_1 : 8);
    AD9176_REG(ILS_K, 0x455, K_1 : 5);
    AD9176_REG(ILS_M, 0x456, M_1 : 8);
    AD9176_REG(ILS_CS_N, 0x457, N_1 : 5, : 1, CS : 2);
    AD9176_REG(ILS_NP, 0x458, NP_1 : 5, SUBCLASSV : 3);
    AD9176_REG(ILS_S, 0x459, S_1 : 5, JESDV : 3);
    AD9176_REG(ILS_HD_CF, 0x45A, CF : 5, : 2, HD : 1);

    static constexpr auto CODE_GRP_SYNC = 0x470;
    static constexpr auto FRAME_SYNC = 0x471;
    static constexpr auto GOOD_CHECKSUM = 0x472;
    static constexpr auto INIT_LANE_SYNC = 0x473;
    AD9176_REG(CTRLREG0, 0x475, REPL_FRM_ENA : 1, : 1, FORCESYNCREQ : 1, SOFTRST : 1);
    static constexpr auto LINK_STATUS0 = 0x4B0;
    static constexpr auto LINK_STATUS1 = 0x4B1;
    static constexpr auto LINK_STATUS2 = 0x4B2;
    static constexpr auto LINK_STATUS3 = 0x4B3;
    static constexpr auto LINK_STATUS4 = 0x4B4;
    static constexpr auto LINK_STATUS5 = 0x4B5;
    static constexpr auto LINK_STATUS6 = 0x4B6;
    static constexpr auto LINK_STATUS7 = 0x4B7;
};
