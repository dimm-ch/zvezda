#include "sidedriver.h"
#include <cmath>
#include <thread>

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

///
/// \brief      Установка и инициализация схемы тактирования модуля.
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
auto Cmfm214x3gda::SetClkMode(size_t src, double Fout, double Fosc, size_t PwrA, size_t PwrB, bool OutA, bool OutB) -> bool
{
    // если LMX уже настроен другим запущенным exam приложением,
    // то мы не делаем расчёт и не трогаем LMX ... пускай работает ...
    if (m_sync_slave) {
        m_lmx_Fout = Fout; // сохраним выходную частоту
        LOG("--==[ SLAVE ]==--");
        LOG("SetClkMode { src: %02zXh, Fosc: %.03f MHz, Fout: %.03f MHz }",
            IndRegRead(m_sync_tetr_num, ADM2IFnr_FSRC), Fosc, Fout);
        auto FDVR2 = IndRegRead(m_sync_tetr_num, 0x15);
        LOG("LMK input: %.02f MHz [ FDVR2: %zd ]", m_lmx_Fout / FDVR2, FDVR2);
        return true;
    }

    // настроим LMX
    LOG("--==[ MASTER ]==--");
    LOG("SetClkMode { src: %02zXh, Fosc: %.03f MHz, Fout: %.03f MHz, PwrA: %zd, PwrB: %zd, OutA: %s, OutB: %s }",
        src, Fosc, Fout, PwrA, PwrB, OutA ? "true" : "false", OutB ? "true" : "false");

    // -------------------------------------------------------------------------
    // Схема тактирования модуля
    // -------------------------------------------------------------------------
    //
    // +-----------+       +---------------+             +----------+
    // | Ситезатор |       | Делитель HMC  |   3200 МГц  |   АЦП    |
    // |    LMX    +---+---> регистр FDVR  +------------->  AD9208  |
    // |           |   |   |               |   максимум  |          |
    // +-----------+   |   +---------------+             +----------+
    //                 |                                                  +----------+
    //                 |   +---------------+             +----------+   +-> JESD АЦП |
    //                 |   | Делитель HMC  |   3100 МГц  | Делитель |   | +----------+
    //                 +---> регистр FDVR2 +------------->   LMK    +---+
    //                 |   |               |   максимум  |          |   | +----------+
    //                 |   +---------------+             +----------+   +-> JESD ЦАП |
    //                 |                                                  +----------+
    //                 |                                 +----------+
    //                 |                      14500 МГц  |   ЦАП    |
    //                 +--------------------------------->  AD9176  |
    //                                         максимум  |          |
    //                                                   +----------+
    //
    // -------------------------------------------------------------------------

    // Reset тетрады SYNC, если в режиме MASTER
    if (m_sync_slave == false) {
        IndRegWrite(m_sync_tetr_num, ADM2IFnr_MODE0, 1);
        std::this_thread::sleep_for(100ms);
        IndRegWrite(m_sync_tetr_num, ADM2IFnr_MODE0, 0);

        // Очистим теневые регистры [0x01 .. 0x1F] тетрады SYNC
        for (auto reg_num = 0x01; reg_num < 0x20; reg_num++)
            IndRegWrite(m_sync_tetr_num, reg_num, 0x00);
    }

    // выбор источника тактовой частоты
    IndRegWrite(m_sync_tetr_num, ADM2IFnr_FSRC, src);

    // выбор сигнала SYSREF и входного сопротивления внешнего старта
    auto ctrl = CONTROL1_0x17 {};
    ctrl.SYSREF_ADC_CNF = 0b00; // 00 - Сигнал SYSREF_ADC на АЦП
        // 01 - Сигнал SYSREF_DAC на АЦП
        // 10 - Внешний SYSREF на АЦП
        // 11 - Внешний старт на SYSREF АЦП
    ctrl.SYSREF_DAC_CNF = 0b00; // 00 - Сигнал SYSREF_DAC на ЦАП
        // 01 - Сигнал SYSREF_ADC на ЦАП
        // 10 - Внешний SYSREF на ЦАП
        // 11 - Внешний старт на SYSREF ЦАП
    IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);

    m_lmx_Fout = Fout; // для внешнего генератора

    // LMX2594 | Инициализация и начальные параметры
    // -------------------------------------------------------------------------
    if (src != 0x81 && src != 0x85) { // нужен всегда, кроме режимов 0x81 и 0x85 (внешний клок)
        this_thread::sleep_for(10ms); // ожидание разгона опорного генератора

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

        m_lmx_Fout = result.vals.Fclk; // сохраним полученную частоту
    }

    // Подбор коэффициентов деления входной тактовой частоты
    // для LMK01801 (FDVR2). Максимальная частота 3100 МГц.
    // -------------------------------------------------------------------------
    auto FDVR2 = 8; // значение по умолчанию
    for (auto div : std::vector<int> { 1, 2, 4, 8 }) {
        if ((m_lmx_Fout / div) > 3'100)
            continue;
        FDVR2 = div;
        break;
    }
    IndRegWrite(m_sync_tetr_num, 0x15, FDVR2);
    LOG("LMK input: %.02f MHz [ FDVR2: %d ]", m_lmx_Fout / FDVR2, FDVR2);

    return true;
}

