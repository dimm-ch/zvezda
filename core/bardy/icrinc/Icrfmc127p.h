//****************** File icrFmc127p.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей FMC127P
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on FMC105P base modules
//
//	Copyright (c) 2015, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  27-11-15 - builded
//
//*******************************************************************

#ifndef __ICRFMC127P_H_
 #define __ICRFMC127P_H_

#pragma pack(push, 1)    

const USHORT FMC127P_CFG_TAG = 0x551F; // тэг для структуры конфигурационных параметров базового модуля FMC127P

// FMC127P module configuration
// Конфигурационные параметры базового модуля FMC127P
typedef struct _ICR_CfgFmc127p {
	U16	wTag;			// тэг структуры (FMC127P_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08	bAdmIfCnt;		// количество интерфейсов ADM
	U32	dSysGen;		// System generator in Hz (default 250 MHz)
	U08	bDdsType;		// тип синтезатора DDS (0-non) - поле присутствует для совместимости
	U08	bSwitchType;	// тип коммутатора (0-non, 1-type6(FMC127P))
	U08	bAdrSwitch;		// адресный код коммутатора: неприменимо (поле присутствует для совместимости)
	U08	bGen0Type;		// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si570)
	U32	nRefGen0;		// частота генератора 0, если он непрограммируемый, или заводская частота (default 50 MHz)
	U32	nRefMaxGen0;	// максимальная частота внутр. генератора (Гц)
	U08	bAdrGen0;		// адресный код внутр. генератора: 0x49 по умолчанию
	U08 bIsFMC2;		// Установлен ли слот FMC2
	U08	bIsSynx2;		// Установлен ли интерфейс SYNX2	
	U08	bIsUserEEPROM;	// Установлено ли пользовательское ППЗУ
	U08	bIsPowerCtrl;	// Установлен ли контроллер питания
	U08	bAdrPower;		// адресный код контроллера питания: 0x48 по умолчанию
	U08 bIsInternalDDR3;	// Установлена ли встроенная память DDR3 128x32 Мслов
} ICR_CfgFmc127p, *PICR_CfgFmc127p, ICR_Cfg551F, *PICR_Cfg551F;

#pragma pack(pop)    

#endif // __ICRFMC127P_H_

// ****************** End of file icrFmc127p.h **********************
