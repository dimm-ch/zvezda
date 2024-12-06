///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief Класс микросхемы AD9208.
///

#pragma once
#include <cstdint>
#include <functional>

class AD9208 {
    static constexpr auto name_class = "AD9208"; // имя класса для отладочных сообщений

    using spi_read_t = std::function<uint8_t(size_t)>;
    using spi_write_t = std::function<void(size_t, uint8_t)>;

    // функции SPI чтения/записи регистров
    spi_read_t spi_read;
    spi_write_t spi_write;

    double freq_clk = 0.0; // частота тактирования АЦП в МГц
    bool complex = false; // признак комплексного IQ сигнала на выходе
    double sample_rate = 0.0; // частота оцифрованного потока в МГц, с учётом децимации
    double lane_rate = 0.0; // частота потока на линиях JESD'а
    size_t decimate = 1; // децимация
    size_t jesd_M = 2; // число виртуальных конверторов [1,2,4,8]: 1 = 1 канал АЦП; 1 = 1 канал Real DDC; 2 = 1 канал Complex DDC
    size_t jesd_L = 8; // число используемых линий
    size_t jesd_F = 2; // число Octets в фрейме

public:
    AD9208();
    ~AD9208() = default;

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
    };
    auto get_chip_id() -> ChipID;

    auto is_complex() -> bool { return complex; }
    auto get_jesd_M() -> size_t { return jesd_M; }
    auto get_jesd_L() -> size_t { return jesd_L; }
    auto get_jesd_F() -> size_t { return jesd_F; }
    auto set_fclk(double value) -> void { freq_clk = value; }
    auto get_fclk() -> double { return freq_clk; }
    auto get_sample_rate() -> double { return sample_rate; }
    auto get_lane_rate() -> double { return lane_rate; }

    auto set_mode(size_t app_mode = 0, size_t decimate = 1) -> void;
    auto set_jesd(double lane_rate_max) -> bool;
    auto set_nco(size_t channel, double value) -> void;
    auto get_nco(size_t channel) -> double;
    auto set_nco_offset(size_t channel, double value) -> void;
    auto set_ddc_input_iq(size_t channel, size_t source_i, size_t source_q) -> bool;
    auto set_invert_channel_a() -> void;
    auto set_input_fs_voltage(size_t channel, double voltage) -> void;
    auto set_input_buffer_control(size_t channel, size_t value) -> void;
    auto set_clock_delay_control(uint8_t value) -> void;
    auto set_clock_delay(size_t channel, uint8_t divider_phase, uint8_t superfine_delay, uint8_t fine_delay) -> void;
    auto set_fast_detect(size_t channel, bool enable, double upper, double lower, uint16_t time) -> void;
    auto set_prbs(uint16_t mode) -> void;
    auto set_time_stamp(bool enable, bool polarity_negative) -> void;

    // -------------------------------------------------------------------------
    // Регистры
    // -------------------------------------------------------------------------
    // #define AD9208_REG(name, addr, ...) static constexpr auto name = addr; using name##_VALUE = union { uint8_t all; struct { uint8_t __VA_ARGS__ ;};}
    // AD9208_REG(NVM_LOADER_EN, 0x705, NVM_BLR_EN:1, NVM_BLR_DONE:1);

    using DDC_0x310 = union {
        uint8_t all;
        struct {
            uint8_t RATE : 3, REAL : 1, IF_MODE : 2, GAIN : 1, MIXER : 1;
        };
    };
    using DDC_0x311 = union {
        uint8_t all;
        struct {
            uint8_t I_SEL : 1, : 1, Q_SEL : 1, : 1, RATE : 4;
        };
    };
};
