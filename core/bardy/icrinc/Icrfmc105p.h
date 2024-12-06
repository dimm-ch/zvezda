//****************** File icrFmc105p.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей FMC105P
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on FMC105P base modules
//
//	Copyright (c) 2012, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  15-03-12 - builded
//
//*******************************************************************

#ifndef __ICRFMC105P_H_
 #define __ICRFMC105P_H_

#pragma pack(push, 1)    

const USHORT FMC105P_CFG_TAG = 0x5509; // тэг для структуры конфигурационных параметров базового модуля FMC105P
const USHORT FMC106P_CFG_TAG = 0x550A; // тэг для структуры конфигурационных параметров базового модуля FMC106P
const USHORT FMC107P_CFG_TAG = 0x5511; // тэг для структуры конфигурационных параметров базового модуля FMC107P
const USHORT FMC103E2_CFG_TAG = 0x550B; // тэг для структуры конфигурационных параметров базового модуля FMC103E2
const USHORT FMC114V_CFG_TAG = 0x550C; // тэг для структуры конфигурационных параметров базового модуля FMC114V
const USHORT FMC110P_CFG_TAG = 0x550D; // тэг для структуры конфигурационных параметров базового модуля FMC110D
const USHORT FMC113E_CFG_TAG = 0x550E; // тэг для структуры конфигурационных параметров базового модуля FMC113E
const USHORT FMC108V_CFG_TAG = 0x550F; // тэг для структуры конфигурационных параметров базового модуля FMC108V
const USHORT FMC115CP_CFG_TAG = 0x53B1; // тэг для структуры конфигурационных параметров базового модуля FMC115cP
//const USHORT FMC112CP_CFG_TAG = 0x53B2; // тэг для структуры конфигурационных параметров базового модуля FMC112cP

// FMC105P/FMC106P modules configuration
// Конфигурационные параметры базовых модулей FMC105P/FMC106P
typedef struct _ICR_CfgFmc105p {
	U16	wTag;			// тэг структуры (FMC105P_CFG_TAG или FMC106P_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08	bAdmIfCnt;		// количество интерфейсов ADM
	U32	dSysGen;		// System generator in Hz (default 250 MHz)
	U08	bDdsType;		// тип синтезатора DDS (0-non, 1-above 50MHz, 2-below 50MHz)
	U08	bSwitchType;	// тип коммутатора (0-non, 1-type1(FMC105P), 2-type2(FMC106P))
	U08	bAdrSwitch;		// адресный код коммутатора: 0x48 по умолчанию
	U08	bGen0Type;		// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si571)
	U32	nRefGen0;		// частота генератора 0, если он непрограммируемый, или заводская частота (default 50 MHz)
	U32	nRefMaxGen0;	// максимальная частота внутр. генератора (Гц)
	U08	bAdrGen0;		// адресный код внутр. генератора: 0x49 по умолчанию
	U32	dRefGen5;		// Reference generator 5 in Hz (default 156.25 MHz)
	U32	dRefGen6;		// Reference generator 6 in Hz (default 100 MHz)
	U08	bDspNodeCfgCnt;	// количество конфигурационных структур узла цифровой обработки сигналов
} ICR_CfgFmc105p, *PICR_CfgFmc105p, ICR_Cfg5509, *PICR_Cfg5509;

#pragma pack(pop)    

#endif // __ICRFMC105P_H_

// ****************** End of file icrFmc105p.h **********************
