///
/// \file ctrl_sync.h
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 13.01.2020
///
/// \copyright InSys Copyright (c) 2020
///
///

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "ctrladmpro.h"

/// Команды управления
typedef enum _BRDctrl_SYNC : uint32_t {
    /// Запрос конфигурации модуля (BRD_SyncCfg)
    BRDctrl_SYNC_GETCFG = BRDctrl_SYNC_SECTION,
    /// Управление режимом работы схемы тактирования (BRD_SyncClkMode)
    BRDctrl_SYNC_SETCLKMODE,
    /// Запрос параметров режима работы схемы тактирования (BRD_SyncClkMode)
    BRDctrl_SYNC_GETCLKMODE,
    /// Управление режимом работы схемы формирования строба (BRD_SyncStbMode)
    BRDctrl_SYNC_SETSTBMODE,
    /// Запрос параметров режима работы схемы формирования строба (BRD_SyncStbMode)
    BRDctrl_SYNC_GETSTBMODE,
    /// Управление фазой сигнала перетактирования (BRD_SyncInvClkPhs)
    BRDctrl_SYNC_SETINVCLKPHS,
    /// Запрос фазы сигнала перетактирования (BRD_SyncInvClkPhs)
    BRDctrl_SYNC_GETINVCLKPHS,
    /// Управление режимом работы схемы формирования строба (BRD_SyncStartMode)
    BRDctrl_SYNC_SETSTARTMODE,
    /// Запрос параметров режима работы схемы формирования строба (BRD_SyncStartMode)
    BRDctrl_SYNC_GETSTARTMODE,
    /// Управление параметрами выхода схемы тактирования (BRD_SyncOutClk)
    BRDctrl_SYNC_SETOUTCLK,
    /// Запрос параметров выхода схемы тактирования (BRD_SyncOutClk)
    BRDctrl_SYNC_GETOUTCLK,
    /// Управление параметрами выхода схемы стробирования (BRD_SyncOutStb)
    BRDctrl_SYNC_SETOUTSTB,
    /// Запрос параметров выхода схемы стробирования (BRD_SyncOutStb)
    BRDctrl_SYNC_GETOUTSTB,
    /// Управление задержкой выхода схемы стробирования (BRD_SyncDelOutStb)
    BRDctrl_SYNC_SETDELOUTSTB,
    /// Запрос задержки выхода схемы стробирования (BRD_SyncDelOutStb)
    BRDctrl_SYNC_GETDELOUTSTB,
    /// Вкл. формирования тактовых импульсов
    BRDctrl_SYNC_STARTCLOCK,
    /// Выкл. формирования тактовых импульсов
    BRDctrl_SYNC_STOPCLOCK,
    /// Вкл. формирования стробирующих импульсов
    BRDctrl_SYNC_STARTSTROBE,
    /// Выкл. формирования стробирующих импульсов
    BRDctrl_SYNC_STOPSTROBE,
    /// Получение температуры субмодуля
    BRDctrl_SYNC_GETTEMP,
    /// Получение текущих значений токов и напряжений субмодуля
    BRDctrl_SYNC_GETSYSMON,
    /// Получение текущего шага изменения стробирующих импульсов
    BRDctrl_SYNC_GETSTBRES,
    /// Подстройка частоты генератора CXO
    BRDctrl_SYNC_ADJCXO,
    /// Недопустимая команда
    BRDctrl_SYNC_ILLEGAL
} BRDctrl_SYNC;

/// Источник тактового сигнала
typedef enum _BRDclksrc_SYNC : uint32_t {
    /// Тактирование выключено
    BRDclksrc_SYNC_DISABLED,
    /// Внешний источник тактирования
    BRDclksrc_SYNC_EXT,
    /// Тактирование от генератора на субмодуле
    BRDclksrc_SYNC_INT,
    /// Тактирование от PLL (внешний опорный генератор)
    BRDclksrc_SYNC_EXTPLL,
    /// Тактирование от PLL (опорный генератор субмодуля)
    BRDclksrc_SYNC_INTPLL
} BRDclksrc_SYNC;

