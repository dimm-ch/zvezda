///
/// \file AD9176.cpp
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
/// \version 0.1
/// \date 26.06.2019
///
/// \copyright InSys Copyright (c) 2019
///

#include "AD9176.h"
#include <cmath>
#include <thread>
#include <utility>
#include <vector>

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

using namespace std::chrono_literals;

///
/// \brief      Конструктор с debug-версией функций SPI.
///
AD9176::AD9176()
    : spi_read([](std::size_t reg) { LOG("R [%03zX] : 0x%02X", reg, 0); return 0; })
    , spi_write([](std::size_t reg, uint8_t val) { LOG("W [%03zX] : 0x%02X", reg, val); })
{
}

///
/// \brief      Перезапуск чипа
///
/// \return     void
///
auto AD9176::reset() -> void
{
    LOG("Reset");
    spi_write(SPI_INTFCONFA, 0x81); // Soft reset
    spi_write(SPI_INTFCONFA, 0x3C); // Release reset and set to 4-wire SPI
    spi_write(ACLK_CTRL, 0x00); // Power Up Clock Receiver
    spi_write(CDR_RESET, 0x01); // Power Up Phys (Take PHYs out of reset)

    auto val = NVM_LOADER_EN_VALUE {};
    val.NVM_BLR_EN = true;
    spi_write(NVM_LOADER_EN, val.all); // Load NVRAM Factory Settings (Enable Boot Loader)
    spi_write(DAC_POWERDOWN, 0x00); // Power on DACs and Bias Circuitry

    std::this_thread::sleep_for(1ms); // NVRAM Reset Period = 1000 us

    val.all = spi_read(NVM_LOADER_EN);
    if (val.NVM_BLR_DONE != 1)
        LOG("ERROR: Init Sequence Fail");
}

///
/// \brief      Вывод ID чипа
///
/// \return     Структура ChipID { chip_type, prod_id, prod_grade, dev_revision,
///             sn }
///
auto AD9176::get_chip_id() -> ChipID
{
    auto chip_grade = SPI_CHIPGRADE_VALUE {};
    chip_grade.all = spi_read(SPI_CHIPGRADE);

    auto id = ChipID {
        .chip_type = uint8_t(spi_read(SPI_CHIPTYPE)),
        .prod_id = uint16_t((spi_read(SPI_PRODIDH) << 8) | spi_read(SPI_PRODIDL)),
        .prod_grade = uint8_t(chip_grade.PROD_GRADE),
        .dev_revision = uint8_t(chip_grade.DEV_REVISION),
        .sn = uint32_t((spi_read(CHIP_ID_H) << 24) | (spi_read(CHIP_ID_M2) << 16)
            | (spi_read(CHIP_ID_M1) << 8) | spi_read(CHIP_ID_L))
    };

    LOG("ChipINFO: type = %d, prod = %04Xh, grade = %d, revision = %d, sn = %d",
        id.chip_type, id.prod_id, id.prod_grade, id.dev_revision, id.sn);
    return id;
}

