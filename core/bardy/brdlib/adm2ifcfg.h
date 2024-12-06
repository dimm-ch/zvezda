/****************** File adm2ifcfg.h ************************
 *
 * Определения конфигурационных констант
 * и структур интерфейса ADM-PRO
 *
 * (C) InSys by Dorokhin A. Mar 2004
 *
 * History:
 *
 * 18.03.2004 - build
 * 27.06.2007 - добавлены структуры для ABMPEX, изменены структуры ADC и DAC (все поля 32-разрядные - отказ от структур ini)
 *
 **********************************************************/

#ifndef _ADM2IFCFG_H_
#define _ADM2IFCFG_H_

#pragma pack(push,1)

#define MAX_PLD		4
#define MAX_ADC		1
#define MAX_BASEDAC 2

#define SUB_MAXREFS 3
#define BASE_MAXREFS 2

typedef struct _BASEDAC_CFG {
	U08		Bits;				// разрядность
	U08		Encoding;			// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
	U32		DacFifoSize;		// размер FIFO ЦАП (в байтах)
	U08		IsCycle;			// возможность циклического режима
	U32		MinRate;			// minimal sampling rate (минимальная частота дискретизации (Гц))
	U32		MaxRate;			// maximal sampling rate (максимальная частота дискретизации (Гц))
	U08		FilterType;			// тип включенного фильтра
	U16		AFRange;			// DAC range for active filter (диапазон ЦАП при работе активного фильтра (мВольты))
	U16		PFRange;			// DAC range for passive filter (диапазон ЦАП при работе пассивного фильтра (мВольты))
	U16		AFCutoff;			// cufoff frequence of active filter (частота среза активного фильтра (сотни Гц))
	U16		PFCutoffLo;			// cutoff lowest frequence of passive filter (нижняя частота среза пассивного фильтра (кГц))
	U16		PFCutoffHi;			// cutoff highest frequence of passive filter (верхняя частота среза пассивного фильтра (кГц))
} BASEDAC_CFG, *PBASEDAC_CFG;

const BASEDAC_CFG BaseDacCfg_dflt = {
	14,			// bits
	1,			// encoding
	16384 * 2,	// FIFO size
	1,			// is cycling
	0,			// min rate
	125000000,	// max rate
	0, 			// passive filter
	1000,		// range active filter = 1000 мВ  
	500,		// range passive filter = 500 мВ 
	10000,		// cuttof active filter = 1 МГц  
	3,			// Low cuttof passive filter = 3 кГц  
	5000		// Hi cuttof passive filter = 5 МГц  
};

typedef struct _ADMIF_CFG {
	U32		RefGen[BASE_MAXREFS];	// frequency of generators (значения опорных генераторов (Гц))
	U32		SysRefGen;				// frequency of system generator (частота системного генератора (Гц))
	U32		ExtClk;					// external frequency of clock (внешняя частота тактирования (Гц))
	U08		DacCnt;					// DAC counter (количество ЦАП)
	U08		IsStartSync;			// Is start syncronization (наличие узла стартовой синхронизации: 1 - есть, 0 - нет)
	U08		AdmPldCnt;				// PLD counter (количество ПЛИС)
	U32		RefPVS;					// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	U08		PioType;				// type of PIO (0-non, 1-TTL, 2-LVDS)
	U08		AdmType;				// ADM2 interface type (0-TTL, 1-LVDS)
	BASEDAC_CFG DacCfg[MAX_BASEDAC];//
	U32		AdcFifoSize[MAX_ADC];	// размер FIFO АЦП (в байтах)
} ADMIF_CFG, *PADMIF_CFG;

const ADMIF_CFG AdmIfCfg_dflt = {
	{60000000,	// Gen0
	50000000},	// Gen1
	66666667,	// system clock
	30000000,	// external clock
	1,			// Dac counter
	1,			// is start
	1,			// PLD counter
	2500,		// опорное напряжение ИПН
	1,			// PIO type
	0,			// ADM interface type
	{{	14,			// bits
		1,			// encoding
		16384 * 2,	// DAC FIFO size
		1,			// is cycling
		0,			// min rate
		30000000,	// max rate
		0, 			// passive filter
		1000,		// range active filter = 1000 мВ  
		500,		// range passive filter = 500 мВ 
		10000,		// cuttof active filter = 1 МГц  
		3,			// Low cuttof passive filter = 3 kHz  
		5000		// Hi cuttof passive filter = 5 MHz  
	},
	{	14,			// bits
		1,			// encoding
		16384 * 2,	// DAC FIFO size
		1,			// is cycling
		0,			// min rate
		30000000,	// max rate
		0, 			// passive filter
		1000,		// range active filter = 1000 мВ  
		500,		// range passive filter = 500 мВ 
		10000,		// cuttof active filter = 1 МГц  
		3,			// Low cuttof passive filter = 3 kHz  
		5000		// Hi cuttof passive filter = 5 MHz  
	}},
	{ 16384 * 4 }		// ADC FIFO size
};

typedef struct _ADMIF2_CFG {
	U32		RefGen;			// frequency of generator (частота генератора (Гц))
	U32		SysRefGen;		// frequency of system generator (частота системного генератора (Гц))
	U32		ExtClk;			// external frequency of clock (внешняя частота тактирования (Гц))
	U08		DdsType;		// type of DDS (0-non, 1-PLL, 2-DDS)
	U08		IsStartSync;	// Is start syncronization (наличие узла стартовой синхронизации: 1 - есть, 0 - нет)
	U08		AdmPldCnt;		// PLD counter (количество ПЛИС)
	U32		RefPVS;			// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	U08		PioType;		// type of PIO (0-non, 1-TTL, 2-LVDS)
	U32		AdcFifoSize;	// размер FIFO АЦП (в байтах)
} ADMIF2_CFG, *PADMIF2_CFG;

const ADMIF2_CFG AdmIf2Cfg_dflt = {
	60000000,	// RefGen
	66666667,	// system clock
	30000000,	// external clock
	1,			// DDS type
	1,			// is start
	1,			// PLD counter
	2500,		// опорное напряжение ИПН
	1,			// PIO type
	16384 * 4		// ADC FIFO size
};

typedef struct _ADC_CFG {
	U32		Bits;		// разрядность
	U32		Encoding;	// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
	U32		MinRate;	// минимальная частота дискретизации
	U32		MaxRate;	// максимальная частота дискретизации
	U32		InpRange;	// входной диапазон
	U32		FifoSize;	// размер FIFO (в байтах)
	U32		TetrNum;	// номер тетрады (перебивает ID тетрады)
} ADC_CFG, *PADC_CFG;

const ADC_CFG AdcCfg_dflt = {
	14,			// bits
	1,			// encoding
	100000,
	100000000,
	500,
	16384 * 4,		// ADC FIFO size
	(U32)-1
};

typedef struct _DAC_CFG {
	U32		Bits;		// разрядность
	U32		Encoding;	// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
	U32		MinRate;	// минимальная частота дискретизации
	U32		MaxRate;	// максимальная частота дискретизации
	U32		OutRange;	// выходной диапазон
	U32		FifoSize;	// размер FIFO (в байтах)
	U32		TetrNum;	// номер тетрады (перебивает ID тетрады)
} DAC_CFG, *PDAC_CFG;

const DAC_CFG DacCfg_dflt = {
	14,			// bits
	1,			// encoding
	0,
	100000000,
	500,
	16384 * 4,		// DAC FIFO size
	(U32)-1
};

#pragma pack(pop)

#endif //_ADM2IFCFG_H_