/// Делитель внешнего тактового сигнала
typedef enum _BRDclkdiv_SYNC : uint32_t {
    BRDclkdiv_SYNC_DISABLE = 1, ///< Делитель отключен
    BRDclkdiv_SYNC_DIV2, ///< Делитель на 2
    BRDclkdiv_SYNC_DIV3, ///< Делитель на 3
    BRDclkdiv_SYNC_DIV4, ///< Делитель на 4
    BRDclkdiv_SYNC_DIV8 ///< Делитель на 8
} BRDclkdiv_SYNC;

/// Источник сигнала стробирования
typedef enum _BRDstbsrc_SYNC : uint32_t {
    BRDstbsrc_SYNC_START1 = (1 << 0), ///< Сигнал START1 от ПЛИС
    BRDstbsrc_SYNC_START2 = (1 << 1), ///< Сигнал START2 от ПЛИС
    BRDstbsrc_SYNC_START3 = (1 << 2), ///< Сигнал START3 от ПЛИС
    BRDstbsrc_SYNC_START4 = (1 << 3), ///< Сигнал START4 от ПЛИС
    BRDstbsrc_SYNC_ALL = 0x0F, ///< Все
    BRDstbsrc_SYNC_NONE = 0x00 ///< Ни один
} BRDstbsrc_SYNC;

/// Источник сигнала старта
typedef enum _BRDstsrc_SYNC : uint32_t {
    BRDstsrc_SYNC_EXT, ///< Внешний источник
    BRDstsrc_SYNC_INT, ///< Внутренний источник
} BRDstsrc_SYNC;

/// Состояние выхода
typedef enum _BRDenb_SYNC : uint32_t {
    BRDenb_SYNC_ENABLE = 1, ///< Вкл.
    BRDenb_SYNC_DISABLE = 0, ///< Выкл.
} BRDenb_SYNC;

/// Пользовательские выходы схем тактирования
typedef enum _BRDclkout_SYNC : uint32_t {
    BRDclkout_SYNC_OUT1 = (1 << 0), ///< Выход 1
    BRDclkout_SYNC_OUT2 = (1 << 1), ///< Выход 2
    BRDclkout_SYNC_OUT3 = (1 << 2), ///< Выход 3
    BRDclkout_SYNC_OUT4 = (1 << 3), ///< Выход 4
    BRDclkout_SYNC_OUT5 = (1 << 4), ///< Выход 5
    BRDclkout_SYNC_OUT6 = (1 << 5), ///< Выход 6
    BRDclkout_SYNC_OUT7 = (1 << 6), ///< Выход 7
    BRDclkout_SYNC_OUT8 = (1 << 7), ///< Выход 8
    BRDclkout_SYNC_ALL = 0xFF, ///< Все выходы
    BRDclkout_SYNC_NONE = 0x00 ///< Ни одного выхода
} BRDclkout_SYNC;

/// Пользовательские выходы схем  стробирования
typedef enum _BRDstbout_SYNC : uint32_t {
    BRDstbout_SYNC_OUT1 = (1 << 0), ///< Выход 1
    BRDstbout_SYNC_OUT2 = (1 << 1), ///< Выход 2
    BRDstbout_SYNC_OUT3 = (1 << 2), ///< Выход 3
    BRDstbout_SYNC_OUT4 = (1 << 3), ///< Выход 4
    BRDstbout_SYNC_OUT5 = (1 << 4), ///< Выход 5
    BRDstbout_SYNC_OUT6 = (1 << 5), ///< Выход 6
    BRDstbout_SYNC_ALL = 0x3F, ///< Все выходы
    BRDstbout_SYNC_NONE = 0x00 ///< Ни одного выхода
} BRDstbout_SYNC;

#pragma pack(push, 1)

