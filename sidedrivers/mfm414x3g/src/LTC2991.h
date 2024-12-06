///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#pragma once
#include <cstdint>
#include <functional>

class LTC2991 {
    static constexpr auto name_class = "LTC2991"; // имя класса для отладочных сообщений

    using spi_read_t = std::function<uint8_t(size_t)>;
    using spi_write_t = std::function<void(size_t, uint8_t)>;

    // функции SPI чтения/записи регистров
    spi_read_t spi_read;
    spi_write_t spi_write;

public:
    LTC2991();
    ~LTC2991() = default;

    auto setup_spi(spi_read_t r, spi_write_t w) -> void
    {
        spi_read = r;
        spi_write = w;
    };
    auto reset() -> void;

    using Data = struct {
        float Tint; ///< Температура чипа
        float V1; ///< Напряжение цепи V1
        float V2; ///< Напряжение цепи V2
        float V3; ///< Напряжение цепи V3
        float V4; ///< Напряжение цепи V4
        float V5; ///< Напряжение цепи V5
        float V6; ///< Напряжение цепи V6
        float V7; ///< Напряжение цепи V7
        float V8; ///< Напряжение цепи V8
    };
    auto get_data() -> Data;
};