///
/// \brief      Установка внутреннего PLL чипа
///
/// \return     `true` при удачном завершении, `false` при ошибке
///
auto AD9176::pll_setup() -> bool
{
    std::vector<std::pair<std::size_t, uint8_t>> sequence {};

    // если коэффициент умножения входной частоты ЦАП равен 1, не используем PLL ЦАП'а
    if (freq_mult == 1) {
        LOG("DAC External PLL Setup:");
        sequence = { { 0x095, 0x01 }, { 0x790, 0xFF }, { 0x791, 0x1F } };
        for (auto& addr_val : sequence)
            spi_write(addr_val.first, addr_val.second);
        std::this_thread::sleep_for(100ms);

    } else // для других коэффициентов умножения используем PLL ЦАП'а
    {
        LOG("DAC Internal PLL Setup: %.2f MHz, Multiplication Factor: %zd", freq_clk, freq_mult);

        // Begin PLL Configuration
        sequence = {
            { 0x095, 0x00 }, { 0x790, 0x00 },
            { 0x791, 0x00 }, { 0x796, 0xE5 }, { 0x7A0, 0xBC }, { 0x794, 0x08 },
            { 0x797, 0x10 }, { 0x797, 0x20 }, { 0x798, 0x10 }, { 0x7A2, 0x7F }
        };
        for (auto& addr_val : sequence)
            spi_write(addr_val.first, addr_val.second);
        std::this_thread::sleep_for(100ms);

        // DAC PLL = 6'000 MHz | DAC REF = 3'000 MHz | M = 4
        // OutDiv = Register 0x094, Bits[1:0] + 1
        // Fdac = (8 × N × Fref  ) / M / OutDiv
        // Fdac = (8 × N × 3'000 ) / 4 / OutDiv
        auto found = false;
        std::size_t n_div = 0;
        std::size_t out_div = 0;
        double vco = 0.0;

        for (n_div = 2; n_div < 51; n_div++) {
            for (out_div = 1; out_div < 4; out_div++) {
                vco = (8 * n_div * (freq_clk / freq_mult)) / 4;
                if ((vco < 8'740.0) || (vco > 12'400.0))
                    continue; // VCO frequency (8.74 GHz to 12.4 GHz)
                if ((vco / out_div) == freq_clk) {
                    found = true;
                    break;
                }
            }
            if (found)
                break;
        }
        if (found == false) {
            LOG("- ERROR: DAC PLL dividers not found");
            return false;
        }
        LOG("- [N:%zd, Vco:%.2f, OutDiv:%zd] = %.2f MHz", n_div, vco, out_div, vco / out_div);

        // PLL Configuration
        sequence = {
            { 0x799, uint8_t(n_div) }, // N Divider
            { 0x08F, 0x01 }, // Power down CLKOUT
            { 0x793, 0x1B }, // M Divider = 4
            { 0x094, uint8_t(out_div - 1) }, // PLL Out Div
            { 0x792, 0x02 }, { 0x792, 0x00 }
        };
        for (auto& addr_val : sequence)
            spi_write(addr_val.first, addr_val.second);
        std::this_thread::sleep_for(100ms);

        // PLL Check Lock
        auto val = spi_read(0x7B5) & 1;
        LOG("- PLL Lock: %s", (val) ? "Ok" : "False");
        if (!val)
            return false;
    }

    // Delay Lock Loop (DLL) Configuration
    sequence = {
        { 0x0C0, 0x00 }, { 0x0DB, 0x00 }, { 0x0DB, 0x01 }, { 0x0DB, 0x00 },
        { 0x0C1, freq_clk < 4500 ? 0x48 : 0x68 },
        { 0x0C1, freq_clk < 4500 ? 0x49 : 0x69 },
        { 0x0C7, 0x01 }
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);

    // Check DLL PLL Lock
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1000);
    while ((spi_read(0x0C3) & 1) == 0 && timeout > std::chrono::steady_clock::now()) {
    };
    LOG("- DLL Lock: %s", (spi_read(0x0C3) & 1) ? "Ok" : "False");
    if ((spi_read(0x0C3) & 1) != 1)
        return false;

    return true;
}

