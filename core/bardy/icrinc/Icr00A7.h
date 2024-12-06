#ifndef _ICR00A7_H
#define _ICR00A7_H

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

#include <cstdint>

#define ADM_CFG_TAG 0x00A7

/// Конфигурационные параметры субмодуля
typedef struct _ICR_Cfg00A7 {
  uint16_t wTag;       ///< Тэг структуры субмодуля
  uint16_t wSize;      ///< Размер последующих данных
  uint8_t bAdmIfNum;   ///< Номер интерфейса ADM
  uint8_t bClkOutCnt;  ///< Количество выходов CLK_OUT
  uint8_t bIntGenType;  ///< Тип кристалла внутреннего генератора:
                        ///< 0 - непрограммируемый,
                        ///< 1 - MXO37,
                        ///< 2 - P213,
                        ///< 3 - Si57x
                        ///< 4 - CB3
  uint8_t bIntGenAdr;  ///< Адресный код внутреннего генератора: 0x49, 0x55
  uint32_t nIntGenRef;  ///< Заводское значение частоты генератора (Гц)
  uint32_t nIntGenRefMax;  ///< Максимальное значение частоты генератора (Гц)
  uint32_t nThdacRange;  ///< шкала преобразования ЦАП (мВ)
  uint8_t bPllType;  ///<  Тип кристалла PLL: 0 - нет, 1 - LMX2594
  uint16_t wRshunt;  ///< Сопротивление шунта (мОм).
  uint8_t bFMCType;  ///< Вариант исполнения интерфейсного разъема: 0 - LPC, 1 - HPC.
  uint16_t wCxoDacVal; ///< Корректировка генератора CXO в отсчетах (0 ... 65535)
} ICR_Cfg00A7, *PICR_Cfg00A7, ICR_CfgAdm, *PICR_CfgAdm;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif  // _ICR00A7_H
