//****************** File Dac16224x192SrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы ЦАП базового модуля
//
//  Constants, structures & functions definitions
//	for API Property Dialog of DAC1624x192 service
//
//	Copyright (c) 2002-2005, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  09-06-07 - builded
//
//*******************************************************************************

#ifndef _DAC1624X192SRVINFO_H
 #define _DAC1624X192SRVINFO_H

//#define MAX_CHAN 32

#define SINGLE_DIVIDER 512
//#define QUAD_DIVIDER 128

//#define DAC_MAX_SINGLE_RATE 50000.
//#define DAC_MAX_DOUBLE_RATE 100000.

#pragma pack(1)

// Struct info about device
typedef struct _DAC1624x192SRV_INFO {
	USHORT		Size;					// sizeof(DAC1624x192SRV_INFO)
	PDACSRV_INFO pDacInfo;
	ULONG		Mute;
} DAC1624x192SRV_INFO, *PDAC1624x192SRV_INFO;

#pragma pack()

#endif // _DAC1624X192SRVINFO_H

// ****************** End of file Dac1624x192SrvInfo.h ***************
