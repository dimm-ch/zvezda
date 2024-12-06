//****************** File Ddc4x16SrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы ЦАП базового модуля
//
//  Constants, structures & functions definitions
//	for API Property Dialog of DDC4x16 service
//
//	Copyright (c) 2002-2005, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  26-01-06 - builded
//
//*******************************************************************************

#ifndef _DDC4X16SRVINFO_H
 #define _DDC4X16SRVINFO_H

//#define MAX_CHAN 16

#pragma pack(1)

// Struct info about device
typedef struct _DDC4X16SRV_INFO {
	USHORT		Size;					// sizeof(DDC4x16SRV_INFO)
	PDDCSRV_INFO pDdcInfo;
	//UCHAR		DblClockMode;			// режим удвоения тактовой частоты
	//ULONG		RefGen;					// frequency of generators (значения опорных генераторов (Гц))
	//double		ExtClk;					// any external frequency of clock (любая из внешних частот тактирования (Гц))
	//double		PhaseTuning[MAX_CHAN];	// подстройка фазы тактового сигнала канала 0
	//double		GainTuning0;			// подстройка усиления канала 0
	//UCHAR		Inp0Ch1;				// 1 - к каналу 1 подключен вход 0
	//ULONG		StartSrc;				// источник старта
	//UCHAR		InvStart;				// инверсия сигнала старта
	//double		StartLevel;				// уровень сигнала старта
} DDC4X16SRV_INFO, *PDDC4X16SRV_INFO;

#pragma pack()

#endif // _DDC4X16SRVINFO_H

// ****************** End of file Ddc4x16SrvInfo.h ***************
