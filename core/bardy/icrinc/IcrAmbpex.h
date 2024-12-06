//****************** File icrAmbpex.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей AMBPEX1/AMBPEX8
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on AMBPEX1/AMBPEX8 base modules
//
//	Copyright (c) 2007, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  26-03-07 - builded
//
//*******************************************************************

#ifndef __ICRAMBPEX_H_
 #define __ICRAMBPEX_H_

#pragma pack(push, 1)    

const USHORT AMBPEX1_CFG_TAG = 0x5502; // тэг для структуры конфигурационных параметров базового модуля AMBPEX1
const USHORT AMBPEX8_CFG_TAG = 0x5503; // тэг для структуры конфигурационных параметров базового модуля AMBPEX8

// AMBPEX1/AMBPEX8 modules configuration
// Конфигурационные параметры базовых модулей AMBPEX1/AMBPEX8
typedef struct _ICR_CfgAmbpex {
	USHORT	wTag;			// тэг структуры (AMBPEX1_CFG_TAG или AMBPEX8_CFG_TAG)
	USHORT	wSize;			// размер всех следующих полей структуры
	UCHAR	bAdmIfCnt;		// количество интерфейсов ADM
	ULONG	dSysGen;		// System generator in Hz (default 66 MHz)
	UCHAR	bDdsType;		// type of DDS (0-non, 1-PLL, 2-DDS)
	UCHAR	bSramCfgCnt;	// количество конфигурационных структур статической памяти SRAM
//	UCHAR	bSdramSlotCnt;	// количество установленных слотов динамической памяти
//	UCHAR	bSdramCfgCnt;	// количество конфигурационных структур динамической памяти
//	UCHAR	bDspNodeCfgCnt;	// количество конфигурационных структур узла цифровой обработки сигналов
} ICR_CfgAmbpex, *PICR_CfgAmbpex, ICR_Cfg5502, *PICR_Cfg5502, ICR_Cfg5503, *PICR_Cfg5503;

#pragma pack(pop)    

#endif // __ICRAMBPEX_H_

// ****************** End of file icrAmbpex.h **********************
