#include <cmath>
#include <thread>

#include "sidedriver.h"

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

///
/// \brief      Установка и инициализация источника тактовой частоты для двух
///             АЦП.
///
/// \param      src   Источник тактовой частоты
/// \param      Fout  Выходная частота LMX
/// \param      Fosc  Частота осцилятора на субмодуле
/// \param      PwrA  LMX Мощность сигнала канала A (по умолчанию 63)
/// \param      PwrB  LMX Мощность сигнала канала B (по умолчанию 63)
/// \param      OutA  LMX Включение канала A (по умолчанию true)
/// \param      OutB  LMX Включение канала B (по умолчанию true)
///
/// \return     true при успехе, false при ошибке
///
auto Cmfm414x3g::SetClkMode(size_t src, double Fout, double Fosc, size_t PwrA, size_t PwrB, bool OutA, bool OutB) -> bool
{
    LOG("SetClkMode { src: %02zXh, Fosc: %.03f MHz, Fout: %.03f MHz, PwrA: %zd, PwrB: %zd, OutA: %s, OutB: %s }",
        src, Fosc, Fout, PwrA, PwrB, OutA ? "true" : "false", OutB ? "true" : "false");

    // Reset тетрады SYNC
    IndRegWrite(m_sync_tetr_num, ADM2IFnr_MODE0, 1);
    std::this_thread::sleep_for(100ms);
    IndRegWrite(m_sync_tetr_num, ADM2IFnr_MODE0, 0);

    // Очистим теневые регистры [0x01 .. 0x1F] тетрады SYNC
    for (auto reg_num = 0x01; reg_num < 0x20; reg_num++)
        IndRegWrite(m_sync_tetr_num, reg_num, 0x00);

    // выбор источника тактовой частоты
    IndRegWrite(m_sync_tetr_num, ADM2IFnr_FSRC, src);

    // выбор сигнала SYSREF и входного сопротивления внешнего старта
    auto ctrl = CONTROL1_0x17 {};
    ctrl.SYSREF_ADC_CNF = 0b00; // 00 - Сигнал SYSREF_ADC на АЦП
        // 10 -  Внешний  SYSREF на АЦП
        // 11 -  Внешний старт на SYSREF АЦП
    IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);

    // LMX2594 | Инициализация и начальные параметры
    // -------------------------------------------------------------------------
    if (src != 0x81 && src != 0x85) { // нужен всегда, кроме режимов 0x81 и 0x85 (внешний клок)
        this_thread::sleep_for(10ms); // ожидание разгона опорного генератора

        // ctrl.PLL_CE = true; // включим питание
        // IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);

        // reset, расчёт делителей для частоты, установка
        m_lmx.reset();
        auto result = m_lmx.freq_integer_calculate(Fosc, Fout);
        if (result.error) {
            LOG("ERROR: %s", result.error_msg);
            return false;
        } else {
            LOG("Fclk: %.03f MHz (%.03f) | Fpd:%.03f MHz Fvco:%.03f MHz | R_PRE:%zd R:%zd N:%zd CHDIV:%zd",
                result.vals.Fclk, result.vals.Delta, result.vals.Fpd, result.vals.Fvco, result.vals.PLL_R_PRE, result.vals.PLL_R, result.vals.PLL_N, result.vals.CHDIV);
        }
        m_lmx.freq_integer_set(Fosc, result.vals.PLL_R_PRE, result.vals.PLL_R, result.vals.PLL_N, result.vals.CHDIV);
        m_lmx.set_output_power(PwrA, PwrB); // 63 = + 4dBM на частоте 2'900 МГц
        m_lmx.set_output_enable(OutA, OutB);
        this_thread::sleep_for(100ms);
    }

    return true;
}

///
/// \brief      Установка уровня и входного сопротивления внешнего старта.
///
/// \param      level       Уровень внешнего старта: от 0 до 5 В
/// \param      resistance  Входное сопротивление внешнего старта: 0 = 1 кОм, 1
///                         = 50 Ом
///
/// \return     void
///
auto Cmfm414x3g::SetStartLevel(double level, size_t resistance) -> void
{
    // входное сопротивление
    auto ctrl = CONTROL1_0x17 {};
    ctrl.all = IndRegRead(m_sync_tetr_num, 0x17);
    ctrl.START_INPR = resistance;
    IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);

    // уровень старта
    auto val = uint16_t(std::round(level * (std::pow(2, 12) - 1) / 5.0));
    val <<= 4;
    SpdWrite(m_sync_tetr_num, 1, 0, 0x00, val);

    LOG("SetStartLevel { level: %.02f V [%02Xh], resistance: %s }",
        level, val, resistance ? "50 Ohm" : "1 kOhm");
}


