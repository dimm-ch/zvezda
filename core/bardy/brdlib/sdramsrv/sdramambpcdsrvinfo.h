//****************** File SdramAmbpcdSrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы SDRAM базового модуля AMBPCD
//
//  Constants, structures & functions definitions
//	for API Property Dialog of SDRAM service of AMBPCD base module
//
//	Copyright (c) 2002-2005, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  14-03-05 - builded
//
//*******************************************************************************

#ifndef _SDRAMAMBPCDSRVINFO_H
 #define _SDRAMAMBPCDSRVINFO_H

#pragma pack(1)

// Struct info about device
typedef struct _SDRAMAMBPCDSRV_INFO {
	USHORT		Size;				// sizeof(SDRAMAMBPCDSRV_INFO)
	PSDRAMSRV_INFO pSdramInfo;
	UCHAR	PrimWidth;		// Primary DDR SDRAM Width
	UCHAR	CasLatency;		// задержка сигнала выборки по столбцам
	UCHAR	Attributes;		// bit 1 = 1 - registered memory
} SDRAMAMBPCDSRV_INFO, *PSDRAMAMBPCDSRV_INFO;

#pragma pack()

#endif // _SDRAMAMBPCDSRVINFO_H

// ****************** End of file SdramAmbpcdSrvInfo.h ***************