///
/// \brief      Установка параметров JESD'а
///
/// \param      dual     Режим линка Single или Dual: true = Dual Link Mode;
///                      false = Single Link Mode
/// \param      mode     Номер режима JESD'а
/// \param      ch_mode  Режим интерполяции каналов
/// \param      dp_mode  Режим интерполяции выходных каналов
///
/// \return     `true` при удачном завершении, `false` при ошибке
///
auto AD9176::jesd_mode_setup(bool dual, uint8_t mode, uint8_t ch_mode, uint8_t dp_mode) -> bool
{
    // if (mode > 22) return false;
    // if ((ch_mode < 1 ) || (ch_mode > 8)) return false;
    // if ((dp_mode < 1 ) || (dp_mode > 12)) return false;

    LOG("JESD Setup Mode: %s %d x%d x%d", dual ? "Dual" : "Single", mode, ch_mode, dp_mode);

    // Отключаем линки перед настройкой
    {
        auto val = GENERAL_JRX_CTRL_0_VALUE {};
        val.all = spi_read(GENERAL_JRX_CTRL_0);
        val.LINK_EN_0 = false;
        val.LINK_EN_1 = false;
        spi_write(GENERAL_JRX_CTRL_0, val.all);
    }

    // Включим JESD
    spi_write(DIG_RESET, DIG_RESET_VALUE { .DIG_DATAPATH_PD = 0 }.all);

    // Установим режим JESD'а
    auto mode_val = JESD_MODE_VALUE {};
    mode_val.JESD_MODE = mode;
    mode_val.JESD_MODE_DUAL = dual;
    spi_write(JESD_MODE, mode_val.all);

    // Установим режим интерполяции всех каналов
    auto intrp_mode = INTRP_MODE_VALUE {};
    intrp_mode.DP_INTERP_MODE = dp_mode;
    intrp_mode.CH_INTERP_MODE = ch_mode;
    spi_write(INTRP_MODE, intrp_mode.all);

    // Проверим правильность конфигурации
    mode_val.all = spi_read(JESD_MODE);
    if (mode_val.MODE_NOT_IN_TABLE == 1) {
        LOG("- ERROR: Mode and Interpolation Mode combination is NOT Valid");
        return false;
    }

    // Расчёт частоты потока в МГц, с учётом интерполяции
    sample_rate = freq_clk / (dp_mode * ch_mode);
    auto jesd_M = std::size_t(ILS_M_VALUE { spi_read(ILS_M) }.M_1 + 1);
    auto jesd_L = std::size_t(ILS_SCR_L_VALUE { spi_read(ILS_SCR_L) }.L_1 + 1);
    lane_rate = (20 * sample_rate * jesd_M) / jesd_L;
    LOG("JESD Parameters: lane_rate = %.03f | M = %zd L = %zd",
        lane_rate, jesd_M, jesd_L);

    // Включим SYSREF
    // spi_write(SYSREF_CTRL, SYSREF_CTRL_VALUE { .SYSREF_PD = 0, .SYSREF_INPUTMODE = 1 } .all);
    auto sysref = SYSREF_CTRL_0x084 {};
    sysref.SYSREF_PD = 0;
    sysref.SYSREF_INPUTMODE = 1;
    spi_write(0x084, sysref.all);

    // SYNCOUT error duration ???
    // spi_write(0x312, 0x04);

    // Установка параметров JESD линков QBD0 и QBD1
    auto val = GENERAL_JRX_CTRL_0_VALUE {};
    val.all = spi_read(GENERAL_JRX_CTRL_0);
    for (auto page = 0; page < 2; page++) {
        val.LINK_PAGE = page;
        spi_write(GENERAL_JRX_CTRL_0, val.all);

        spi_write(CTRLREG0, 0x09); // Enable Soft Reset JESD

        auto ils = ILS_S_VALUE {};
        ils.all = spi_read(ILS_S);
        ils.JESDV = 1; // установим версию JESD204B
        spi_write(ILS_S, ils.all);

        auto ils_np = ILS_NP_VALUE {};
        ils_np.all = spi_read(ILS_NP);
        ils_np.SUBCLASSV = 1;
        spi_write(ILS_NP, ils_np.all);

        spi_write(CTRLREG0, 0x01); // Disable Soft Reset JESD

        // spi_write(GENERAL_JRX_CTRL_0, GENERAL_JRX_CTRL_0_VALUE { .LINK_MODE = 1, .LINK_PAGE = 0 } .all); // Dual-Link Mode, Disable Links
        // spi_write(CTRLREG0, CTRLREG0_VALUE { .REPL_FRM_ENA = 1, .SOFTRST = 1} .all); // Enable Soft Reset JESD
        // spi_write(ILS_SCR_L, ILS_SCR_L_VALUE { .SCR = 1, .L_1 = (2 - 1) } .all); // Scramble Enable, Number of lanes (L) - 1 | Table 15,16
        // spi_write(ILS_NP, ILS_NP_VALUE { .SUBCLASSV = 1, .NP_1 = (16 - 1) } .all); // Subclass 0, Numberof bits per sample (NP) - 1 | Table 15,16
        // spi_write(ILS_S, ILS_S_VALUE { .JESDV = 1, .S_1 = 0 } .all); // JESD Version = JESD204B, S - 1
        // spi_write(CTRLREG0, CTRLREG0_VALUE { .REPL_FRM_ENA = 1, .SOFTRST = 0} .all); // Disable Soft Reset JESD
    }

    return true;
}

