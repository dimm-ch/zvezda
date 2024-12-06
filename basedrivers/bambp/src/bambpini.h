//****************** File bambpini.h *******************************
// Definitions of structures and constants 
// for initialization from ini-file into Data Acquisition Carrier Boards.
//
//	Copyright (c) 2004, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  05-10-04 - builded
//
//*******************************************************************

#ifndef _BAMBPINI_H
 #define _BAMBPINI_H

#include	"utypes.h"

#define MAX_MEM_INI 2

#define DLGDLLNAME_SIZE 64

#pragma pack(push, 1)    

typedef struct _DSP_INI {
	U32		PldType;		// type of PLD (0-EP1K,1-EP1KA,2-EP10KE,...) (серия(тип) ПЛИС)
	U32		PldVolume;		// volume of PLD (объем ПЛИС)
	U32		PldPins;		// pins counter of PLD (число выводов)
	U32		PldSpeedGrade; // быстродействие 1,2,3,...
	U32		LoadRom;		// Is loading ROM (наличие загрузочного ПЗУ: 0 - нет, 1 - есть, 0xFF - снимаемое)
	U32		PioType;		// type of PIO (0-non, 1-TTL, 2-LVDS)
	U32		SramChipCnt;		// количество установленных микросхем
	U32		SramChipDepth;		// Depth of Chip (глубина (размер) микросхемы (в словах))
	U32		SramChipBitsWidth;	// Width of Chip (ширина микросхемы (число бит в слове))
} DSP_INI, *PDSP_INI;

typedef struct _MEM_INI {
	TCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	U32		SlotCnt;		// количество установленных слотов
	U32		ModuleCnt;		// количество установленных DIMM-модулей
	U32		ModuleBanks;	// количество банков в DIMM-модуле
	U32		RowAddrBits;	// количество разрядов адреса строк
	U32		ColAddrBits;	// количество разрядов адреса столбцов
	U32		ChipBanks;		// количество банков в микросхемах
	U32		PrimWidth;		// Primary DDR SDRAM Width
	U32		CasLatency;		// задержка сигнала выборки по столбцам
	U32		Attributes;		// атрибуты DIMM-модуля
} MEM_INI, *PMEM_INI;

typedef struct _BASE_INI {
	U32		PID;			// серийный (физический) номер
	U32		BusNumber;		// номер шины (Bus Number)
	U32		DevNumber;		// номер устройства (Device Number)
	U32		SlotNumber;		// номер слота (Slot Number)
	U32		SysGen;			// значение частоты системного генератора в Гц
	MEM_INI MemIni[MAX_MEM_INI];// параметры инициализации памяти
	DSP_INI DspIni[MAX_MEM_INI];// параметры инициализации узла ЦОС
} BASE_INI, *PBASE_INI;

#pragma pack(pop)    

#endif // _BAMBPINI_H

// ****************** End of file bambpini.h **********************
