//****************** File Icr0086.h ***********************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  для субмодулей 
//
//	Copyright (c) 2002-2006, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  25-01-06 - builded
//
//*********************************************************

#ifndef _ICR0086_H
 #define _ICR0086_H

#pragma pack(push, 1)    

#define ADM_CFG_TAG 0x0086

// Конфигурационные параметры субмодуля
typedef struct _ICR_Cfg0086 {
	USHORT	wTag;		// тэг структуры (ADM_CFG_TAG)
	USHORT	wSize;		// размер всех следующих полей структуры
	UCHAR	bAdmIfNum;	// номер интерфейса ADM
	ULONG	dGen;		// значение опорного генератора
	UCHAR	bAdcCnt;	// количество АЦП
	UCHAR	bDdcCnt;	// количество DDC
	UCHAR	bRes;		// пока не используется (наличие фильтра ЦАП ?)
} ICR_Cfg0086, *PICR_Cfg0086, ICR_CfgAdm, *PICR_CfgAdm;

#pragma pack(pop)    

#endif // _ICR0086_H

//****************** End of file Icr0086.h **********************
