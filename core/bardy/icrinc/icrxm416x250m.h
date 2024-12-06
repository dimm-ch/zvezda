//****************** File icrxm416x250m.h *******************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//  базовых модулей XM416x250M
//
//	Copyright (c) May 2014, Instrumental Systems,Corp.
//	Written by Ekkore
//
//*******************************************************************

#ifndef __XM416x250M_H_
 #define __XM416x250M_H_

#pragma pack(push, 1)    

const USHORT XM416x250M_CFG_TAG = 0x5516; // тэг для структуры конфигурационных параметров базовой части комбинированоого модуля XM416x250M

// XM416x250M module configuration
// Конфигурационные параметры базового модуля XM416x250M
typedef struct _ICR_CfgXm416x250m {
	U16	wTag;			// тэг структуры (XM416x250M_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08	bAdmIfCnt;		// количество интерфейсов ADM
	U32	dSysGen;			// Системный генератор (Гц) (по умолчанию 250 МГц)
	U08	isDDR3;			// Наличие памяти DDR3
	U08	isVoltageMonitor;	// Наличие монитора напряжений
	U08	isCurrentMonitor;	// Наличие монитора тока
} ICR_CfgXm416x250m, *PICR_CfgXm416x250m, ICR_Cfg5516, *PICR_Cfg5516;

#pragma pack(pop)    

#endif // __XM416x250M_H_

// ****************** End of file icrxm416x250m.h **********************