///
/// \brief      Получение информация о выбранном режиме JESD
///
/// \return     Структура JESD_Info { L, F, K }
///
auto AD9176::jesd_get_info() -> JESD_Info
{
    // Информация о выбранном режиме (Table 30)
    // L − 1  | Number of lanes minus 1.                                | 0x453, Bits[4:0]
    // F − 1  | Number of ((octets per frame) per lane) minus 1.        | 0x454, Bits[7:0]
    // K − 1  | Number of frames per multiframe minus 1.                | 0x455, Bits[4:0]
    // M − 1  | Number of converters minus 1.                           | 0x456, Bits[7:0]
    // N − 1  | Converter bit resolution minus 1.                       | 0x457, Bits[4:0]
    // NP − 1 | Bit packing per sample minus 1.                         | 0x458, Bits[4:0]
    // S − 1  | Number of ((samples per converter) per frame) minus 1.  | 0x459, Bits[4:0]
    // HD     | High density format. Set to 1.                          | 0x45A, Bit 7
    // DID    | Device ID. Match the device ID sent by the transmitter. | 0x450, Bits[7:0]
    // BID    | Bank ID. Match the bank ID sent by the transmitter.     | 0x451, Bits[7:0]
    // LID0   | Lane ID for Lane 0. Match the Lane ID sent by the       | 0x452, Bits[4:0]
    //        | transmitter on Logical Lane 0.                          |
    // JESDV  | JESD204x version. Match the version sent by the         | 0x459, Bits[7:5]
    //        | transmitter (0x0 = JESD204A, 0x1 = JESD204B).

    auto val = GENERAL_JRX_CTRL_0_VALUE {};
    val.all = spi_read(GENERAL_JRX_CTRL_0);
    val.LINK_PAGE = 0;
    spi_write(GENERAL_JRX_CTRL_0, val.all);

    LOG("[%s] L=%d, M=%d, F=%d, S=%d, NP=%d, N=%d, K=%d, HD=%d",
        ILS_S_VALUE { spi_read(ILS_S) }.JESDV == 1 ? "JESD204B" : "JESD204A",
        ILS_SCR_L_VALUE { spi_read(ILS_SCR_L) }.L_1 + 1,
        ILS_M_VALUE { spi_read(ILS_M) }.M_1 + 1,
        ILS_F_VALUE { spi_read(ILS_F) }.F_1 + 1,
        ILS_S_VALUE { spi_read(ILS_S) }.S_1 + 1,
        ILS_NP_VALUE { spi_read(ILS_NP) }.NP_1 + 1,
        ILS_CS_N_VALUE { spi_read(ILS_CS_N) }.N_1 + 1,
        ILS_K_VALUE { spi_read(ILS_K) }.K_1 + 1,
        ILS_HD_CF_VALUE { spi_read(ILS_HD_CF) }.HD);

    return JESD_Info {
        std::size_t(ILS_SCR_L_VALUE { spi_read(ILS_SCR_L) }.L_1 + 1),
        std::size_t(ILS_F_VALUE { spi_read(ILS_F) }.F_1 + 1),
        std::size_t(ILS_K_VALUE { spi_read(ILS_K) }.K_1 + 1)
    };
}

