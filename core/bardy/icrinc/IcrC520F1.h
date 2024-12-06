//****************** File icrc520f1.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей Ц520Ф1-АЦП (C520F1)
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on FMC112cP base modules
//
//	Copyright (c) 2015, Instrumental Systems,Corp.
//	Written by Ekkore
//
//  History:
//  25-02-15 - builded
//
//*******************************************************************

#ifndef __ICRC520F1_H_
 #define __ICRC520F1_H_

#pragma pack(push, 1)    

const USHORT C520F1_CFG_TAG = 0x53A3; // тэг для структуры конфигурационных параметров базового модуля Ц520Ф1-АЦП

// C520F1 modules configuration
// Конфигурационные параметры базовых модулей Ц520Ф1-АЦП
typedef struct _ICR_CfgC520F1 {
	U16	wTag;			// тэг структуры (C520F1_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08	bAdmIfCnt;		// количество интерфейсов ADM
	U08	reserve;			// резерв
	U08	bGen0Type;		// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si571)
	U08	bAdrGen0;		// адресный код внутр. генератора: 0x49 по умолчанию
	U32	nRefGen0;		// частота генератора 0, если он непрограммируемый, или заводская частота (default 50 MHz)
	U32	nRefMaxGen0;	// максимальная частота внутр. генератора (Гц)
} ICR_CfgC520F1, *PICR_CfgC520F1, ICR_Cfg53A3, *PICR_Cfg53A3;

#pragma pack(pop)    

#endif // __ICRC520F1_H_

// ****************** End of file icrc520f1.h **********************
