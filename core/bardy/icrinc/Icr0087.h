//****************** File Icr0087.h ***********************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  для субмодулей 
//
//	Copyright (c) 2002-2003, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  31-07-03 - builded
//
//*********************************************************

#ifndef _ICR0087_H
 #define _ICR0087_H

#pragma pack(push, 1)    

#define ADM_CFG_TAG 0x0087

// Конфигурационные параметры субмодуля
typedef struct _ICR_Cfg0087 {
	USHORT	wTag;		// тэг структуры (ADM_CFG_TAG)
	USHORT	wSize;		// размер всех следующих полей структуры
	UCHAR	bAdmIfNum;	// номер интерфейса ADM
	ULONG	dGen;		// значение опорного генератора
	UCHAR	bDacCnt;	// количество ЦАП
} ICR_Cfg0087, *PICR_Cfg0087, ICR_CfgAdm, *PICR_CfgAdm;

#pragma pack(pop)    

#endif // _ICR0087_H

//****************** End of file Icr0087.h **********************