///
/// \brief      Расчёт частоты CoreCLK и RefCLK для JESD'ов в ПЛИС.
///
/// \param      cmd        Команда драйверу (чтобы отличить ADC от DAC)
/// \param      lane_rate  Скорость на одной JESD линии.
///
/// \return     true при успехе, false при ошибке
///
auto Cmfm214x3gda::SetJesdCoreAndRefCLK(size_t cmd, double lane_rate, bool ext_core_clk) -> bool
{
    // Схема тактирования JESD'ов
    // -----------------------------------------------------------------------------
    //                                             EXT CORE CLK <--+
    //                                                             |    +----------+
    // +-----------+   +---------------+             +----------+  |  +-> JESD АЦП |
    // | Ситезатор |   | Делитель HMC  |   1600 МГц  | Делитель |  |  | +----------+
    // |    LMX    +---> регистр FDVR2 +------------->   LMK    +--+--+
    // |           |   |               |   максимум  |          |     | +----------+
    // +-----------+   +---------------+             +----------+     +-> JESD ЦАП |
    //                                                                  +----------+

    // Переключение JESD CoreCLK на внешний генератор (для НИУ-6)
    // -------------------------------------------------------------------------
    // если мы в режиме SLAVE, то ничего не переключаем
    if (!m_sync_slave) {
        auto ctrl = CONTROL1_0x17 {};
        ctrl.all = IndRegRead(m_sync_tetr_num, 0x17);
        ctrl.EXT_JESD_CORECLK = ext_core_clk;
        IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);
    }

    // LMK01801 | Делители CoreCLK и RefCLK для JESD'ов в ПЛИС
    // -------------------------------------------------------------------------
    // RefCLK -- опорная частота узла JESD в ПЛИС
    // CoreCLK -- тактовая частота узла JESD в ПЛИС
    //
    // Расчёт частоты CoreCLK и RefCLK:
    //   BitRate = (20 * (Fclk / decimation) * M) / L = (20 * (2900 / 2) * 2) / 8 = 7250
    //   CoreCLK = BitRate / 40 = 181.25
    //   RefCLK  = BitRate / 40 = 181.25 (должен находиться в диапазоне QPLL, CPLL и т.д.)
    //   Divider = Fclk / CoreCLK = 2900 / 181.25 = 16
    //
    using LMK_R0 = union {
        uint32_t all;
        struct {
            uint32_t
                RESET : 1,
                POWERDOWN : 1,
                CLKout0_3_PD : 1, CLKout4_7_PD : 1, CLKout8_11_PD : 1, CLKout12_13_PD : 1,
                CLKin0_BUF_TYPE : 1, CLKin1_BUF_TYPE : 1, : 2,
                CLKin0_DIV : 3, CLKin0_MUX : 2,
                CLKin1_DIV : 3, CLKin1_MUX : 2;
        };
    };
    using LMK_R3 = union {
        uint32_t all;
        struct {
            uint32_t
                CLKout12_13_ADLY : 6,
                CLKout12_13_HS : 1, SYNC1_QUAL : 2, : 1,
                SYNC0_POL_INV : 1, SYNC1_POL_INV : 1,
                NO_SYNC_CLKout0_3 : 1, NO_SYNC_CLKout4_7 : 1, NO_SYNC_CLKout8_11 : 1, NO_SYNC_CLKout12_13 : 1, : 3,
                SYNC0_FAST : 1, SYNC1_FAST : 1, SYNC0_AUTO : 1, SYNC1_AUTO : 1;
        };
    };
    using LMK_R5 = union {
        uint32_t all;
        struct {
            uint32_t
                CLKout0_3_DIV : 3,
                CLKout4_7_DIV : 3, CLKout8_11_DIV : 3,
                CLKout12_ADLY_SEL : 1, CLKout13_ADLY_SEL : 1, : 2,
                CLKout12_13_DIV : 11;
        };
    };

    // значения по умолчанию
    auto r0 = LMK_R0 { 0b01001000'00000'00000'11'00000000 };
    auto r3 = LMK_R3 { 0b00010'0000'011'000000'0'000000000 };
    auto r5 = LMK_R5 {};

    // отключим AUTO SYNC
    r3.SYNC0_AUTO = false;
    r3.SYNC1_AUTO = false;
    r3.SYNC0_POL_INV = true;
    r3.SYNC1_POL_INV = true;
    r3.CLKout12_13_ADLY = 5; // Digital Delay setting for CLKout12 & CLKout13.

    // если мы в режиме SLAVE, то читаем параметры из файла синхронизации
    if (m_sync_slave) {
        auto f = fopen(m_sync_fname_data.c_str(), "r");
        fscanf(f, "%08X:%08X", &r0.all, &r5.all);
        fclose(f);
        auto div_log = [](size_t div) { return (div == 0) ? 8 : div; };
        LOG("LMK Dividers: in %zd, out %zd,%zd; in %zd, out %zd,%zd",
            div_log(r0.CLKin0_DIV), div_log(r5.CLKout0_3_DIV), div_log(r5.CLKout4_7_DIV),
            div_log(r0.CLKin1_DIV), div_log(r5.CLKout8_11_DIV), div_log(r5.CLKout12_13_DIV));
    }

    // Расчёт делителей LMK
    auto found = false;
    auto lmk_clk = m_lmx_Fout / IndRegRead(m_sync_tetr_num, 0x15); // FDVR2
    m_jesd_coreclk = lane_rate / 40;
    double common_div_float = lmk_clk / m_jesd_coreclk; // для отбрасывания дробных значений
    ssize_t common_div = lmk_clk / m_jesd_coreclk;

    // Выбрасываем ошибку при дробном общем делителе частоты (мы работает только с целочисленными)
    if ((common_div_float - floor(common_div_float)) != 0) {
        LOG("- ERROR: Frequency dividers cannot be non-integer (%.02f)", common_div_float);
        return false;
    }

    if (m_sync_slave) {
        // подбор только выходного делителя
        auto div1 = (r0.CLKin0_DIV == 0) ? 8 : r0.CLKin0_DIV;
        m_jesd_refclk = lmk_clk / div1 / ((r5.CLKout0_3_DIV == 0) ? 8 : r5.CLKout0_3_DIV);
        for (auto div2 = 1; div2 <= 8; div2++)
            if (div1 * div2 == common_div) {
                found = true; // подобрали делитель

                // установка CoreCLK (выходы LMK 04 [ADC] и 12 [DAC])
                if (cmd == BRDctrl_ADC_SETJESD)
                    r5.CLKout4_7_DIV = (div2 == 8) ? 0 : div2; // выходной делитель (значение восемь заменяем на ноль)
                if (cmd == BRDctrl_DAC_SETJESDMODE)
                    r5.CLKout12_13_DIV = div2; // значение делителя прямое, замена не нужна

                break;
            }
    } else {
        // подбор всех делителей
        for (auto div1 = 2; div1 <= 8 && !found; div1++)
            for (auto div2 = 1; div2 <= 8; div2++)
                if (div1 * div2 == common_div) {
                    found = true; // подобрали делители
                    r0.CLKin0_MUX = r0.CLKin1_MUX = (div1 == 1) ? 0 : 1; // MUX входного делителя (bypass OR divide)
                    r0.CLKin0_DIV = r0.CLKin1_DIV = (div1 == 8) ? 0 : div1; // входной делитель (значение восемь заменяем на ноль)

                    // установка CoreCLK (выходы LMK 04 [ADC] и 12 [DAC])
                    r5.CLKout4_7_DIV = (div2 == 8) ? 0 : div2; // выходной делитель (значение восемь заменяем на ноль)
                    r5.CLKout12_13_DIV = div2; // значение делителя прямое, замена не нужна

                    // корректировка RefCLK, диаппазон [60 .. 820] МГц
                    m_jesd_refclk = m_jesd_coreclk;
                    while (m_jesd_refclk < 60) {
                        m_jesd_refclk *= 2; // увеличиваем частоту на 2
                        div2 /= 2;
                    }

                    // установка RefCLK
                    r5.CLKout0_3_DIV = r5.CLKout8_11_DIV = (div2 == 8) ? 0 : div2; // значение восемь заменяем на ноль
                    break;
                }
    }

    if (found == false) {
        LOG("- ERROR: LMK dividers not found");
        return false;
    }

    auto div_log = [](size_t div) { return (div == 0) ? 8 : div; };
    LOG("CoreCLK: %.02f MHz, RefCLK: %.02f MHz [ LMK Dividers: in %zd, out %zd,%zd; in %zd, out %zd,%zd ]",
        m_jesd_coreclk, m_jesd_refclk,
        div_log(r0.CLKin0_DIV), div_log(r5.CLKout0_3_DIV), div_log(r5.CLKout4_7_DIV),
        div_log(r0.CLKin1_DIV), div_log(r5.CLKout8_11_DIV), div_log(r5.CLKout12_13_DIV));

    // Программирование LMK
    if (!m_sync_slave) {
        r0.RESET = true;
        SpdWrite(m_sync_tetr_num, 3, 0, 0x000, r0.all);
        r0.RESET = false;
        r0.CLKin0_BUF_TYPE = r0.CLKin1_BUF_TYPE = 0; // Bipolar (LVDS)
        r0.CLKout8_11_PD = 1; // Out8 (RefClk DAC) отключим, т.к. RefClk общий для АЦП, ЦАП с выхода 0

        // запись в файл синхронизации для двух разных процессов
        auto f = fopen(m_sync_fname_data.c_str(), "w");
        fprintf(f, "%08X:%08X", r0.all, r5.all);
        fclose(f);
    }
    SpdWrite(m_sync_tetr_num, 3, 0, 0x000, r0.all);
    SpdWrite(m_sync_tetr_num, 3, 0, 0x003, r3.all);
    SpdWrite(m_sync_tetr_num, 3, 0, 0x005, r5.all);

    return true;
}

