///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#pragma once
#include <array>
#include <cstdint>
#include <functional>

class LMX2594 {
    static constexpr auto name_class = "LMX2594"; // имя класса для отладочных сообщений

    using spi_read_t = std::function<uint16_t(size_t)>;
    using spi_write_t = std::function<void(size_t, uint16_t)>;

    spi_read_t spi_read; // функции SPI чтения/записи регистров
    spi_write_t spi_write;

    // Предельные значения частоты опорного генератора в МГц
    static constexpr auto Fosc_MIN = 5.0;
    static constexpr auto Fosc_MAX = 1400.0;
    // Предельные значения выходной частоты в МГц
    static constexpr auto Fout_MIN = 10.0;
    static constexpr auto Fout_MAX = 15000.0;
    // Предельные значения частоты Fpd в МГц
    static constexpr auto Fpd_MIN = 0.125;
    static constexpr auto Fpd_MAX = 400.0;
    // Предельные значения частоты Fvco в МГц
    static constexpr auto Fvco_MIN = 7500.0;
    static constexpr auto Fvco_MAX = 15000.0;
    // Значения делителя канала (CHDIV)
    static constexpr auto CHDIV_vals = std::array<uint16_t, 19> { 1, 2, 4, 6, 8, 12, 16, 24, 32, 48, 64, 72, 96, 128, 192, 256, 384, 512, 768 };

    // -------------------------------------------------------------------------
    double fosc; // частота опорного генератора
    auto get_chandiv(size_t index) -> int; // значение делителя CHDIV
    auto set_ndivider(double freq) -> void; // обновление частоты VCO (расчёт)
    auto set_phase_detector_delay(double vco_freq) -> void; // обновление phase detector delay
    // -------------------------------------------------------------------------

public:
    LMX2594();
    ~LMX2594() = default;

    auto setup_spi(spi_read_t r, spi_write_t w) -> void
    {
        spi_read = r;
        spi_write = w;
    };
    auto reset() -> void;

    // Выходные значения функции `freq_calculate`
    using Values = struct {
        double Fclk; ///< Полученная частота (точная или приближённая)
        double Delta; ///< Дельта частоты (запрашиваемая - расчётная)
        double Fpd; ///< Частота PFD
        double Fvco; ///< Частота VCO
        size_t PLL_R_PRE; ///< Пред-делитель
        size_t PLL_R; ///< Пост-делитель
        size_t PLL_N; ///< Множитель
        size_t CHDIV; ///< Делитель каналов
    };

    using Result = struct {
        bool error; ///< Признак ошибки
        union {
            const char* error_msg; ///< Сообщение об ошибке
            Values vals; ///< Выходные значения
        };
    };

    auto freq_integer_calculate(double fosc, double fout) -> Result;
    auto freq_integer_set(double fosc, size_t r_pre, size_t r, size_t n, size_t chdiv) -> void;
    auto output_mux(size_t outa_mux, size_t outb_mux) -> void;
    auto output_enable(bool outa = false, bool outb = false) -> void;
    auto output_power(size_t pwra = 31, size_t pwrb = 31) -> void;

