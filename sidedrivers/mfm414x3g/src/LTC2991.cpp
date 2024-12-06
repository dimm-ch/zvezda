///
/// \author Alexander U.Jurankov (devlab@insys.ru)
/// \brief
///

#include <thread>
using namespace std::chrono_literals;
#include <cmath>

#include "LTC2991.h"

// #ifndef NDEBUG
// #define NDEBUG // отключение любой отладочной информации для этого класса
// #endif
#include "log.h"

///
/// \brief      Конструктор с debug-версией функций SPI.
///
LTC2991::LTC2991()
    : spi_read([](size_t reg) { LOG("R [%04zX] : 0x%02X", reg, 0); return 0; })
    , spi_write([](size_t reg, uint8_t val) { LOG("W [%04zX] : 0x%02X", reg, val); })
{
}

///
/// \brief      Перезапуск чипа
///
/// \return     void
///
auto LTC2991::reset() -> void
{
    LOG("Reset");
    spi_write(0x01, 0b1100'1000); // Enable Tint, V5-V6, V7-V8
    spi_write(0x08, 0b0001'0000); // Enable Repeated Mode
}

///
/// \brief      Получение данных температуры и трёх напряжений.
///
/// \return     Структура данных `Data`.
///
auto LTC2991::get_data() -> Data
{
    auto result = Data {};

    // значение температуры делим на 16, получаем градацию 0.0625
    result.Tint = ((spi_read(0x1A) & 0b0001'1111) << 8 | spi_read(0x1B)) / 16.0;

    // значение напряжения умножаем на 305.18µV
    result.V6 = ((spi_read(0x14) & 0b0011'1111) << 8 | spi_read(0x15)) * 0.00030518;
    result.V7 = ((spi_read(0x16) & 0b0011'1111) << 8 | spi_read(0x17)) * 0.00030518;
    result.V8 = ((spi_read(0x18) & 0b0011'1111) << 8 | spi_read(0x19)) * 0.00030518;

    return result;
}
