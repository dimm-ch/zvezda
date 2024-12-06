//****************** File icrAmbp.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей AMBPCD/AMBPCX
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on AMBPCD/AMBPCX base modules
//
//	Copyright (c) 2004, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  05-10-04 - builded
//  14-10-04 - add AMBPCD
//  20-10-04 - add dSysGen
//
//*******************************************************************

#ifndef __ICRAMBP_H_
 #define __ICRAMBP_H_

#pragma pack(push, 1)    

const USHORT AMBPCD_CFG_TAG = 0x4D44; // тэг для структуры конфигурационных параметров базового модуля AMBPCD
const USHORT AMBPCX_CFG_TAG = 0x4D58; // тэг для структуры конфигурационных параметров базового модуля AMBPCX
const USHORT AMBPEX2_CFG_TAG = 0x5508; // тэг для структуры конфигурационных параметров базового модуля AMBPEX2

// AMBPCD/AMBPCX modules configuration
// Конфигурационные параметры базовых модулей AMBPCD/AMBPCX
typedef struct _ICR_CfgAmbp {
	USHORT	wTag;			// тэг структуры (AMBPCX_CFG_TAG или AMBPCD_CFG_TAG)
	USHORT	wSize;			// размер всех следующих полей структуры
	UCHAR	bAdmIfCnt;		// количество интерфейсов ADM
	ULONG	dSysGen;		// System generator in Hz (default 66 MHz)
	UCHAR	bSdramSlotCnt;	// количество установленных слотов динамической памяти
	UCHAR	bSdramCfgCnt;	// количество конфигурационных структур динамической памяти
	UCHAR	bDspNodeCfgCnt;	// количество конфигурационных структур узла цифровой обработки сигналов
} ICR_CfgAmbp, *PICR_CfgAmbp, ICR_Cfg4d58, *PICR_Cfg4d58, ICR_CfgAmbpcx, *PICR_CfgAmbpcx, ICR_Cfg4d44, *PICR_Cfg4d44, ICR_CfgAmbpcd, *PICR_CfgAmbpcd;

#pragma pack(pop)    

#endif // __ICRAMBP_H_

// ****************** End of file IcrAmbp.h **********************
