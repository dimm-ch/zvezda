//****************** File Icr0088.h ***********************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  для субмодулей 
//
//	Copyright (c) 2002-2003, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  09-10-03 - builded
//
//*********************************************************

#ifndef _ICR0088_H
 #define _ICR0088_H

#pragma pack(push, 1)    

#define ADM_CFG_TAG 0x0088

// Конфигурационные параметры субмодуля
typedef struct _ICR_Cfg0088 {
	USHORT	wTag;		// тэг структуры (ADM_CFG_TAG)
	USHORT	wSize;		// размер всех следующих полей структуры
	UCHAR	bAdmIfNum;	// номер интерфейса ADM
	UCHAR	bBufType;	// тип буфера
} ICR_Cfg0088, *PICR_Cfg0088, ICR_CfgAdm, *PICR_CfgAdm;

#pragma pack(pop)    

#endif // _ICR0087_H

//****************** End of file Icr0087.h **********************
