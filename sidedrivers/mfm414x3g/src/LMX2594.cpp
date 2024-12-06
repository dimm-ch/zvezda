#include <cmath>
#include <limits>
#include <thread>

#include "LMX2594.h"

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

constexpr decltype(LMX2594::CHDIV_vals) LMX2594::CHDIV_vals;

///
/// \brief      Конструктор с debug-версией функций SPI.
///
LMX2594::LMX2594()
    : spi_read([](size_t reg) { LOG("R [%03zX] : 0x%04X", reg, 0); return 0; })
    , spi_write([](size_t reg, uint16_t val) { LOG("W [%03zX] : 0x%04X", reg, val); })
{
}

auto LMX2594::reset() -> void
{
    LOG("Reset");

    regs.RESET = true;
    spi_write(0, regs.all[0]);
    regs.RESET = false;
    spi_write(0, regs.all[0]);

    // regs.LD_DLY = 0;
    // regs.CAL_CLK_DIV = 0;

    // запись значений по умолчанию (сперва нужно записать всё задом на перёд)
    auto regnum = 112;
    do
        spi_write(regnum, regs.all[regnum]);
    while (--regnum >= 0);

    // ---------------------------------------------------------------------
    // set_input_path(96, true, 1, 1, 3); // OSC MHz, OSC_2X, Mult Bypass, PLL_R_PRE, PLL_R
    // set_mash_order_reset(3, true); // MASH_ORDER, MASH_RESET_N
    // set_fraction(64);
    // set_freq(981, 1);
}

///
/// \brief      Расчёт значений (делителей/умножителей) для желаемой выходной
///             частоты в целочисленном режиме.
///
/// \param      fosc  Частота опорного генератора в МГц
/// \param      fout  Желаемая выходная частота в МГц
///
/// \return     Структура `Result`
///
auto LMX2594::freq_integer_calculate(double fosc, double fout) -> Result
{
    auto result_error = Result {};
    result_error.error = true;

    if ((fosc < Fosc_MIN) || (fosc > Fosc_MAX)) {
        result_error.error_msg = "Bad Fosc_FREQ";
        return result_error;
    }
    if ((fout < Fout_MIN) || (fout > Fout_MAX)) {
        result_error.error_msg = "Bad Fout_FREQ";
        return result_error;
    }

    // начальные значения
    auto PLL_R_PRE = 1u;
    auto PLL_R = 1u;
    auto PLL_N = 120000u;
    auto CHDIV = CHDIV_vals[CHDIV_vals.size() - 1];
    auto Fpd = 0.0;
    auto Fvco = 0.0;
    auto Fclk = 0.0;
    auto delta = 0.0;
    auto delta_pre = std::numeric_limits<double>::max();

    auto result = Values {}; // полученные значения
    auto found = false;

    // цикл поиска значений (делителей/умножителей)
    for (PLL_R_PRE = 1, PLL_R = 1; PLL_R_PRE < 129 && PLL_R < 256; (Fpd > 250.0) ? PLL_R_PRE++ : PLL_R++) {
        Fpd = fosc / (PLL_R_PRE * PLL_R);
        if (Fpd < Fpd_MIN)
            break;

        if (Fpd_MIN <= Fpd && Fpd <= Fpd_MAX) {
            // printf("Fpd = %.03f, PLL_R_PRE = %d, PLL_R = %d\n", Fpd, PLL_R_PRE, PLL_R);

            for (PLL_N = 120000; PLL_N > 27; PLL_N--) {
                Fvco = Fpd * PLL_N;
                if (Fvco_MIN <= Fvco && Fvco <= Fvco_MAX) {
                    // printf("Fvco = %.03f, PLL_N = %d\n", Fvco, PLL_N);

                    for (auto i = (Fvco > 11500.0) ? 2 : 18; i > -1; i--) {
                        Fclk = Fvco / CHDIV_vals[i];
                        delta = std::abs(Fclk - fout);

                        // точно подобрали частоту
                        // if (delta == 0.0) {
                        if (delta < std::numeric_limits<double>::epsilon()) {
                            found = true;
                            result = { Fclk, delta, Fpd, Fvco, PLL_R_PRE, PLL_R, PLL_N, CHDIV_vals[i] };
                            break;
                        }

                        // сохранение ближайшего найденного результата
                        if (delta < delta_pre) {
                            delta_pre = delta;
                            result = { Fclk, delta, Fpd, Fvco, PLL_R_PRE, PLL_R, PLL_N, CHDIV_vals[i] };
                        }
                    }
                }
                if (found)
                    break;
            }
        }
        if (found)
            break;

        // спорное использование всего диапазона делителей
        // увеличивает поиск в 8-9 раз
        // if (PLL_R == 255) {
        //     PLL_R = 1;
        //     PLL_R_PRE++;
        // }
    }

    auto result_ok = Result {};
    result_ok.error = false;
    result_ok.vals = result;
    return result_ok;
}