///
/// \brief      Настройка SERDES линий
///
/// \param      lanes        Маска используемых линий
/// \param      calibration  Использовать калибровку SERDES PHY физических
///                          каналов
///
/// \return     `true` при удачном завершении, `false` при ошибке
///
auto AD9176::jesd_serdes_setup(uint8_t lanes, bool calibration) -> bool
{
    LOG("JESD SERDES Setup");
    std::vector<std::pair<std::size_t, uint8_t>> sequence {};

    sequence = {
        // {0x234, 0x69}, // !!! ONLY KIT !!! PHYx invert the bit polarity (default 0x66)
        // {0x240, 0xAA}, {0x241, 0xAA}, {0x242, 0x55}, {0x243, 0x55}, // EQ settings for =< 11 dB
        { 0x240, 0xFF }, { 0x241, 0xFF }, { 0x242, 0xFF }, { 0x243, 0xFF }, // EQ settings for > 11 dB

        { 0x244, 0x1F }, { 0x245, 0x1F }, { 0x246, 0x1F }, { 0x247, 0x1F }, { 0x248, 0x1F }, { 0x249, 0x1F }, { 0x24A, 0x1F }, { 0x24B, 0x1F },
        { 0x201, 0x00 }, { 0x203, 0x00 }, { 0x253, 0x01 }, { 0x254, 0x01 },
        { 0x210, 0x16 }, { 0x216, 0x05 },
        { 0x212, 0xFF }, { 0x212, 0x00 },
        { 0x200, 0x00 }, // включим все линии
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
    std::this_thread::sleep_for(100ms);
    sequence = {
        { 0x210, 0x86 }, { 0x216, 0x40 },
        { 0x213, 0x01 }, { 0x213, 0x00 }, { 0x210, 0x86 }, { 0x216, 0x00 }, { 0x213, 0x01 },
        { 0x213, 0x00 }, { 0x280, 0x03 }, { 0x280, 0x01 }, { 0x280, 0x05 }, { 0x280, 0x01 }
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
    std::this_thread::sleep_for(100ms);

    auto result = spi_read(0x281) & 1;
    LOG(" - SERDES PLL Lock: %s", result ? "Ok" : "False");
    if (result)
        return true;
    else
        return false;
}

auto AD9176::jesd_crossbar_setup() -> void
{
    // После Reset'а устанавливается прямой порядок линий
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x308, 0b001'000 }, // [5:3] SRC_LANE1, [2:0] SRC_LANE0
        { 0x309, 0b011'010 }, // [5:3] SRC_LANE3, [2:0] SRC_LANE2
        { 0x30A, 0b101'100 }, // [5:3] SRC_LANE5, [2:0] SRC_LANE4
        { 0x30B, 0b111'110 }, // [5:3] SRC_LANE7, [2:0] SRC_LANE6
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

///
/// \brief Получает информацию о статусах линий JESD'ов
///
auto AD9176::jesd_get_links_status() -> std::pair<std::array<uint8_t, 4>, std::array<uint8_t, 4>>
{
    auto val = GENERAL_JRX_CTRL_0_VALUE {};
    val.all = spi_read(GENERAL_JRX_CTRL_0);

    std::pair<std::array<uint8_t, 4>, std::array<uint8_t, 4>> status {};
    val.LINK_PAGE = 0;
    spi_write(GENERAL_JRX_CTRL_0, val.all);
    status.first = { spi_read(CODE_GRP_SYNC), spi_read(FRAME_SYNC), spi_read(GOOD_CHECKSUM), spi_read(INIT_LANE_SYNC) };

    val.LINK_PAGE = 1;
    spi_write(GENERAL_JRX_CTRL_0, val.all);
    status.second = { spi_read(CODE_GRP_SYNC), spi_read(FRAME_SYNC), spi_read(GOOD_CHECKSUM), spi_read(INIT_LANE_SYNC) };

    return status;
}

///
/// \brief Выводит информацию о статусах линий JESD'ов
///
auto AD9176::jesd_get_links_status_print() -> void
{
    auto val = GENERAL_JRX_CTRL_0_VALUE {};
    val.all = spi_read(GENERAL_JRX_CTRL_0);

    for (auto page = 0; page < 2; page++) {
        val.LINK_PAGE = page;
        spi_write(GENERAL_JRX_CTRL_0, val.all);

        LOG_WITHOUT_NEWLINE("JESD Status Link [%d]: ", page);
        for (auto link = 0; link < 8; link++)
            LOG_WITHOUT_NAMECLASS("%02X ", spi_read(LINK_STATUS0 + link));
        LOG_WITHOUT_NAMECLASS("\n");

        LOG("- CODE_GRP_SYNC: %02X, FRAME_SYNC_REG: %02X, GOOD_CHECKSUM_REG: %02X, INIT_LANE_SYNC_REG: %02X",
            spi_read(CODE_GRP_SYNC), spi_read(FRAME_SYNC), spi_read(GOOD_CHECKSUM), spi_read(INIT_LANE_SYNC));
    }
}

///
/// \brief      Запуск линий JESD'а
///
/// \return     `true` при удачном завершении, `false` при ошибке
///
auto AD9176::jesd_links_enable() -> bool
{
    LOG("JESD Enable");
    std::vector<std::pair<std::size_t, uint8_t>> sequence {};

    auto rotation_mode = ROTATION_MODE_VALUE {};
    rotation_mode.ROTATION_MODE = 1,
    rotation_mode.NCORST_AFTER_ROT_EN = 1,
    rotation_mode.PERIODIC_RST_EN = 1,
    rotation_mode.WRITE_1 = 1,
    rotation_mode.SYNCLOGIC_EN = 1,
    spi_write(ROTATION_MODE, rotation_mode.all);

    // Set up sync for one-shot sync mode
    auto sync = SYSREF_MODE_VALUE {};
    sync.SYSREF_MODE_ONESHOT = true;
    spi_write(SYSREF_MODE, sync.all);
    // <<< Send SYSREF signal for synchronization
    // sync.SYSREF_MODE_ONESHOT = false; // в описании написано, что в режим монитора возвращается сама, после SYSREF'а
    // spi_write(SYSREF_MODE, sync.all);

    // Включим линки
    auto val = GENERAL_JRX_CTRL_0_VALUE {};
    val.all = spi_read(GENERAL_JRX_CTRL_0);
    val.LINK_EN_0 = true;
    val.LINK_EN_1 = true;
    spi_write(GENERAL_JRX_CTRL_0, val.all);

    // Проверим синхронизацию JESD'ов (только для SubClass 0)
    // std::this_thread::sleep_for(100ms);
    // sync.all = spi_read(SYSREF_MODE);
    // LOG_WITHOUT_NAMECLASS("Sync %s\n", sync.SYNC_ROTATION_DONE ? "DONE" : "ERROR");
    // if (sync.SYNC_ROTATION_DONE) return true;
    // else return false;
    return true;
}

///
/// \brief      Запись значений для калибровки JESD'ов. Описание на стр.44.
///
/// \param      delay     значение LMFC Delay
/// \param      variable  значение LMFC Variable Delay
///
/// \return     void
///
auto AD9176::jesd_set_lmfc(std::size_t delay, std::size_t variable) -> void
{
    delay &= 0b111111;
    variable &= 0b111111;

    LOG("Set LMFC: 0x304,0x305 = %zd; 0x306,0x307 = %zd", delay, variable);
    spi_write(0x304, delay);
    spi_write(0x305, delay);
    spi_write(0x306, variable);
    spi_write(0x307, variable);
}

///
/// \brief      Получение значений латентности для калибровки JESD'ов. Описание
///             на стр.44.
///
/// \return     [7:0] - значение линка 0, [15:8] - значение линка 1
///
auto AD9176::jesd_get_lmfc() -> std::size_t
{
    // ожидаем сигнал SysRef
    auto sync = SYSREF_MODE_VALUE {};
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(5000);
    do {
        sync.all = spi_read(SYSREF_MODE);
    } while (sync.SYNC_ROTATION_DONE == false && timeout > std::chrono::steady_clock::now());

    std::this_thread::sleep_for(100ms);

    // читаем значения
    uint8_t link0 = spi_read(0x302) & 0b111111;
    uint8_t link1 = spi_read(0x303) & 0b111111;

    auto mode_val = JESD_MODE_VALUE {};
    mode_val.all = spi_read(JESD_MODE);
    if (mode_val.JESD_MODE_DUAL == false)
        link1 = link0;

    LOG("Get LMFC (%s): 0x302 = %d; 0x303 = %d", mode_val.JESD_MODE_DUAL ? "dual" : "single", link0, link1);

    return (link1 << 8) | link0;
}

///
/// \brief      Установка цифрового усиления
///
/// \return     void
///
auto AD9176::digital_gain_setup() -> void
{
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        // Digital Gain
        { 0x008, 0x01 }, { 0x146, 0x00 }, { 0x147, 0x08 }, // Channel 0 [Channel 0 of DAC0]
        { 0x008, 0x02 }, { 0x146, 0x00 }, { 0x147, 0x08 }, // Channel 1 [Channel 1 of DAC0]
        { 0x008, 0x04 }, { 0x146, 0x00 }, { 0x147, 0x08 }, // Channel 2 [Channel 2 of DAC0]

        { 0x008, 0x08 }, { 0x146, 0x00 }, { 0x147, 0x08 }, // Channel 3 [Channel 0 of DAC1]
        { 0x008, 0x10 }, { 0x146, 0x00 }, { 0x147, 0x08 }, // Channel 4 [Channel 1 of DAC1]
        { 0x008, 0x20 }, { 0x146, 0x00 }, { 0x147, 0x08 }, // Channel 5 [Channel 2 of DAC1]
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

///
/// \brief      Установка NCO's на выходах каналов DAC's
///
/// \param      chan   Номер канала Main
/// \param      freq   Частота в Гц
/// \param      phase  Начальная фаза в градусах
///
/// \return     void
///
auto AD9176::nco_channel_setup(std::size_t chan, double freq, double phase) -> void
{
    auto main_interp = INTRP_MODE_VALUE {};
    main_interp.all = spi_read(INTRP_MODE);
    auto DDSC_FTW = uint64_t(round((freq / ((freq_clk * 1'000'000) / main_interp.DP_INTERP_MODE)) * std::pow(2, 48)));
    auto DDSC_PHASE = uint16_t(round((phase / 180.) * std::pow(2, 15)));

    LOG("Channel NCO's Setup: CH%zd = %.0f Hz [%012zXh], Phase = %.0f [%04Xh]", chan, freq, DDSC_FTW, phase, DDSC_PHASE);

    auto page = uint8_t {};
    switch (chan) {
    case 0:
        page = 0b000001;
        break; // Ch0
    case 1:
        page = 0b000010;
        break; // Ch1
    case 2:
        page = 0b000100;
        break; // Ch2
    case 3:
        page = 0b001000;
        break; // Ch3
    case 4:
        page = 0b010000;
        break; // Ch4
    case 5:
        page = 0b100000;
        break; // Ch5
    }

    // Channel NCO's
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, page }, // Select Main NCO Page
        { 0x130, 0x40 },
        { 0x131, 0x00 }, // Disable Update NCO
        { 0x132, DDSC_FTW >> 0 & 0xFF }, { 0x133, DDSC_FTW >> 8 & 0xFF }, { 0x134, DDSC_FTW >> 16 & 0xFF },
        { 0x135, DDSC_FTW >> 24 & 0xFF }, { 0x136, DDSC_FTW >> 32 & 0xFF }, { 0x137, DDSC_FTW >> 40 & 0xFF },
        { 0x138, DDSC_PHASE >> 0 & 0xFF }, { 0x139, DDSC_PHASE >> 8 & 0xFF },
        { 0x131, 0x01 }, // Enable Update NCO
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

///
/// \brief      Установка NCO's на выходах DAC's && PA Protect
///
/// \param      chan   Номер канала Main
/// \param      freq   Частота в Гц
/// \param      phase  Начальная фаза в градусах
///
/// \return     void
///
auto AD9176::nco_main_setup(std::size_t chan, double freq, double phase) -> void
{
    auto DDSM_FTW = uint64_t(round((freq / (freq_clk * 1'000'000)) * std::pow(2, 48)));
    auto DDSM_PHASE = uint16_t(round((phase / 180.) * std::pow(2, 15)));

    LOG("Main NCO's Setup: DAC%zd = %.0f Hz [%012zXh], Phase = %.0f [%04Xh]", chan, freq, DDSM_FTW, phase, DDSM_PHASE);

    auto page = uint8_t {};
    switch (chan) {
    case 0:
        page = 0x40;
        break; // DAC0
    case 1:
        page = 0x80;
        break; // DAC1
    }

    // Main NCO's && PA Protect
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, page }, // Select Main NCO Page
        { 0x112, 0x09 },
        // {0x1E6, 0x00},
        { 0x113, 0x00 }, // Disable Update NCO
        { 0x114, DDSM_FTW >> 0 & 0xFF }, { 0x115, DDSM_FTW >> 8 & 0xFF }, { 0x116, DDSM_FTW >> 16 & 0xFF },
        { 0x117, DDSM_FTW >> 24 & 0xFF }, { 0x118, DDSM_FTW >> 32 & 0xFF }, { 0x119, DDSM_FTW >> 40 & 0xFF },
        { 0x11C, DDSM_PHASE >> 0 & 0xFF }, { 0x11D, DDSM_PHASE >> 8 & 0xFF },
        { 0x113, 0x01 }, // Enable Update NCO
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

auto AD9176::enable_tx() -> void
{
    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, 0b11000000 }, // Page all main DACs for TXEN control update
        // {0x596, 0x0C}, // SPI turn on TXENx feature
        { 0x596, 0x08 }, // SPI turn on TXENx feature
        { 0x599, 0x00 }, // Disable TX Datapath Flush
        { 0x03F, 0x30 }, // TX_ENABLE
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

auto AD9176::enable_calibration() -> void
{
    using CAL_STAT = struct {
        uint8_t CAL_FINISH : 1, CAL_FAIL_SEARCH : 1, CAL_ACTIVE : 1;
    };
    CAL_STAT stat_dac0 {};
    CAL_STAT stat_dac1 {};
    LOG("Calibration Starting ...");

    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, 0b11000000 }, // Select All Main Pages
        { 0x050, 0x2A },
        { 0x061, 0x68 },
        { 0x051, 0x82 },
        { 0x051, 0x83 },
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);

    LOG("- DAC0: Active  Fail   Finish | DAC1: Active  Fail   Finish");
    do {
        spi_write(0x008, 0b01000000);
        *reinterpret_cast<uint8_t*>(&stat_dac0) = spi_read(0x052);
        spi_write(0x008, 0b10000000);
        *reinterpret_cast<uint8_t*>(&stat_dac1) = spi_read(0x052);
        LOG("-       %-8s%-7s%-6s |       %-8s%-7s%-6s",
            stat_dac0.CAL_ACTIVE ? "true" : "false",
            stat_dac0.CAL_FAIL_SEARCH ? "true" : "false",
            stat_dac0.CAL_FINISH ? "true" : "false",
            stat_dac1.CAL_ACTIVE ? "true" : "false",
            stat_dac1.CAL_FAIL_SEARCH ? "true" : "false",
            stat_dac1.CAL_FINISH ? "true" : "false");
    } while (!stat_dac0.CAL_FINISH && !stat_dac1.CAL_FINISH);

    spi_write(0x081, 0x03); // Power Down Calibration CLocks
}

auto AD9176::dds_enable(bool enable) -> void
{
    LOG("DDS %s", enable ? "Enable" : "Disable");

    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        // {0x008, 0b11000000}, // Page All Main DACs
        // {0x1E6, enable ? 0x02 : 0x00},

        { 0x008, 0b00111111 }, // Page All Channel DACs
        { 0x130, enable ? 0x41 : 0x40 },
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

auto AD9176::dds_amplitude(std::size_t chan, uint16_t ampl) -> void
{
    LOG("DDS Amplitude Setup: ch%zd = %04Xh", chan, ampl);

    auto page = uint8_t {};
    switch (chan) {
    case 0:
        page = 0b000001;
        break; // Ch0
    case 1:
        page = 0b000010;
        break; // Ch1
    case 2:
        page = 0b000100;
        break; // Ch2
    case 3:
        page = 0b001000;
        break; // Ch3
    case 4:
        page = 0b010000;
        break; // Ch4
    case 5:
        page = 0b100000;
        break; // Ch5
    }

    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x8, page },
        { 0x148, ampl & 0xFF }, { 0x149, ampl >> 8 }, // амплитуда тестового сигнала
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

auto AD9176::set_digital_gain(std::size_t chan, double gain) -> void
{
    auto CHNL_GAIN = uint16_t(round(pow(10, gain / 20.) * pow(2, 11)));
    LOG("Channel Gain Setup: ch%zd = %04Xh (%.02f dB)", chan, CHNL_GAIN, gain);

    auto page = uint8_t {};
    switch (chan) {
    case 0:
        page = 0b000001;
        break; // Ch0
    case 1:
        page = 0b000010;
        break; // Ch1
    case 2:
        page = 0b000100;
        break; // Ch2
    case 3:
        page = 0b001000;
        break; // Ch3
    case 4:
        page = 0b010000;
        break; // Ch4
    case 5:
        page = 0b100000;
        break; // Ch5
    }

    std::vector<std::pair<std::size_t, uint8_t>> sequence = {
        { 0x008, page },
        { 0x146, CHNL_GAIN & 0xFF }, { 0x147, CHNL_GAIN >> 8 }, // цифровое усиление канала
    };
    for (auto& addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}