/// Конфигурация модуля
typedef struct _BRD_SyncCfg {
    uint32_t clkNum; ///< Число каналов тактирования
    uint32_t stbNum; ///< Число каналов стробирования
    uint32_t stbSrcNum; ///< Число каналов формирования стробов
    struct {
        uint32_t major; ///< Major
        uint32_t minor; ///< Minor
    } version; ///< Версия платы
} BRD_SyncCfg, *PBRD_SyncCfg;

/// Параметры управления схемой тактирования
typedef struct _BRD_SyncClkMode {
    BRDclksrc_SYNC source; ///< Источник тактового сигнала
    BRDclkdiv_SYNC divider; ///< Делитель тактового/опорного сигнала
    double rate; ///< частота тактового сигнала (Гц)
    double reference; ///< частота опорного сигнала в режиме PLL (Гц)
} BRD_SyncClkMode, *PBRD_SyncClkMode;

/// Параметры управления выходом схемы тактирования
typedef struct _BRD_SyncOutClk {
    BRDclkout_SYNC outputs; ///< Выходы для которых устанавливаются параметры
    BRDenb_SYNC enable; ///< Состояние выхода
} BRD_SyncOutClk, *PBRD_SyncOutClk;

/// Параметры управления фазой сигнала перетактирования
typedef struct _BRD_SyncInvClkPhs {
    BRDenb_SYNC enable; // Включение сдвига фазы сигнала перетактирования на 180
} BRD_SyncInvClkPhs, *PBRD_SyncInvClkPhs;

/// Параметры управления стробами
typedef struct _BRD_SyncStbMode {
    BRDstbsrc_SYNC source; ///< Источник формирования строба для которого
        ///< устанавливаются параметры
    BRDenb_SYNC enable; ///< Состояние формирователя строба
    BRDenb_SYNC inverse; ///< Полярность строба
    uint32_t delay; ///< Начальная задержка строба
    uint32_t width; ///< Ширина строба
    uint32_t period; ///< Период повторения
    uint32_t count; ///< Число стробов
} BRD_SyncStbMode, *PBRD_SyncStbMode;

/// Параметры управления выходом схемы стробирования
typedef struct _BRD_SyncOutStb {
    BRDstbout_SYNC outputs; ///< Выходы для которых устанавливаются параметры
    BRDstbsrc_SYNC source; ///< Источник сигнала стробирования
    BRDenb_SYNC enable; ///< Состояние выхода (зарезервирован)
} BRD_SyncOutStb, *PBRD_SyncOutStb;

/// Параметры управления задердкой выходов схемы стробирования
typedef struct _BRD_SyncDelOutStb {
    BRDstbout_SYNC outputs; ///< Выходы для которых устанавливаются параметры
    uint32_t delay; ///< Задержка на выходе
} BRD_SyncDelOutStb, *PBRD_SyncDelOutStb;

/// Параметры управления стартом
typedef struct _BRD_SyncStartMode {
    BRDstsrc_SYNC source; ///< Тип источника старта
    BRDenb_SYNC inverse; ///< Инверсия старта
    BRDenb_SYNC restart; ///< Режим рестарта (не реализован)
    double level; ///< Уровень сигнала внешнего старта
    BRDenb_SYNC couple; ///< Резистор 50 Ом
} BRD_SyncStartMode, *PBRD_SyncStartMode;

/// Параметры системного монитора
typedef struct _BRD_SyncSysMon {
    struct _voltages {
        double _Adj_V;
        double _3p3_V; ///< Напряжение 3.3 В
        double _2p5_V; ///< Напряжение 2.5 В
        double _5p0_V; ///< Напряжение 5.0 В
        double _12p0_V; ///< Напряжение 12.0 В
        double _PLL_V; ///< Напряжение питания генератора PLL
        double _pCmp_V;
        double _nCmp_V;
    } voltages; ///< Значения питающих напряжений
    double current_A; ///< Ток потребления
    double power_W; ///< Потребляемая мощность
} BRD_SyncSysMon, *PBRD_SyncSysMon;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif
