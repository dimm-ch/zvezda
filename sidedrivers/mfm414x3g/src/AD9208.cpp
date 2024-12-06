///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#include <array>
#include <cmath>
#include <thread>
#include <utility>
#include <vector>

#include "AD9208.h"
// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

using namespace std::chrono_literals;

///
/// \brief      Конструктор с debug-версией функций SPI.
///
AD9208::AD9208()
    : spi_read([](size_t reg) { LOG("R [%04zX] : 0x%02X", reg, 0); return 0; })
    , spi_write([](size_t reg, uint8_t val) { LOG("W [%04zX] : 0x%02X", reg, val); })
{
}

///
/// \brief      Перезапуск чипа
///
/// \return     void
///
auto AD9208::reset() -> void
{
    LOG("Reset");
    spi_write(0x0002, 0x00); // Channel Power-Up
    spi_write(0x0000, 0x81); // Soft reset
    std::this_thread::sleep_for(5ms); // Hardware Reset Period = 5ms
    spi_write(0x0001, 0x02); // Datapath soft reset

    LOG("Set ADC Channel Select: Core A && Core B");
    spi_write(0x0008, 0b11); // все дальнейшие команды будут передаваться Core A и Core B

    // ad9208_set_pdn_pin_mode(&phy->ad9208, phy->powerdown_pin_en, phy->powerdown_mode == AD9208_POWERUP);
    // ad9208_set_input_clk_duty_cycle_stabilizer(&phy->ad9208, phy->duty_cycle_stabilizer_en);

    LOG("Set Input CLK Config: Divide by 1");
    // sample_rate = 2'550'000'000 * 1; // sampling_frequency_hz * input_div;
    // adc_clk_freq_hz = 2'550'000'000 / 1; // sample_rate / input_div;
    // if ((adc_clk_freq_hz > 3'100'000'000) || (adc_clk_freq_hz < 2'500'000'000)) return ERROR;
    spi_write(0x0108, 0); // 0 = div1, 1 = div2, 3 = div4

    // clk_set_rate(conv->clk, sample_rate); // не для AD9208
    // clk_prepare_enable(conv->clk); // не для AD9208

    // LOG("Set ADC Input Config");
    // current_scale = AD9208_ADC_SCALE_1P7_VPP;
    // ad9208_adc_set_input_cfg(&phy->ad9208, phy->analog_input_mode ? COUPLING_DC : COUPLING_AC, phy->ext_vref_en, phy->current_scale);

    // LOG("Set Input Buffer Config");
    // ad9208_adc_set_input_buffer_cfg(&phy->ad9208, phy->buff_curr_n, phy->buff_curr_p, AD9208_BUFF_CURR_600_UA);

    LOG("Input Clock: %s", (spi_read(0x011B) == 1) ? "OK" : "no");

    // SERDOUTx± data invert
    // spi_write(0x05BF, 0b00111111);

    // Enable DC offset calibration control
    spi_write(0x0701, 0x86);

    // Enable Fast Detect A/B output
    spi_write(0x0040, 0x00);

    // Set LMFC Offset (работает при значениях >=6)
    spi_write(0x0578, 0x0C);
}

///
/// \brief      Вывод ID чипа
///
/// \return     Структура ChipID { chip_type, prod_id, prod_grade, dev_revision
///             }
///
auto AD9208::get_chip_id() -> ChipID
{
    using CHIPGRADE = union {
        uint8_t all;
        struct {
            uint8_t DEV_REVISION : 4, PROD_GRADE : 2;
        };
    };

    auto chip_grade = CHIPGRADE {};
    chip_grade.all = spi_read(0x0006);

    auto id = ChipID {
        .chip_type = uint8_t(spi_read(0x0003)),
        .prod_id = uint16_t((spi_read(0x0005) << 8) | spi_read(0x0004)),
        .prod_grade = uint8_t(chip_grade.PROD_GRADE),
        .dev_revision = uint8_t(chip_grade.DEV_REVISION),
    };

    LOG("ChipINFO: type = %d, id = %04Xh, grade = %d, revision = %d",
        id.chip_type, id.prod_id, id.prod_grade, id.dev_revision);
    return id;
}

