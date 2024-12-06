//****************** File icrAdp201x1.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей ADP201X1
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//  on ADP201X1 base modules
//
//	Copyright (c) 2009, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  01-09-09 - builded
//
//*******************************************************************

#ifndef __ICRADP201X1_H_
 #define __ICRADP201X1_H_

#pragma pack(push, 1)    

const USHORT ADP201X1AMB_CFG_TAG = 0x5504; // тэг для структуры конфигурационных параметров базового модуля ADP201X1_AMB
const USHORT ADP201X1DSP_CFG_TAG = 0x5505; // тэг для структуры конфигурационных параметров базового модуля ADP201X1_DSP

// ADP201X1 modules configuration
// Конфигурационные параметры базовых модулей ADP201X1
typedef struct _ICR_CfgAdp201x1 {
	U16		wTag;			// тэг структуры (ADP201X1AMB_CFG_TAG или ADP201X1DSP_CFG_TAG)
	U16		wSize;			// размер всех следующих полей структуры
	U08		bAdmIfCnt;		// количество интерфейсов ADM
	U32		dSysGen;		// System generator in Hz (default 250 MHz)
	U08		bDdsType;		// type of DDS (0-non, 1-DDS5, 2-DDS60)
	U16		wSizeOfPldSdram;// Размер SDRAM при ПЛИС в Мбайтах 0, 1Gb, 2Gb
	U08		bLlcuConnector;	// наличие разъема LLCU (вместо субмодуля)
	U32		dRefMgtGen;		// Reference MGT generator in Hz (default 156.25 MHz)
	U32		dRefCpuGen;		// Reference CPU generator in Hz (default 100 MHz)
	U08		bIsCpu;			// наличие CPU
	U16		wCpuClock;		// CPU clock (MHz) - 500 or 600
	U16		wSizeOfCpuSdram;// Размер SDRAM при процессоре в Мбайтах 0, 128Mb, 256Mb, 512Mb
	U16		wSizeOfCpuFlash;// Размер FLASH при процессоре в кбайтах 0, 2Mb, 4Mb
} ICR_CfgAdp201x1, *PICR_CfgAdp201x1, ICR_Cfg5504, *PICR_Cfg5504, ICR_Cfg5505, *PICR_Cfg5505;

#pragma pack(pop)    

#endif // __ICRADP201X1_H_

// ****************** End of file icrAdp201x1.h **********************