///
/// \brief      Установка JESD GTX Common QPLL множителей и JESD GT Channel's
///             делителей.
///
/// \return     true при успехе, false при ошибке
///
auto Cmfm214x3gda::SetJesdQPLL(size_t cmd) -> bool
{
    // ADC GTX Common QPLL's
    auto lane_rate_ref = m_jesd_refclk * 40;
    auto qpll_n = (lane_rate_ref > 8'994.0) ? 38 : // значение 40
        (lane_rate_ref > 4'497.0) ? 78
                                  : // значение 80
        158; // значение 160
    if (!m_sync_slave) {
        SpdWrite(m_sync_tetr_num, 4, 8, 0x014, qpll_n); // QPLL 0 (4 lane)
        SpdWrite(m_sync_tetr_num, 4, 9, 0x014, qpll_n); // QPLL 1 (4 lane)
    }
    LOG("GTX Common QPLL's: %d [0x014 = %d]", qpll_n == 38 ? 40 : qpll_n == 78 ? 80
                                                                               : 160,
        qpll_n);

    // ADC GT Channel 0..7
    using DRP_0x63 = union {
        uint32_t all;
        struct {
            uint32_t RXOUT_DIV : 3;
        };
    };
    using DRP_0x7C = union {
        uint32_t all;
        struct {
            uint32_t : 8, TXOUT_DIV : 3;
        };
    };

    auto lane_rate = m_jesd_coreclk * 40;

    // для АЦП
    if (cmd == BRDctrl_ADC_SETJESD) {
        auto rx = DRP_0x63 {};
        rx.all = SpdRead(m_sync_tetr_num, 4, 0, 0x063);
        for (auto i = 0; i < 8; i++) {
            rx.RXOUT_DIV = (lane_rate > 8'994.0) ? 0 : (lane_rate > 4'497.0) ? 1
                                                                             : 2; // значения: 1, 2, 4
            SpdWrite(m_sync_tetr_num, 4, i, 0x063, rx.all);
        }
        LOG("GT Channel 0..7 | RxOut_DIV: %d [0x063 = %d]", rx.RXOUT_DIV == 0 ? 1 : rx.RXOUT_DIV == 1 ? 2
                                                                                                      : 4,
            rx.RXOUT_DIV);
    }

    // для ЦАП
    if (cmd == BRDctrl_DAC_SETJESDMODE) {
        auto tx = DRP_0x7C {};
        tx.all = SpdRead(m_sync_tetr_num, 4, 0, 0x07C);
        for (auto i = 0; i < 8; i++) {
            tx.TXOUT_DIV = (lane_rate > 8'000.0) ? 0 : (lane_rate > 4'000.0) ? 1
                                                                             : 2; // значения: 1, 2, 4
            SpdWrite(m_sync_tetr_num, 4, i, 0x07C, tx.all);
        }
        LOG("GT Channel 0..7 | TxOut_DIV: %d [0x07C = %d]", tx.TXOUT_DIV == 0 ? 1 : tx.TXOUT_DIV == 1 ? 2
                                                                                                      : 4,
            tx.TXOUT_DIV);
    }

    return true;
}

///
/// \brief      Установка JESD GTX Common CPLL множителей и JESD GT Channel's
///             делителей.
///
/// \return     true при успехе, false при ошибке
///
/// \note Lane Rate GB | RXOUT_DIV | CPLL_REFCLK_DIV | CPLL_FBDIV | CPLL_FBDIV_45
///       ------------ | --------- | --------------- | ---------- | -------------
///       4.0 - 12.5   | 1 (0)     | 1 (16)          | 5 (3)      | 4 (0)
///       2.0 - 6.25   | 2 (1)     | 1 (16)          | 5 (3)      | 4 (0)
///       1.0 - 3.125  | 4 (2)     | 1 (16)          | 5 (3)      | 4 (0)
///
auto Cmfm214x3gda::SetJesdCPLL(size_t cmd) -> bool
{
    // ADC GT Common CPLL's
    using DRP_0x28 = union {
        uint32_t all;
        struct {
            uint32_t : 8,
                CPLL_FBDIV : 8,
                CPLL_FBDIV_45 : 1;
        };
    };
    using DRP_0x2A = union {
        uint32_t all;
        struct {
            uint32_t : 11,
                CPLL_REFCLK_DIV : 5;
        };
    };

    // ADC GT Channel 0..7
    using DRP_0x63 = union {
        uint32_t all;
        struct {
            uint32_t RXOUT_DIV : 3;
        };
    };
    using DRP_0x7C = union {
        uint32_t all;
        struct {
            uint32_t : 8, TXOUT_DIV : 3;
        };
    };

    DRP_0x63 val63 {};
    DRP_0x7C val7C {};
    DRP_0x28 val28 {};
    DRP_0x2A val2A {};

    // получим значения для ADC/DAC GT0
    val63.all = SpdRead(m_sync_tetr_num, 4, 0, 0x063);
    val7C.all = SpdRead(m_sync_tetr_num, 4, 0, 0x07C);
    val28.all = SpdRead(m_sync_tetr_num, 4, 0, 0x028);

    auto lane_rate = m_jesd_coreclk * 40;

    // выставим параметры
    val63.RXOUT_DIV = (lane_rate > 4'000.0) ? 0 /*1*/ : (lane_rate > 2'000.0) ? 1 /*2*/
                                                                              : 2 /*4*/;
    val7C.TXOUT_DIV = (lane_rate > 4'000.0) ? 0 /*1*/ : (lane_rate > 2'000.0) ? 1 /*2*/
                                                                              : 2 /*4*/;
    val28.CPLL_FBDIV = 3; // значение 5
    val28.CPLL_FBDIV_45 = 0; // значение 4

    LOG("JESD CPLL, GTX 0..7: %s = %d, FbDiv = %d, FbDiv_45 = %d, RefClkDiv = %d",
        (cmd == BRDctrl_ADC_SETJESD) ? "RxOutDiv" : "TxOutDiv",
        (cmd == BRDctrl_ADC_SETJESD) ? val63.RXOUT_DIV : val7C.TXOUT_DIV,
        val28.CPLL_FBDIV, val28.CPLL_FBDIV_45, val2A.CPLL_REFCLK_DIV);

    // ADC GT Channel 0..7
    for (auto ch_num : { 0, 1, 2, 3, 4, 5, 6, 7 }) {
        if (cmd == BRDctrl_ADC_SETJESD)
            SpdWrite(m_sync_tetr_num, 4, ch_num, 0x063, val63.all);
        if (cmd == BRDctrl_DAC_SETJESDMODE)
            SpdWrite(m_sync_tetr_num, 4, ch_num, 0x07C, val7C.all);

        if (m_sync_slave == false) {
            SpdWrite(m_sync_tetr_num, 4, ch_num, 0x028, val28.all);
            val2A.all = SpdRead(m_sync_tetr_num, 4, ch_num, 0x02A); // необходимо чтение, т.к. значения неизменяемых битов разные
            val2A.CPLL_REFCLK_DIV = 16; // значение 1
            SpdWrite(m_sync_tetr_num, 4, ch_num, 0x02A, val2A.all);
        }
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
auto Cmfm214x3gda::SetStartLevel(double level, size_t resistance) -> void
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
/// \brief      Установка параметров сигнала SysRef для ЦАП.
///
/// \param      level       Уровень внешнего сигнала SysRef: от 0 до 5 В
/// \param      resistance  Выбор источника сигнала SysRef
///
/// \return     void
///
auto Cmfm214x3gda::SetSysRefDAC(double level, size_t select) -> void
{
    // источник сигнала
    auto ctrl = CONTROL1_0x17 {};
    ctrl.all = IndRegRead(m_sync_tetr_num, 0x17);
    ctrl.SYSREF_DAC_CNF = select;
    IndRegWrite(m_sync_tetr_num, 0x17, ctrl.all);

    // уровень старта
    auto val = uint16_t(std::round(level * (std::pow(2, 12) - 1) / 5.0));
    val <<= 4;
    SpdWrite(m_sync_tetr_num, 2, 0, 0x00, val);

    LOG("SetSysRef for DAC { level: %.02f V [%02Xh], select: 0x%02zX }", level, val, select);
}

///
/// \brief      Установка параметров сигнала SysRef для АЦП.
///
/// \param      level       Уровень внешнего сигнала SysRef: от 0 до 5 В
/// \param      resistance  Выбор источника сигнала SysRef
///
/// \return     void
///
auto Cmfm214x3gda::SetSysRefADC(double level, size_t select) -> void
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