///
/// \brief      Установка частоты настройки NCO для каналов DDC
///
/// \param      value  значение частоты настройки в МГц
///
/// \return     void
///
auto AD9208::set_nco(size_t channel, double value) -> void
{
    auto nco = (value * 0x1'0000'0000'0000) / freq_clk;
    auto reg = 0x0316 + channel * 0x20;

    for (auto i = 0; i < 6; i++)
        spi_write(reg + i, (uint64_t(nco) >> (i * 8)) & 0xFF);

    LOG("Channel: %zd, Fnco: %.02f MHz, Fclk: %.02f MHz | FTW: %012zXh",
        channel, value, freq_clk, uint64_t(nco) & 0xFFFF'FFFF'FFFF);
}

auto AD9208::get_nco(size_t channel) -> double
{
    auto nco = uint64_t {};
    auto reg = 0x0316 + channel * 0x20;

    for (auto i = 0; i < 6; i++)
        nco |= uint64_t(spi_read(reg + i) & 0xFF) << (i * 8);

    double value = (nco * freq_clk) / 0x1'0000'0000'0000;

    // LOG("Channel: %zd, Fnco: %.02f MHz, Fclk: %.02f MHz | FTW: %012zXh", channel, value, sample_rate, nco);

    return value;
}

///
/// \brief      Установка смещения частоты настройки NCO для каналов DDC
///
/// \param      value  значение смещения в градусах
///
/// \return     void
///
auto AD9208::set_nco_offset(size_t channel, double value) -> void
{
    auto offset = std::round((value * 0x1'0000'0000'0000) / 360.0);
    auto reg = 0x031D + channel * 0x20;

    for (auto i = 0; i < 6; i++)
        spi_write(reg + i, (uint64_t(offset) >> (i * 8)) & 0xFF);

    LOG("Channel: %zd, Fnco Phase: %.02f\xF8 | OFF: %012zXh",
        channel, value, uint64_t(offset) & 0xFFFF'FFFF'FFFF);
}

///
/// \brief      Установка режима работы АЦП.
///
/// \param      app_mode  Номер режима приложения. (Chip Application Mode)
/// \param      dcm       Значение децимации.
///
/// \return     void
///
auto AD9208::set_mode(size_t app_mode, size_t dcm) -> void
{
    using CHIPMODE = union {
        uint8_t all;
        struct {
            uint8_t APPMODE : 4, : 1, Q_IGNORE : 1;
        };
    };
    auto mode = CHIPMODE {};
    complex = true;

    // режим работы АЦП
    switch (app_mode) {
    case 0b0001:
        LOG("Set Chip Mode: One DDC");
        mode.APPMODE = app_mode;
        jesd_M = 2; // 2 виртуальных конвертера для DDC IQ
        break;
    case 0b0010:
        LOG("Set Chip Mode: Two DDC");
        mode.APPMODE = app_mode;
        jesd_M = 4; // 4 виртуальных конвертера для DDC IQ
        break;
    case 0b0011:
        LOG("Set Chip Mode: Four DDC");
        mode.APPMODE = app_mode;
        jesd_M = 8; // 8 виртуальных конвертера для DDC IQ
        break;
    case 0b1110:
    case 0b1111:
        LOG("Set Chip Mode: One ADC %s (Full Bandwidth)", app_mode == 0b1111 ? "SWAP" : "");
        mode.APPMODE = 0b0000;
        complex = false;
        jesd_M = 1; // 1 виртуальный конвертер для ADC
        break;
    default:
        LOG("Set Chip Mode: Two ADC (Full Bandwidth)");
        mode.APPMODE = 0b0000;
        complex = false;
        jesd_M = 2; // 2 виртуальных конвертера для ADC IQ
    }
    spi_write(0x0200, mode.all);
    if (app_mode == 0b1111)
        spi_write(0x0564, 1); // режим свопа каналов АЦП
    else
        spi_write(0x0564, 0);

    // децимация
    decimate = dcm;
    if (dcm > 48)
        decimate = 48;
    if (dcm < 2)
        decimate = 2;
    if (mode.APPMODE == 0)
        decimate = 1;
    switch (decimate) {
    case 1:
        spi_write(0x0201, 0b0000);
        break;
    case 2:
        spi_write(0x0201, 0b0001);
        break;
    case 3:
        spi_write(0x0201, 0b1000);
        break;
    case 4:
        spi_write(0x0201, 0b0010);
        break;
    case 5:
        spi_write(0x0201, 0b0101);
        break;
    case 6:
        spi_write(0x0201, 0b1001);
        break;
    case 8:
        spi_write(0x0201, 0b0011);
        break;
    case 10:
        spi_write(0x0201, 0b0110);
        break;
    case 12:
        spi_write(0x0201, 0b1010);
        break;
    case 15:
        spi_write(0x0201, 0b0111);
        break;
    case 16:
        spi_write(0x0201, 0b0100);
        break;
    case 20:
        spi_write(0x0201, 0b1101);
        break;
    case 24:
        spi_write(0x0201, 0b1011);
        break;
    case 30:
        spi_write(0x0201, 0b1110);
        break;
    case 40:
        spi_write(0x0201, 0b1111);
        break;
    case 48:
        spi_write(0x0201, 0b1100);
        break;
    default:
        decimate = 2;
        spi_write(0x0201, 0b0001);
    }

    LOG_WITHOUT_NEWLINE("Set Chip Decimation Ratio: [ 0x0201 = %02X ] ", spi_read(0x0201));
    if (decimate == 1) {
        LOG_WITHOUT_NAMECLASS("Full Sample Rate\n");
    } else {
        LOG_WITHOUT_NAMECLASS("Decimate by %zd\n", decimate);
    }

    // фильтры для децимации
    // clang-format off
    auto filter1 = (decimate == 2) ? 0b011 : (decimate == 4) ? 0b000 : (decimate == 6) ? 0b100 : (decimate == 8) ? 0b001 : (decimate == 12) ? 0b101 : (decimate == 16) ? 0b010 : (decimate == 24) ? 0b110 : 0b111;
    auto filter2 = (decimate == 3) ? 0b0111 : (decimate == 10) ? 0b0010 : (decimate == 15) ? 0b1000 : (decimate == 20) ? 0b0011 : (decimate == 30) ? 0b1001 : (decimate == 40) ? 0b0100 : 0b0000;
    // clang-format on

    // using DDC_CONTROL = union { uint8_t all; struct { uint8_t RATE:3, REAL:1, IF_MODE:2, GAIN:1, MIXER:1 ;};};
    // auto control = DDC_CONTROL {};
    auto ddc_0x310 = DDC_0x310 {};
    ddc_0x310.GAIN = 1; // 6dB gain
    ddc_0x310.RATE = filter1;

    // using DDC_INPUT = union { uint8_t all; struct { uint8_t I_SEL:1, :1, Q_SEL:1, :1, RATE:4 ;};};
    // auto input = DDC_INPUT {};
    auto ddc_0x311 = DDC_0x311 {};
    ddc_0x311.RATE = filter2;

    for (auto reg : std::array<size_t, 4> { 0x310, 0x330, 0x350, 0x370 }) {
        spi_write(reg, ddc_0x310.all);
        spi_write(reg + 1, ddc_0x311.all);
    }
}

///
/// \brief      Установка DDC IQ входов на конкретный ADC.
///
/// \param      channel   Номер DDC канала
/// \param      source_i  Номер АЦП на I входе DDC
/// \param      source_q  Номер АЦП на Q входе DDC
///
/// \return     результат операции
///
auto AD9208::set_ddc_input_iq(size_t channel, size_t source_i, size_t source_q) -> bool
{
    if (channel > 3 || source_i > 1 || source_q > 1)
        return false;

    auto ddc_input = DDC_0x311 {};
    ddc_input.all = spi_read(0x311 + channel * 0x20);
    ddc_input.I_SEL = source_i; // Select Channel A || B
    ddc_input.Q_SEL = source_q; // Select Channel A || B
    spi_write(0x311 + channel * 0x20, ddc_input.all);

    LOG("Set DDC%zd %04zXh = %02X", channel, 0x311 + channel * 0x20, ddc_input.all);

    return true;
}

///
/// \brief      Настройка JESD'ов
///
/// \param      lane_rate_max    Максимальное значение на одной линии JESD в ПЛИС
///
/// \return     true при успехе, false при ошибке
///
auto AD9208::set_jesd(double lane_rate_max) -> bool
{
    sample_rate = freq_clk / decimate; // Расчёт частоты оцифрованного потока в МГц, с учётом децимации
    // lane_rate = (20 * sample_rate * jesd_M) / jesd_L;

    // Расчёт скорости на JESD линиях (подбор количества используемых линий)
    for (jesd_L = 1; jesd_L < 16; jesd_L <<= 1) { // lanes: 1,2,4,8
        lane_rate = (20 * sample_rate * jesd_M) / jesd_L;
        if (lane_rate <= lane_rate_max)
            break;
    }

    if (lane_rate > lane_rate_max) {
        LOG("- ERROR: Lane Count Out of Range");
        return false;
    }

    // clang-format off
    jesd_F = (jesd_M == 1) ? 2 : (jesd_M == 2) ? (jesd_L == 1) ? 4 : 2 : (jesd_M == 4) ? (jesd_L == 1) ? 8 : (jesd_L == 2) ? 4 : 2 : (jesd_M == 8) ? (jesd_L == 1) ? 16 : (jesd_L == 2) ? 8 : (jesd_L == 4) ? 4 : 2 : 0;
    // clang-format on

    LOG("Set JESD Parameters: lane_rate = %.03f | M = %zd L = %zd F = %zd",
        lane_rate, jesd_M, jesd_L, jesd_F);

    if (jesd_M == 0) {
        LOG("- ERROR: Lane Count Out of Range");
        return false;
    }

    // -------------------------------------------------------------------------
    // JESD: L = 8, M = 2, F = 1, K = 32, S = 2, N' = 16, N = 14, CS = 0, HD = 1
    // -------------------------------------------------------------------------
    // 01. Write 0x00 to Register 0x0200 (full bandwidth mode).
    // 02. Write 0x00 to Register 0x0201 (chip decimation ratio = 1).
    // 03. Write 0x15 to Register 0x0571 (JESD204B link powerdown).
    // 04. Write 0x87 to Register 0x058B (scrambling enabled, L = 8).
    // 05. Write 0x01 to Register 0x058E (M = 2).
    // 06. Write 0x00 to Register 0x058C (F = 1).
    // 07. Write 0x00 to Register 0x056E (lane rate = 6.75 Gbps to 13.5 Gbps).
    // 08. Write 0x14 to Register 0x0571 (JESD204B link power-up).
    // 09. Write 0x4F to Register 0x1228.
    // 10. Write 0x0F to Register 0x1228.
    // 11. Write 0x00 to Register 0x1222.
    // 12. Write 0x04 to Register 0x1222.
    // 13. Write 0x00 to Register 0x1222.
    // 14. Write 0x08 to Register 0x1262.
    // 15. Write 0x00 to Register 0x1262.
    // 16. Read Register 0x056F (PLL status register).
    // -------------------------------------------------------------------------

    // ad9208_jesd_syref_lmfc_offset_set(&phy->ad9208, phy->sysref_lmfc_offset);
    // ad9208_jesd_syref_config_set(&phy->ad9208, phy->sysref_edge_sel, phy->sysref_clk_edge_sel, phy->sysref_neg_window_skew, phy->sysref_pos_window_skew);
    // ad9208_jesd_syref_mode_set(&phy->ad9208, phy->sysref_mode, phy->sysref_count);
    spi_write(0x0120, 1 << 1); // SYSREF± mode select: Continuous
    spi_write(0x0300, 1 << 0); // DDC Sync mode: SYSREF± start NCO synchronize

    // LOG("Set JESD204 Interface Config");
    // ad9208_jesd_set_if_config(&phy->ad9208, phy->jesd_param, &lane_rate_kbps);

    // >>> 03. Write 0x15 to Register 0x0571 (JESD204B link powerdown).
    spi_write(0x0571, 0x15);

    // >>> 07. Write 0x00 to Register 0x056E (lane rate = 6.75 Gbps to 13.5 Gbps).
    // [  1687,  3375 ) = 5 (0101) |  1.687 Gbps to  3.375 Gbps
    // [  3375,  6750 ) = 1 (0001) |  3.375 Gbps to  6.750 Gbps
    // [  6750, 13500 ) = 0 (0000) |  6.750 Gbps to 13.500 Gbps
    // [ 13500, 15500 ) = 3 (0011) | 13.500 Gbps to 15.500 Gbps
    if (lane_rate <= 3'375.0)
        spi_write(0x056E, 0b0101 << 4);
    else if (3'375.0 < lane_rate && lane_rate <= 6'750.0)
        spi_write(0x056E, 0b0001 << 4);
    else if (6'750.0 < lane_rate && lane_rate <= 13'500.0)
        spi_write(0x056E, 0b0000 << 4);
    else if (13'500.0 < lane_rate)
        spi_write(0x056E, 0b0011 << 4);
    LOG("JESD Lane Rate = %.03f MHz [ 0x056E = %02X ]", lane_rate, spi_read(0x056E));

    if (lane_rate < 1'687.5 || lane_rate > 15'500.0) {
        LOG("- ERROR: Lane Rate Out of Range");
        return false;
    }

    // >>> 05. Write 0x01 to Register 0x058E (M = 2).
    spi_write(0x058E, jesd_M - 1); // M
    spi_write(0x058F, (0 << 6) | (14 - 1)); // CS & N
    spi_write(0x0590, (1 << 5) | (16 - 1)); // Subclass 1 & N'
    // >>> 06. Write 0x00 to Register 0x058C (F = 1).
    spi_write(0x058C, jesd_F - 1); // F Octets per Frame
    spi_write(0x058D, 32 - 1); // K Frames per Multiframe
    // >>> 04. Write 0x87 to Register 0x058B (scrambling enabled, L = 8).
    spi_write(0x058B, (1 << 7) | (jesd_L - 1)); // Scrambler & L
    // LOG("Set JESD Parameters: M = %zd, L = %zd, F = %zd, K = 32, N = 14, N' = 16, CS = 0, Subclass 1, Scrambler Enable",
    //     jesd_M, jesd_L, jesd_F);

    LOG("JESD Enable Link");
    // using LINK_CONTROL = union { uint8_t all; struct { uint8_t LINK_PWD:1, FACI:1, ILAS:2, LANESYNC:1, LONGTEST:1, TAIL:1, STANDBY:1 ;};};
    // auto link = LINK_CONTROL {};
    // link.all = spi_read(0x0571);
    // link.LINK_PWD = true;
    // spi_write(0x0571, link.all);

    // >>> 08. Write 0x14 to Register 0x0571 (JESD204B link power-up).
    spi_write(0x0571, 0x14);
    // link.LINK_PWD = false;
    // spi_write(0x0571, link.all);

    std::vector<std::pair<size_t, uint8_t>> sequence = {
        { 0x1228, 0x4F }, // >>> 09. Write 0x4F to Register 0x1228.
        { 0x1228, 0x0F }, // >>> 10. Write 0x0F to Register 0x1228.
        { 0x1222, 0x00 }, // >>> 11. Write 0x00 to Register 0x1222.
        { 0x1222, 0x04 }, // >>> 12. Write 0x04 to Register 0x1222.
        { 0x1222, 0x00 }, // >>> 13. Write 0x00 to Register 0x1222.
        { 0x1262, 0x08 }, // >>> 14. Write 0x08 to Register 0x1262.
        { 0x1262, 0x00 }, // >>> 15. Write 0x00 to Register 0x1262.
    };
    for (auto addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);

    // >>> 16. Read Register 0x056F (PLL status register).
    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(2000);
    auto pll_lock = false;
    do {
        pll_lock = spi_read(0x056F) >> 7;
    } while ((pll_lock != true) && (timeout > std::chrono::steady_clock::now()));

    LOG("JESD PLL Lock: %s", pll_lock ? "Ok" : "Error");
    if (pll_lock)
        return true;
    else
        return false;
}

///
/// \brief      Установка инверсии второго канала АЦП.
///
/// \return     void
///
auto AD9208::set_invert_channel_a() -> void
{
    std::vector<std::pair<size_t, uint8_t>> sequence = {
        { 0x0008, 0x03 },
        { 0x0DF8, 0x01 },
        { 0x0008, 0x01 },
        { 0x0E00, 0x00 },
        { 0x0E01, 0x80 },
        { 0x0008, 0x02 },
        { 0x0E00, 0xFF },
        { 0x0E01, 0x7F },
        { 0x0008, 0x03 },
        { 0x000F, 0x01 },
    };
    for (auto addr_val : sequence)
        spi_write(addr_val.first, addr_val.second);
}

///
/// \brief      Установка входной амплитуды канала.
///
/// \param      channel  Номер канала АЦП
/// \param      voltage  Значение напряжения [1.13, 1.25, 1.70, 1.81, 1.93,
///                      2.04]
///
/// \return     void
///
auto AD9208::set_input_fs_voltage(size_t channel, double voltage) -> void
{
    switch (channel) {
    case 0:
        spi_write(0x0008, 0x01);
        break;
    case 1:
        spi_write(0x0008, 0x02);
        break;
    default:
        return;
    }

    switch (size_t(std::round(voltage * 100))) {
    case 113:
        spi_write(0x1910, 0b1000);
        break; // 1.13V
    case 125:
        spi_write(0x1910, 0b1001);
        break; // 1.25V
    case 170:
        spi_write(0x1910, 0b1101);
        break; // 1.70V
    case 181:
        spi_write(0x1910, 0b1110);
        break; // 1.81V
    case 193:
        spi_write(0x1910, 0b1111);
        break; // 1.93V
    case 204:
        spi_write(0x1910, 0b0000);
        break; // 2.04V
    default:
        spi_write(0x1910, 0x0D);
    }
    LOG("Set Input FS Voltage [ 0x1910 = %02X ]", spi_read(0x1910));

    spi_write(0x0008, 0x03);
}

///
/// \brief      Установка входного значения Buffer Control канала в uA.
///
/// \param      channel  Номер канала АЦП
/// \param      value    Значение Buffer Control [400, 500, 600, 700, 800, 1000]
///
/// \return     void
///
auto AD9208::set_input_buffer_control(size_t channel, size_t value) -> void
{
    switch (channel) {
    case 0:
        spi_write(0x0008, 0x01);
        break;
    case 1:
        spi_write(0x0008, 0x02);
        break;
    default:
        return;
    }

    switch (value) {
    case 400:
        spi_write(0x1A4C, 0b00'0100);
        spi_write(0x1A4D, 0b00'0100);
        break; //  400uA
    case 500:
        spi_write(0x1A4C, 0b00'1001);
        spi_write(0x1A4D, 0b00'1001);
        break; //  500uA
    case 600:
        spi_write(0x1A4C, 0b01'1110);
        spi_write(0x1A4D, 0b01'1110);
        break; //  600uA
    case 700:
        spi_write(0x1A4C, 0b10'0011);
        spi_write(0x1A4D, 0b10'0011);
        break; //  700uA
    case 800:
        spi_write(0x1A4C, 0b10'1000);
        spi_write(0x1A4D, 0b10'1000);
        break; //  800uA
    case 1000:
        spi_write(0x1A4C, 0b11'0010);
        spi_write(0x1A4D, 0b11'0010);
        break; // 1000uA
    default:
        spi_write(0x1A4C, 0x19);
        spi_write(0x1A4D, 0x19);
    }
    spi_write(0x0008, 0x03);
}

///
/// \brief      Управление регистром задержки
///
/// \param      value  Значение регистра
///
/// \return     void
///
auto AD9208::set_clock_delay_control(uint8_t value) -> void
{
    spi_write(0x0110, value);
    LOG("- 0x110:%02Xh", spi_read(0x110));
}

///
/// \brief      Установка задержки принимаемого сигнала для канала АЦП.
///
/// \param      channel          Номер канала АЦП
/// \param      divider_phase    0x109 Clock divider phase
/// \param      superfine_delay  0x111 Clock superfine delay
/// \param      fine_delay       0x112 Clock fine delay
///
/// \return     void
///
auto AD9208::set_clock_delay(size_t channel, uint8_t divider_phase, uint8_t superfine_delay, uint8_t fine_delay) -> void
{
    switch (channel) {
    case 0:
        spi_write(0x0008, 0x01);
        break;
    case 1:
        spi_write(0x0008, 0x02);
        break;
    default:
        return;
    }

    spi_write(0x0109, divider_phase);
    spi_write(0x0111, superfine_delay);
    spi_write(0x0112, fine_delay);

    LOG("- 0x109:%02Xh 0x111:%02Xh 0x112:%02Xh", spi_read(0x109), spi_read(0x111), spi_read(0x112));

    spi_write(0x0008, 0x03);
}

///
/// \brief      Установка параметров Fast Detect'а АЦП
///
/// \param      channel  Номер канала АЦП
/// \param      enable   Признак включения fast detect'а
/// \param      upper    Верхний порог [-78..0] dBFS, по умолчанию 0
/// \param      lower    Нижний порог [-78..0] dBFS, по умолчанию 0
/// \param      time     Время срабатывания [1..65535] в сэмплах, по умолчанию 1
///
/// \return     void
///
auto AD9208::set_fast_detect(size_t channel, bool enable, double upper, double lower, uint16_t time) -> void
{
    switch (channel) {
    case 0:
        spi_write(0x0008, 0x01);
        break;
    case 1:
        spi_write(0x0008, 0x02);
        break;
    default:
        return;
    }

    spi_write(0x0245, enable);
    spi_write(0x024B, time & 0xFF);
    spi_write(0x024C, time >> 8);

    // clang-format off
    upper = upper < -78 ? -78 : upper > 0 ? 0 : upper;
    lower = lower < -78 ? -78 : lower > 0 ? 0 : lower;
    // clang-format on

    auto _upper = uint16_t(round(pow(10, upper / 20.) * pow(2, 13)));
    auto _lower = uint16_t(round(pow(10, lower / 20.) * pow(2, 13)));

    spi_write(0x0247, _upper & 0xFF);
    spi_write(0x0248, _upper >> 8);
    spi_write(0x0249, _lower & 0xFF);
    spi_write(0x024A, _lower >> 8);

    LOG("- 0x245:%02Xh 0x247:%02Xh 0x248:%02Xh 0x249:%02Xh 0x24A:%02Xh 0x24B:%02Xh 0x24C:%02Xh",
        spi_read(0x245), spi_read(0x247), spi_read(0x248), spi_read(0x249), spi_read(0x24A), spi_read(0x24B), spi_read(0x24C));

    spi_write(0x0008, 0x03);
}

///
/// \brief Установка режима тестовой последовательности
///
/// \param mode Режим теста
///
/// \return void
///
auto AD9208::set_prbs(uint16_t mode) -> void
{
    spi_write(0x0550, mode | (1 << 4) | (1 << 5));
    spi_write(0x0550, mode);
    LOG("Set PRBS mode: 0x550 = %02Xh", spi_read(0x550));
}

///
/// \brief    Установка режима временных меток TimeStamp
///
/// \param    enable   Признак включения режима
/// \param    polarity_negative   Признак отрицательной полярность внешнего сигнала SYSREF
///
/// \return   void
///
auto AD9208::set_time_stamp(bool enable, bool polarity_negative) -> void
{
    if (enable == false) {
        LOG("Disable TimeStamp Mode");
        spi_write(0x01FF, 0x00);
        return;
    }

    LOG("Enable TimeStamp Mode | Polarity of SYSREF signal: %s", polarity_negative ? "negative" : "positive");
    spi_write(0x01FF, 0x01);
    spi_write(0x058F, spi_read(0x058F) | (2 << 6)); // используем 2 контрольных бита (CS=2)
    spi_write(0x0559, 0x50); // контрольный бит 1 = 0, контрольный бит 0 = SYSREF
    spi_write(0x0120, spi_read(0x0120) | (polarity_negative << 4)); // SYSREF± transition select
}
