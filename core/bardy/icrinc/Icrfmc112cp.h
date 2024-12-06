//****************** File icrFmc112cp.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей FMC112cP, FMC117cP
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on FMC112cP, FMC117cP base modules
//
//	Copyright (c) 2014, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  06-06-14 - builded
//
//*******************************************************************

#ifndef __ICRFMC112CP_H_
 #define __ICRFMC112CP_H_

#pragma pack(push, 1)    

const USHORT FMC112CP_CFG_TAG = 0x53B2; // тэг для структуры конфигурационных параметров базового модуля FMC112cP
const USHORT FMC117CP_CFG_TAG = 0x53B3; // тэг для структуры конфигурационных параметров базового модуля FMC117cP

// FMC112cP/FMC117cP modules configuration
// Конфигурационные параметры базовых модулей FMC112cP/FMC117cP
typedef struct _ICR_CfgFmc112cp {
	U16	wTag;			// тэг структуры (FMC112CP_CFG_TAG, FMC117CP_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08	bAdmIfCnt;		// количество интерфейсов ADM
	U32	dSysGen;		// System generator in Hz (default 250 MHz)
	U08	bDdsType;		// тип синтезатора DDS (0-non, 1-above 50MHz, 2-below 50MHz)
	U08	bSwitchType;	// тип коммутатора (0-non, 1-type3(FMC112cP))
	U08	bAdrSwitch;		// адресный код коммутатора: 0x48 по умолчанию
	U08	bGen0Type;		// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si571)
	U32	nRefGen0;		// частота генератора 0, если он непрограммируемый, или заводская частота (default 50 MHz)
	U32	nRefMaxGen0;	// максимальная частота внутр. генератора (Гц)
	U08	bAdrGen0;		// адресный код внутр. генератора: 0x49 по умолчанию
	U32	dRefGen5;		// MGT Reference generator 5 in Hz (default 156.25 MHz)
	U32	dRefGen6;		// Reference generator 6 in Hz (default 100 MHz)
	U08	bDspNodeCfgCnt;	// количество конфигурационных структур узла цифровой обработки сигналов
	U08	isDDR3;			// Наличие памяти DDR3
} ICR_CfgFmc112cp, *PICR_CfgFmc112cp, ICR_Cfg53B2, *PICR_Cfg53B2;

#pragma pack(pop)    

#endif // __ICRFMC112CP_H_

// ****************** End of file icrFmc112cp.h **********************