    // -------------------------------------------------------------------------
    auto set_input_path(double freq, bool x2, int pre_div, size_t mult, uint8_t div) -> void;
    // auto set_vco_calibration(size_t lpfd_adj, size_t hpfd_adj, size_t clk_div) -> void;
    auto set_fraction(size_t v) -> void;
    auto get_fraction() -> int;
    auto set_mash_order_reset(size_t order = 3, bool reset = true) -> void;
    auto set_mash_reset_count(size_t v) -> void;
    auto set_ramp_thresh(uint64_t v) -> void;
    auto set_ramp_limits(uint64_t high = 0, uint64_t low = 0) -> void;
    auto set_ramps_inc(size_t ramp0 = 0, size_t ramp1 = 0) -> void;
    auto set_freq(double freq, size_t port = 0) -> void;
    auto set_output_mux(size_t outa_mux, size_t outb_mux) -> void;
    auto set_output_enable(bool outa = false, bool outb = false) -> void;
    auto set_output_power(size_t pwra = 31, size_t pwrb = 31) -> void;
    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Регистры
    // -------------------------------------------------------------------------
    union {
        // массив всех регистров
        std::array<volatile uint16_t, 113> all {
            0x2410, 0x080B, 0x0500, 0x0642, 0x0A43, 0x00C8, 0xC802, 0x00B2, 0x2000, 0x0604,
            0x10D8, 0x0018, 0x5001, 0x4000, 0x1E70, 0x064F, 0x0080, 0x00FE, 0x0064, 0x27F7,
            0xB848, 0x0401, 0x0001, 0x007C, 0x071A, 0x0624, 0x0DB0, 0x0002, 0x0488, 0x318C,
            0x318C, 0x43EC, 0x0393, 0x1E21, 0x0000, 0x0004, 0x0032, 0x0204, 0xFFFF, 0xFFFF,
            0x0000, 0x0000, 0x0000, 0x0000, 0x1FE2, 0xCEDF, 0x07FF, 0x0300, 0x0300, 0x4180,
            0x0000, 0x0080, 0x0820, 0x0000, 0x0000, 0x0000, 0x0000, 0x0020, 0x8001, 0x0001,
            0x03F8, 0x00A8, 0x0322, 0x0000, 0x1388, 0x0000, 0x01F4, 0x0000, 0x03E8, 0x0000,
            0xC350, 0x0081, 0x0001, 0x003F, 0x0000, 0x0800, 0x000C, 0x0000, 0x0065, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0800, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x4440, 0x0007, 0x0000, 0x0000, 0x0000,
            0x0000, 0x0000, 0x0000
        };
#pragma pack(push, 1)
        struct {
            // R0
            unsigned POWERDOWN : 1, RESET : 1, MUXOUT_LD_SEL : 1, FCAL_EN : 1, : 1, FCAL_LPFD_ADJ : 2, FCAL_HPFD_ADJ : 2, OUT_MUTE : 1, : 4, VCO_PHASE_SYNC_EN : 1, RAMP_EN : 1;
            unsigned CAL_CLK_DIV : 3, : 13;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 14, OUT_FORCE : 1, : 1;
            unsigned : 11, VCO_CAPCTRL_FORCE : 1, : 2, VCO_DACISET_FORCE : 1, : 1;
            unsigned : 12, OSC_2X : 1, : 3;
            // R10
            unsigned : 7, MULT : 5, : 4; // MULT=1 (Bypass)
            unsigned : 4, PLL_R : 8, : 4; // PLL_R=3 (R)
            unsigned PLL_R_PRE : 12, : 4; // PLL_R_PRE=1 (PreR)
            unsigned : 16;
            unsigned : 4, CPG : 3, : 9; // CPG=7 (15mA)
            unsigned : 16;
            unsigned VCO_DACISET : 9, : 7; // VCO_DACISET=128
            unsigned VCO_DACISET_STRT : 9, : 7; // VCO_DACISET_STRT=300
            unsigned : 16;
            unsigned VCO_CAPCTRL : 8, : 8; // VCO_CAPCTRL=183
            // R20
            unsigned : 10, VCO_SEL_FORCE : 1, VCO_SEL : 3, : 2; // VCO_SEL=4
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            // R30
            unsigned : 16;
            unsigned : 14, SEG1_EN : 1, : 1;
            unsigned : 16;
            unsigned : 16;
            unsigned PLL_N_18_16 : 3, : 13;
            unsigned : 16;
            unsigned PLL_N_15_0 : 16; // PLL_N_15_0=124
            unsigned : 8, PFD_DLY_SEL : 6, : 1, MASH_SEED_EN : 1; // PFD_DLY_SEL=3
            unsigned PLL_DEN_31_16 : 16;
            unsigned PLL_DEN_15_0 : 16; // PLL_DEN_15_0=4
            // R40
            unsigned MASH_SEED_31_16 : 16;
            unsigned MASH_SEED_15_0 : 16;
            unsigned PLL_NUM_31_16 : 16;
            unsigned PLL_NUM_15_0 : 16;
            unsigned MASH_ORDER : 3, : 2, MASH_RESET_N : 1, OUTA_PD : 1, OUTB_PD : 1, OUTA_PWR : 6, : 2; // MASH_ORDER=3, MASH_RESET_N=1, OUTA_PWR=31
            unsigned OUTB_PWR : 6, : 3, OUT_ISET : 2, OUTA_MUX : 2, : 3; // OUTB_PWR=31
            unsigned OUTB_MUX : 2, : 14;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            // R50
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 9, INPIN_FMT : 3, INPIN_LVL : 2, INPIN_HYST : 1, INPIN_IGNORE : 1; // INPIN_IGNORE=1
            unsigned LD_TYPE : 1, : 15; // LD_TYPE=1
            // R60
            unsigned LD_DLY : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned MASH_RST_COUNT_31_16 : 16; // MASH_RST_COUNT_31_16=0
            // R70
            unsigned MASH_RST_COUNT_15_0 : 16; // MASH_RST_COUNT_15_0=50000
            unsigned : 2, SYSREF_REPEAT : 1, SYSREF_EN : 1, SYSREF_PULSE : 1, SYSREF_DIV_PRE : 3, : 8; // SYSREF_DIV_PRE=4
            unsigned SYSREF_DIV : 11, : 5; // SYSREF_DIV=1
            unsigned JESD_DAC1_CTRL : 6, JESD_DAC2_CTRL : 6, : 4; // JESD_DAC1_CTRL=63
            unsigned JESD_DAC3_CTRL : 6, JESD_DAC4_CTRL : 6, SYSREF_PULSE_CNT : 4;
            unsigned : 6, CHDIV : 5, : 5; // CHDIV=3
            unsigned : 16;
            unsigned : 16;
            unsigned : 1, VCO_CAPCTRL_STRT : 8, QUICK_RECAL_EN : 1, : 1, RAMP_THRESH_32 : 1, : 4; // VCO_CAPCTRL_STRT=1
            unsigned RAMP_THRESH_31_16 : 16; // RAMP_THRESH_31_16=26h
            // R80
            unsigned RAMP_THRESH_15_0 : 16; // RAMP_THRESH_15_0=6666h (2'516'582)
            unsigned RAMP_LIMIT_HIGH_32 : 1, : 15;
            unsigned RAMP_LIMIT_HIGH_31_16 : 16; // RAMP_LIMIT_HIGH_31_16=1E00h
            unsigned RAMP_LIMIT_HIGH_15_0 : 16; // RAMP_LIMIT_HIGH_15_0=0000h (503'316'480)
            unsigned RAMP_LIMIT_LOW_32 : 1, : 15; // 0001h
            unsigned RAMP_LIMIT_LOW_31_16 : 16; // D300h
            unsigned RAMP_LIMIT_LOW_15_0 : 16; // 0000h (7'834'959'872)
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            // R90
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            unsigned : 2, RAMP_BURST_COUNT : 13, RAMP_BURST_EN : 1;
            unsigned RAMP_BURST_TRIG : 2, : 1, RAMP_TRIGA : 4, RAMP_TRIGB : 4, : 4, RAMP0_RST : 1; // RAMP_TRIGA=1, RAMP_TRIGB=1
            unsigned RAMP0_DLY : 1, : 1, RAMP0_INC_29_16 : 14; // RAMP0_INC_29_16=80h
            unsigned RAMP0_INC_15_0 : 16; // RAMP0_INC_15_0=0h (8'388'608)
            // R100
            unsigned RAMP0_LEN : 16;
            unsigned RAMP0_NEXT_TRIG : 2, : 2, RAMP0_NEXT : 1, RAMP1_RST : 1, RAMP1_DLY : 1, : 9; // RAMP0_NEXT_TRIG=1, RAMP0_NEXT=1
            unsigned RAMP1_INC_29_16 : 14, : 2; // RAMP1_INC_29_16=3F80h
            unsigned RAMP1_INC_15_0 : 16; // RAMP1_INC_15_0=0000h (1'065'353'216)
            unsigned RAMP1_LEN : 16;
            unsigned RAMP1_NEXT_TRIG : 2, : 2, RAMP1_NEXT : 1, RAMP_MANUAL : 1, RAMP_DLY_CNT : 10; // RAMP1_NEXT_TRIG=1
            unsigned RAMP_SCALE_COUNT : 3, : 1, RAMP_TRIG_CAL : 1, : 11;
            unsigned : 16;
            unsigned : 16;
            unsigned : 16;
            // R110
            unsigned : 5, rb_VCO_SEL : 3, : 1, rb_LD_VTUNE : 2, : 5;
            unsigned rb_VCO_CAPCTRL : 8, : 8;
            unsigned rb_VCO_DACISET : 9, : 7;
        };
#pragma pack(pop)
    } regs;
};