///
/// \brief      Установка параметров сигнала SysRef для АЦП.
///
/// \param      level       Уровень внешнего сигнала SysRef: от 0 до 5 В
/// \param      select  Выбор источника сигнала SysRef
///
/// \return     void
///
auto Cmfm414x3g::SetSysRef(double level, size_t select) -> void
{
    // источник сигнала
    auto ctrl = CONTROL1_0x17{};
    ctrl.all = IndRegRead(m_sync_tetr_num, 0x17);
    ctrl.SYSREF_ADC_CNF = select;
    IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);

    // уровень старта
    auto val = uint16_t(std::round(level * (std::pow(2, 12) - 1) / 5.0));
    val <<= 4;
    SpdWrite(m_sync_tetr_num, 2, 0, 0x00, val);

    LOG("SetSysRef for ADC { level: %.02f V [%02Xh], select: 0x%02zX }", level, val, select);
}

///
/// \brief Установка и инициализация JESD PHY в FPGA Kintex Ultrascale (Plus) [JESD ID = 0x12]
///
/// \note Lane Rate GB | RXOUT_DIV | CPLL_REFCLK_DIV | CPLL_FBDIV | CPLL_FBDIV_45
///       ------------ | --------- | --------------- | ---------- | -------------
///       4.0 - 12.5   | 1 (0)     | 1 (16)          | 5 (3)      | 4 (0)
///       2.0 - 6.25   | 2 (1)     | 1 (16)          | 5 (3)      | 4 (0)
///       1.0 - 3.125  | 4 (2)     | 1 (16)          | 5 (3)      | 4 (0)
///
void Cmfm414x3g::setup_jesd_cpll(double lane_rate)
{
    DRP_0x63 val63 {};
    DRP_0x28 val28 {};
    DRP_0x2A val2A {};

    // получим значения для ADC0 GT0
    val63.all = SpdRead(m_sync_tetr_num, 4, 0, 0x063);
    val28.all = SpdRead(m_sync_tetr_num, 4, 0, 0x028);

    // выставим параметры
    val63.RXOUT_DIV = (lane_rate > 4'000.0) ? 0 /*1*/ : (lane_rate > 2'000.0) ? 1 /*2*/
                                                                              : 2 /*4*/;
    val28.CPLL_FBDIV = 3; // значение 5
    val28.CPLL_FBDIV_45 = 0; // значение 4

    LOG("JESD CPLL, GTX 0..7: RxOutDiv = %d, FbDiv = %d, FbDiv_45 = %d, RefClkDiv = %d",
        val63.RXOUT_DIV, val28.CPLL_FBDIV, val28.CPLL_FBDIV_45, val2A.CPLL_REFCLK_DIV);

    for (auto ch_num : { 0, 1, 2, 3, 4, 5, 6, 7 }) {
        // ADC0 GT Channel 0..7
        SpdWrite(m_sync_tetr_num, 4, ch_num, 0x063, val63.all);
        SpdWrite(m_sync_tetr_num, 4, ch_num, 0x028, val28.all);
        val2A.all = SpdRead(m_sync_tetr_num, 4, ch_num, 0x02A); // необходимо чтение, т.к. значения неизменяемых битов разные
        val2A.CPLL_REFCLK_DIV = 16; // значение 1
        SpdWrite(m_sync_tetr_num, 4, ch_num, 0x02A, val2A.all);

        // ADC1 GT Channel 0..7
        SpdWrite(m_sync_tetr_num, 4, 10 + ch_num, 0x063, val63.all);
        SpdWrite(m_sync_tetr_num, 4, 10 + ch_num, 0x028, val28.all);
        val2A.all = SpdRead(m_sync_tetr_num, 4, 10 + ch_num, 0x02A); // необходимо чтение, т.к. значения неизменяемых битов разные
        val2A.CPLL_REFCLK_DIV = 16; // значение 1
        SpdWrite(m_sync_tetr_num, 4, 10 + ch_num, 0x02A, val2A.all);
    }
}

///
/// \brief Установка и инициализация JESD PHY в FPGA Kintex Ultrascale [ JESD ID = 0x10 ]
///
void Cmfm414x3g::setup_jesd_qpll0(double lane_rate)
{
    auto qpll_n = (lane_rate > 8'994.0) ? 38 : // значение 40
        (lane_rate > 4'497.0) ? 78
                              : // значение 80
        158; // значение 160
    SpdWrite(m_sync_tetr_num, 4, 8, 0x014, qpll_n); // ADC0 GTX Common QPLL 0
    SpdWrite(m_sync_tetr_num, 4, 9, 0x014, qpll_n); // ADC0 GTX Common QPLL 1
    SpdWrite(m_sync_tetr_num, 4, 18, 0x014, qpll_n); // ADC1 GTX Common QPLL 0
    SpdWrite(m_sync_tetr_num, 4, 19, 0x014, qpll_n); // ADC1 GTX Common QPLL 1

    auto rxout_div = (lane_rate > 8'994.0) ? 0 : // значение 1
        (lane_rate > 4'497.0) ? 1
                              : // значение 2
        2; // значение 4
    for (auto i = 0; i < 8; i++) {
        SpdWrite(m_sync_tetr_num, 4, i, 0x063, rxout_div); // ADC0 GT Channel 0..7
        SpdWrite(m_sync_tetr_num, 4, 10 + i, 0x063, rxout_div); // ADC1 GT Channel 0..7
    }

    LOG("JESD QPLL0, GTX 0..7: 0x063 = %d, QPLL's: 0x014 = %d", rxout_div, qpll_n);
}
