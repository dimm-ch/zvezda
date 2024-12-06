//****************** File icrAmb3us.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базового модуля AMB3UV
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on AMB3UV base module
//
//	Copyright (c) 2007, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//
//*******************************************************************
#ifndef __ICRAMB3UV_H
 #define __ICRAMB3UV_H

#pragma pack(push, 1)    

const USHORT AMB3UV_CFG_TAG = 0x5467; // тэг для структуры конфигурационных параметров базового модуля AMB3UV

// AMB3UV module configuration
// Конфигурационная структура базового модуля AMB3UV
typedef struct _ICR_Cfg5467 {
	USHORT	wTag;			// тэг структуры (AMB3UV_CFG_TAG)
	USHORT	wSize;			// размер всех следующих полей структуры
	UCHAR	bAdmIfCnt;		// количество интерфейсов ADM = 1
	ULONG	dSysGen;		// System generator in Hz (default 100 MHz)
	UCHAR	bDdsType;		// type of DDS (0-non, 1-PLL, 2-DDS)
	UCHAR	bDspNodeCfgCnt;	// количество конфигурационных структур узла цифровой обработки сигналов
} ICR_Cfg5467, *PICR_Cfg5467, ICR_CfgAmb3uv, *PICR_CfgAmb3uv;

#pragma pack(pop)    

#endif	// __ICRAMB3UV_H

// ****************** End of file icrAmb3uv.h **********************