auto LMX2594::freq_integer_set(double fosc, size_t r_pre, size_t r, size_t n, size_t chdiv) -> void
{
    auto Fpd = fosc / (r_pre * r);
    auto Fvco = Fpd * n;

    regs.OSC_2X = 0;
    regs.MULT = 1;
    regs.PLL_R_PRE = r_pre;
    regs.PLL_R = r;
    regs.CPG = 5; // 9mA
    spi_write(9, regs.all[9]); // OSC_2X
    spi_write(10, regs.all[10]); // MULT
    spi_write(11, regs.all[11]); // PLL_R_PRE
    spi_write(12, regs.all[12]); // PLL_R
    spi_write(14, regs.all[14]); // CPG

    regs.PLL_N_18_16 = (n >> 16);
    regs.PLL_N_15_0 = n & 0xFFFF;
    spi_write(34, regs.all[34]); // PLL_N
    spi_write(36, regs.all[36]); // PLL_N

    regs.PFD_DLY_SEL = (Fvco > 12'500.0) ? 2 : 1;
    regs.MASH_ORDER = 0; // Integer Mode
    regs.MASH_RESET_N = 0; // Integer Mode
    regs.PLL_NUM_31_16 = 0;
    regs.PLL_NUM_15_0 = 0;
    regs.PLL_DEN_31_16 = 0;
    regs.PLL_DEN_15_0 = 1;
    spi_write(44, regs.all[44]); // MASH_ORDER, MASH_RESET_N
    spi_write(37, regs.all[37]); // PFD_DLY_SEL
    spi_write(38, regs.all[38]); // PLL_DEN
    spi_write(39, regs.all[39]); // PLL_DEN
    spi_write(42, regs.all[42]); // PLL_NUM
    spi_write(43, regs.all[43]); // PLL_NUM

    regs.SEG1_EN = (chdiv > 2) ? true : false;
    // режим `VCO` или `Channel divider`
    regs.OUTA_MUX = (chdiv == 1) ? 1 : 0;
    regs.OUTB_MUX = (chdiv == 1) ? 1 : 0;
    regs.OUT_ISET = 0; // 0 - максимальная мощность, 3 - уменьшение помех
    regs.OUT_FORCE = true;
    for (auto i = 0u; i < CHDIV_vals.size(); i++)
        if (CHDIV_vals[i] == chdiv)
            regs.CHDIV = (i == 0) ? 0 : i - 1;
    spi_write(7, regs.all[7]); // OUT_FORCE
    spi_write(31, regs.all[31]); // SEG1_EN
    spi_write(45, regs.all[45]); // OUTA_MUX, OUT_ISET
    spi_write(46, regs.all[46]); // OUTB_MUX
    spi_write(75, regs.all[75]); // CHDIV

    // regs.CAL_CLK_DIV = (fosc > 800) ? 3 : (fosc > 400) ? 2 : (fosc > 200) ? 1 : 0;
    regs.CAL_CLK_DIV = 3; // `Div 8, All Fosc` в этом режиме минимальное количество шумов
    spi_write(1, regs.all[1]); // CAL_CLK_DIV

    regs.FCAL_LPFD_ADJ = (Fpd < 2.5) ? 3
        : (Fpd < 5)                  ? 2
        : (Fpd < 10)                 ? 1
                                     : 0;
    regs.FCAL_HPFD_ADJ = (Fpd > 200) ? 3
        : (Fpd > 150)                ? 2
        : (Fpd > 100)                ? 1
                                     : 0;
    regs.FCAL_EN = true;

    // Lock Detect
    // -------------------------------------------------------------------------
    // если MUXOUT_LD_SEL == true, то используется LED при успешной калибровки
    // если MUXOUT_LD_SEL == false, то используется SPI для чтения регистров

    regs.MUXOUT_LD_SEL = false; // включим чтение по SPI
    spi_write(0, regs.all[0]); // FCAL_LPFD_ADJ, FCAL_HPFD_ADJ, MUXOUT_LD_SEL, FCAL_EN

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(2000);
    do {
        regs.all[110] = spi_read(110);
    } while ((regs.rb_LD_VTUNE != 2) && (timeout > std::chrono::steady_clock::now()));
    if (regs.rb_LD_VTUNE != 2)
        LOG("ERROR: No invalid lock. rb_LD_VTUNE = %d", regs.rb_LD_VTUNE);

    regs.MUXOUT_LD_SEL = true; // LED для успешной калибровки
    spi_write(0, regs.all[0]); // FCAL_LPFD_ADJ, FCAL_HPFD_ADJ, MUXOUT_LD_SEL, FCAL_EN
}

// -----------------------------------------------------------------------------

// значение делителя CHDIV
auto LMX2594::get_chandiv(size_t index) -> int
{
    constexpr auto divs = std::array<int, 18> {
        2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 72,
        96, 128, 192, 256, 384, 512, 768
    };

    if (index < 18)
        return divs[index];
    else
        return 1;
}

// обновление частоты VCO (расчёт)
auto LMX2594::set_ndivider(double freq) -> void
{
    auto fraction = get_fraction();
    // auto pre_r = regs.PLL_R_PRE;

    auto mult_out = fosc * (regs.OSC_2X + 1) * regs.MULT / regs.PLL_R_PRE;
    mult_out = (mult_out < 0.000001) ? fosc : mult_out;

    // TODO: `INCLUDED_DIVIDE` расчётная величина, на неё умножается `mult_out`
    // пока считаем, что она всегда равна единице
    auto ndiv_f = regs.PLL_R * freq / (mult_out * 1); // (mult_out * flexINCLUDED_DIVIDE)
    auto ndiv = uint32_t(std::floor(ndiv_f));
    auto fract_num = uint32_t(std::round(fraction * (ndiv_f - ndiv)));

    regs.PLL_N_18_16 = (ndiv >> 16); // & 0b111;
    regs.PLL_N_15_0 = ndiv & 0xFFFF;
    regs.PLL_NUM_31_16 = fract_num >> 16;
    regs.PLL_NUM_15_0 = fract_num & 0xFFFF;

    spi_write(34, regs.all[34]); // PLL_N
    spi_write(36, regs.all[36]); // PLL_N
    spi_write(42, regs.all[42]); // PLL_NUM
    spi_write(43, regs.all[43]); // PLL_NUM

    LOG("PLL_N: %d, PLL_NUM: %d", ndiv, fract_num);
}

// обновление phase detector delay
auto LMX2594::set_phase_detector_delay(double vco_freq) -> void
{
    switch (regs.MASH_ORDER) {
    case 0:
        regs.PFD_DLY_SEL = (vco_freq < 12'500.00001) ? 1 : 2;
        break;
    case 1:
        if (vco_freq < 10'000.01)
            regs.PFD_DLY_SEL = 1;
        else
            regs.PFD_DLY_SEL = (vco_freq < 12'500.00001) ? 2 : 3;
        break;
    case 2:
        regs.PFD_DLY_SEL = (vco_freq < 10'000.00001) ? 2 : 3;
        break;
    case 3:
        regs.PFD_DLY_SEL = (vco_freq < 10'000.00001) ? 3 : 4;
        break;
    case 4:
        regs.PFD_DLY_SEL = (vco_freq < 10'000.00001) ? 5 : 6;
        break;
    }

    spi_write(37, regs.all[37]);

    LOG("PFD_DLY_SEL: %d", regs.PFD_DLY_SEL);
}

//
// Установка параметров `Input Path`
//
// freq:    частота осцилятора в МГц
// x2:      признак умножения на два
// pre_div: предварительный делитель [1..128]
// mult:    множитель (1,3,4,5,6,7)
// div:     делитель [0..255]
auto LMX2594::set_input_path(double freq, bool x2, int pre_div, size_t mult, uint8_t div) -> void
{
    fosc = freq;
    regs.OSC_2X = x2;

    pre_div = (pre_div < 1) ? 1 : pre_div;
    pre_div = (pre_div > 128) ? 128 : pre_div;
    regs.PLL_R_PRE = pre_div;

    mult = ((mult < 3) || (mult > 7)) ? 1 : mult;
    regs.MULT = mult;

    regs.PLL_R = div;

    spi_write(9, regs.all[9]);
    spi_write(10, regs.all[10]);
    spi_write(11, regs.all[11]);
    spi_write(12, regs.all[12]);

    LOG("Freq: %.0f MHz, OSC_2X: %d, PLL_R_PRE: %d, MULT: %d, PLL_R: %d",
        fosc, regs.OSC_2X, regs.PLL_R_PRE, regs.MULT, regs.PLL_R);

    // VCO Calibration
    auto fpd = (fosc * ((x2) ? 2 : 1) * mult / pre_div) / div; // round(freq, 10)

    regs.FCAL_LPFD_ADJ = (fpd < 2.5) ? 3 : (fpd < 5) ? 2
        : (fpd < 10)                                 ? 1
                                                     : 0;
    regs.FCAL_HPFD_ADJ = (fpd > 200) ? 3 : (fpd > 150) ? 2
        : (fpd > 100)                                  ? 1
                                                       : 0;
    // regs.CAL_CLK_DIV   = (fosc > 800) ? 3 : (fosc > 400) ? 2 : (fosc > 200) ? 1 : 0;
    regs.CAL_CLK_DIV = 3; // В этом режиме минимальное количество шумов
    regs.FCAL_EN = true;
    spi_write(0, regs.all[0]);

    LOG("VCO Calibration: FPD Freq = %.0f MHz, LPFD_ADJ = '%s', HPFD_ADJ = '%s', CAL_CLK_DIV = '%s'",
        fpd,
        (regs.FCAL_LPFD_ADJ == 0)       ? "Fpd >= 10"
            : (regs.FCAL_LPFD_ADJ == 1) ? "5 >= Fpd > 10"
            : (regs.FCAL_LPFD_ADJ == 2) ? "2.5 >= Fpd > 5"
            : (regs.FCAL_LPFD_ADJ == 3) ? "Fpd < 2.5"
                                        : "NONE",
        (regs.FCAL_HPFD_ADJ == 0)       ? "Fpd <= 100"
            : (regs.FCAL_HPFD_ADJ == 1) ? "100 < Fpd <= 150"
            : (regs.FCAL_HPFD_ADJ == 2) ? "150 <= Fpd =< 200"
            : (regs.FCAL_HPFD_ADJ == 3) ? "Fpd > 200"
                                        : "NONE",

        (regs.CAL_CLK_DIV == 0)       ? "Fosc <= 200"
            : (regs.CAL_CLK_DIV == 1) ? "200 < Fosc <= 400"
            : (regs.CAL_CLK_DIV == 2) ? "400 < Fosc <= 800"
            : (regs.CAL_CLK_DIV == 3) ? "Fosc > 800"
                                      : "NONE");
}

auto LMX2594::set_fraction(size_t v) -> void
{
    regs.PLL_DEN_31_16 = v >> 16;
    regs.PLL_DEN_15_0 = v & 0xFFFF;

    spi_write(38, regs.all[38]);
    spi_write(39, regs.all[39]);

    LOG("PLL_DEN: %zu", v);
}

auto LMX2594::get_fraction() -> int
{
    return (regs.PLL_DEN_31_16 << 16) | regs.PLL_DEN_15_0;
}

auto LMX2594::set_mash_order_reset(size_t order, bool reset) -> void
{
    order = (order > 4) ? 1 : order;
    regs.MASH_ORDER = order;
    regs.MASH_RESET_N = reset;

    spi_write(44, regs.all[44]);

    LOG("MASH_ORDER: %d, MASH_RESET_N: %d", regs.MASH_ORDER, regs.MASH_RESET_N);
}

auto LMX2594::set_mash_reset_count(size_t v) -> void
{
    regs.MASH_RST_COUNT_31_16 = v >> 16;
    regs.MASH_RST_COUNT_15_0 = v & 0xFFFF;
}

auto LMX2594::set_ramp_thresh(uint64_t v) -> void
{
    regs.RAMP_THRESH_32 = v >> 32;
    regs.RAMP_THRESH_31_16 = v >> 16;
    regs.RAMP_THRESH_15_0 = v & 0xFFFF;
}

auto LMX2594::set_ramp_limits(uint64_t high, uint64_t low) -> void
{
    regs.RAMP_LIMIT_HIGH_32 = high >> 32;
    regs.RAMP_LIMIT_HIGH_31_16 = high >> 16;
    regs.RAMP_LIMIT_HIGH_15_0 = high & 0xFFFF;

    regs.RAMP_LIMIT_LOW_32 = low >> 32;
    regs.RAMP_LIMIT_LOW_31_16 = low >> 16;
    regs.RAMP_LIMIT_LOW_15_0 = low & 0xFFFF;
}

auto LMX2594::set_ramps_inc(size_t ramp0, size_t ramp1) -> void
{
    regs.RAMP0_INC_29_16 = (ramp0 >> 16) & 0x3FFF;
    regs.RAMP0_INC_15_0 = ramp0 & 0xFFFF;
    regs.RAMP1_INC_29_16 = (ramp1 >> 16) & 0x3FFF;
    regs.RAMP1_INC_15_0 = ramp1 & 0xFFFF;
}

auto LMX2594::set_output_enable(bool outa, bool outb) -> void
{
    regs.OUTA_PD = !outa; // set power down
    regs.OUTB_PD = !outb;
    spi_write(44, regs.all[44]);
}

auto LMX2594::set_output_power(size_t pwra, size_t pwrb) -> void
{
    regs.OUTA_PWR = pwra & 0b11'1111;
    regs.OUTB_PWR = pwrb & 0b11'1111;
    spi_write(44, regs.all[44]);
    spi_write(45, regs.all[45]);
}

// выставление выходной частоты на порту (расчёт)
//  - freq: желаемая частота в МГц
//  - port: номер выходного порта (0-Порт A, 1-порт B)
auto LMX2594::set_freq(double freq, size_t port) -> void
{
    auto vco_freq = double {};

    if (freq > 7499.999) {
        if (port == 0)
            regs.OUTA_MUX = 1;
        else
            regs.OUTB_MUX = 1;
        vco_freq = freq; // round(freq, 10) -> 10 знаков после запятой
        if ((regs.CHDIV > 2) && (vco_freq > 11500)) // может быть CHDIV обнулять всегда ?
            regs.CHDIV = 0;
    } else {
        if (port == 0)
            regs.OUTA_MUX = 0;
        else
            regs.OUTB_MUX = 0;

        auto div = get_chandiv(regs.CHDIV);
        auto vco = freq * div;
        if ((vco > 7499.9999) && (vco < 15000.0001) && ((vco < 11500.001) || (div < 7)))
            vco_freq = vco; // round(freq, 10)
        else {
            auto found = false;
            for (auto i = 0U; i < 18; i++) {
                div = get_chandiv(i);
                vco = freq * div;
                if ((vco > 7499.9999) && ((vco < 11500.001) || (div < 7))) {
                    vco_freq = vco; // round(freq, 10)
                    regs.CHDIV = i;
                    found = true;
                    break;
                }
            }

            if (found == false) {
                vco_freq = 7500.0;
                regs.CHDIV = 17;
                freq = 7500.0 / 768;
            }
        }
    }

    LOG("Set Freq %.04f on port %zu", freq, port);
    LOG(" - VCO Freq: %.04f", vco_freq);
    LOG(" - CHDIV: %d '%d'", regs.CHDIV, get_chandiv(regs.CHDIV));
    LOG(" - OUT%c_MUX: %d", (port == 0) ? 'A' : 'B', (port == 0) ? regs.OUTA_MUX : regs.OUTB_MUX);

    spi_write(45, regs.all[45]); // OUTA_MUX
    spi_write(46, regs.all[46]); // OUTB_MUX
    spi_write(75, regs.all[75]); // CHDIV

    set_ndivider(vco_freq);
    set_phase_detector_delay(vco_freq);

    // Lock Detect
    // если MUXOUT_LD_SEL == true, то используется LED при успешной калибровки
    // если MUXOUT_LD_SEL == false, то используется SPI для чтения регистров

    regs.MUXOUT_LD_SEL = false; // включим чтение по SPI
    // regs.FCAL_EN = true;
    spi_write(0, regs.all[0]);

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(2000);
    do {
        regs.all[110] = spi_read(110);
    } while ((regs.rb_LD_VTUNE != 2) && (timeout > std::chrono::steady_clock::now()));
    if (regs.rb_LD_VTUNE != 2)
        LOG("ERROR: No invalid lock. rb_LD_VTUNE = %d", regs.rb_LD_VTUNE);

    regs.MUXOUT_LD_SEL = true; // LED для успешной калибровки
    spi_write(0, regs.all[0]);
}

/*
def UpdateVCOGain():
    Fvco=Fvco_FREQ.dValue

    if (Fvco<7500):
        Kvco = 73
    elif (Fvco<8600):
        Kvco = 73+(114-73)*(Fvco-7500)/(8600-7500)
    elif (Fvco<9800):
        Kvco = 61+(121-61)*(Fvco-8600)/(9800-8600)
    elif (Fvco<10800):
        Kvco = 98 +(132-98)*(Fvco-9800)/(10800-9800)
    elif (Fvco<12000):
        Kvco = 106 +(141-106)*(Fvco-10800)/(12000-10800)
    elif (Fvco<12900):
        Kvco = 170+(215-170)*(Fvco-12000)/(12900-12000)
    elif (Fvco<13900):
        Kvco = 172+(218-172)*(Fvco-12900)/(13900-12900)
    elif (Fvco<15000):
        Kvco = 182+(239-182)*(Fvco-13900)/(15000-13900)
    else:
        Kvco = 239
    flexKVCO.dValue=round(Kvco)
*/

/*
def UpdateIncludedDivide():
    if (VCO_PHASE_SYNC.iValue==0):
        flexINCLUDED_DIVIDE.iValue=1
    elif (SYSREF_EN.iValue==0) and (OUTA_MUX.iValue==1) and (OUTB_MUX.iValue==1):
        flexINCLUDED_DIVIDE.iValue=1
    elif (SYSREF_EN.iValue==0) and (OUTA_MUX.iValue==2) and (OUTB_MUX.iValue==1):
        flexINCLUDED_DIVIDE.iValue=1
    elif (flexSEG1.iValue % 3 == 0):
        flexINCLUDED_DIVIDE.iValue=6
    else:
        flexINCLUDED_DIVIDE.iValue=4

    UpdateSYSREFInterpolator();
*/
