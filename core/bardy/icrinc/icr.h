//****************** File icr.h ***************************
//
//  Определения констант и структур 
//	для работы с конфигурационным ППЗУ
//
//  Constants & structures definitions
//	for identification & configuration EPROM
//
//	Copyright (c) 2002-2004, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  15-01-03 - builded
//  31-07-03 - add ICR_Cfg0310 Submodule DAC configuration structure
//  09-09-03 - add ICR_Cfg0510 Host PLD configuration structure
//  09-09-03 - add ICR_Cfg0610 ADM2 interface configuration structure
//  14-10-04 - add ICR_Cfg0710 DDR SDRAM, ICR_Cfg0720 SRAM and ICR_Cfg0800 DSP node configuration structures
//
//*********************************************************

#ifndef __ICR_H_
 #define __ICR_H_

//#include "utypes.h"

#ifndef _UTYPES_H_

typedef	unsigned char	U08, *PU08;
typedef	signed   char	S08, *PS08;
typedef	unsigned short	U16, *PU16;
typedef	signed   short	S16, *PS16;
typedef	unsigned int	U32, *PU32;
typedef	signed   int	S32, *PS32;

typedef	unsigned long long int	U64, *PU64;
typedef	signed   long long int	S64, *PS64;

typedef float			REAL32, *PREAL32;
typedef double			REAL64, *PREAL64;

#endif /* _UTYPES_H_ */

#pragma pack(push, 1)    

const U16 END_TAG			= 0x0000; // tag of end data (тэг, информирующий об окончании данных)
const U16 ALT_END_TAG		= 0xffff; // alt tag of end data (альтернативный тэг, информирующий об окончании данных)

const U16 BASE_ID_TAG		= 0x4953; // tag of base module identification structure
const U16 AMBPCI_CFG_TAG	= 0x4D50; // tag of AMBPCI configuration structure 
const U16 COMMENT_ID_TAG	= 0x0620; // tag of comment

const U16 ADM_ID_TAG		= 0x0080; // tag of ADM submodule identification structure

const U16 ADC_CFG_TAG		= 0x0100; // tag of ADC configuration structure (freq in Hz)
const U16 ADCKHZ_CFG_TAG	= 0x0101; // tag of ADC configuration structure (freq in kHz)
const U16 ADC_FIFO_TAG		= 0x0200; // tag of ADC FIFO configuration structure
const U16 FIFO_HOST_TAG		= 0x0210; // tag of FIFO HOST configuration structure
const U16 DAC_CFG_TAG		= 0x0300; // tag of DAC configuration structure
const U16 ADMDAC_CFG_TAG	= 0x0310; // tag of Submodule DAC configuration structure (freq in Hz)
const U16 DACKHZ_CFG_TAG	= 0x0311; // tag of Submodule DAC configuration structure (freq in kHz)
const U16 DAC_FIFO_TAG		= 0x0400; // tag of DAC FIFO configuration structure
const U16 PLD_CFG_TAG		= 0x0500; // tag of PLD configuration structure
const U16 HOSTPLD_CFG_TAG	= 0x0510; // tag of HOST PLD configuration structure
const U16 ADMIF_CFG_TAG		= 0x0600; // tag of ADM-interface configuration structure
const U16 ADM2IF_CFG_TAG	= 0x0610; // tag of ADM2-interface configuration structure

const U16 SDRAM_CFG_TAG		= 0x0700; // тэг для структуры конфигурационных параметров динамической памяти
const U16 DDRSDRAM_CFG_TAG	= 0x0710; // тэг для структуры конфигурационных параметров динамической памяти DDR SDRAM
const U16 SRAM_CFG_TAG		= 0x0720; // тэг для структуры конфигурационных параметров статической памяти SRAM
const U16 DDR3_CFG_TAG		= 0x0730; // тэг для структуры конфигурационных параметров динамической памяти DDR3
const U16 DDR4_CFG_TAG		= 0x0740; // тэг для структуры конфигурационных параметров динамической памяти DDR4

const U16 DSPNODE_CFG_TAG	= 0x0800; // tag of DSP node configuration structure (for AMBPCD)

const U16 BASEMOD_CFGMEM_SIZE = 256/*512*/; // размер ППЗУ, устанавливаемом на базовый модуль
const U16 SUBMOD_CFGMEM_SIZE = 512;  // размер ППЗУ, устанавливаемом на субмодуль

enum
{
	READ_WRITE_BASEMODULE,
	READ_WRITE_SUBMODULE,
	READ_WRITE_ALL,
};

// Base module identification
// Идентификационная структура базового модуля
typedef struct _ICR_Id4953 {
	U16	wTag;		// tag of structure (BASE_ID_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U16	wSizeAll;	// size of all data, writing into base module EPROM
	U32	dSerialNum;	// serial number (серийный (физический) номер)
	U16	wDeviceId;	// base module type (тип базового модуля)
	U08	bVersion;	// base module version (версия базового модуля)
	U08	bDay;		// day of Data (день записи данных в ППЗУ базового модуля)
	U08	bMon;		// montag of Data (месяц записи данных в ППЗУ базового модуля)
	U16	wYear;		// year of Data (год записи данных в ППЗУ базового модуля)
} ICR_Id4953, *PICR_Id4953, ICR_IdBase, *PICR_IdBase;

// Comment identification
// Идентификационная структура комментария
typedef struct _ICR_Id0620 {
	U16	wTag;		// tag of structure (COMMENT_ID_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdComment) - 4
	U08 bInterfaceNum;	// ADM-Interface number (номер ADM-интерфейса), 0xFF - comment for base module
	U08	abComment[16];	// comment
} ICR_Id0620, *PICR_Id0620, ICR_IdComment, *PICR_IdComment;

// ADM identification
// Идентификационная структура субмодуля
typedef struct _ICR_Id0080 {
	U16	wTag;		// tag of structure (ADM_ID_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U16	wSizeAll;	// размер всех данных, записанных в ППЗУ субмодуля
	U08	bAdmNum;	// ADM-Interface number (номер ADM-интерфейса)
	U32	dSerialNum;	// serial number (серийный (физический) номер)
	U16	wType;		// ADM-submodule type (тип ADM-субмодуля)
	U08	bVersion;	// ADM-submodule version (версия ADM-субмодуля)
	U08	bDay;		// day of Data (день записи данных в ППЗУ субмодуля)
	U08	bMon;		// montag of Data (месяц записи данных в ППЗУ субмодуля)
	U16	wYear;		// year of Data (год записи данных в ППЗУ субмодуля)
} ICR_Id0080, *PICR_Id0080, ICR_IdAdm, *PICR_IdAdm;

// AMBPCI module configuration structure
// Конфигурационная структура базового модуля AMBPCI
typedef struct _ICR_Cfg4D50 {
	U16	wTag;		// tag of structure (AMBPCI_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bAdmIfCnt;	// ADM-Interface counter (количество ADM-интерфейсов)
} ICR_Cfg4D50, *PICR_Cfg4D50, ICR_CfgAmbpci, *PICR_CfgAmbpci;

// ADC configuration structure
// Конфигурационная структура АЦП
typedef struct _ICR_Cfg0100 {
	U16	wTag;		// tag of structure (ADC_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// number of ADC (номер АЦП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bBits;		// Word Length (разрядность АЦП (число бит))
	U08	bEncoding;	// type of code (тип кодировки)
	U32	dMinRate;	// minimal sampling rate (минимальная частота дискретизации (Гц))
	U32	dMaxRate;	// maximal sampling rate (максимальная частота дискретизации (Гц))
	U16	wRange;		// input range (входной диапазон АЦП (мВольт))
} ICR_Cfg0100, *PICR_Cfg0100, ICR_CfgAdc, *PICR_CfgAdc;

//
// ADC configuration structure (once more)
// Конфигурационная структура АЦП (еще одна)
// Частоты выражены в килогерцах
//
typedef struct _ICR_Cfg0101 {
	U16	wTag;		// tag of structure (ADC_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// number of ADC (номер АЦП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bBits;		// Word Length (разрядность АЦП (число бит))
	U08	bEncoding;	// type of code (тип кодировки)
	U32	dMinRate;	// minimal sampling rate (минимальная частота дискретизации (кГц))
	U32	dMaxRate;	// maximal sampling rate (максимальная частота дискретизации (кГц))
	U16	wRange;		// input range (входной диапазон АЦП (мВольт))
} ICR_Cfg0101, *PICR_Cfg0101;

// ADC FIFO configuration structure
// Конфигурационная структура FIFO АЦП
typedef struct _ICR_Cfg0200 {
	U16	wTag;		// tag of structure (ADC_FIFO_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// id number of the ADC FIFO (номер FIFO АЦП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bDepth;		// Depth of FIFO (глубина (размер) FIFO (слова))
	U08	bBitsWidth;	// Width of FIFO (ширина FIFO (число бит в слове))
} ICR_Cfg0200, *PICR_Cfg0200, ICR_CfgAdcFifo, *PICR_CfgAdcFifo;

// FIFO HOST configuration structure
// Конфигурационная структура FIFO HOST
typedef struct _ICR_Cfg0210 {
	U16	wTag;		// tag of structure (FIFO_HOST_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// id number of the FIFO HOST (номер FIFO HOST)
	U08	bDepth;		// Depth of FIFO (глубина (размер) FIFO (слова))
	U08	bBitsWidth;	// Width of FIFO (ширина FIFO (число бит в слове))
} ICR_Cfg0210, *PICR_Cfg0210, ICR_CfgFifoHost, *PICR_CfgFifoHost;

// DAC configuration structure
// Конфигурационная структура ЦАП
typedef struct _ICR_Cfg0300 {
	U16	wTag;		// tag of structure (DAC_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// id number of the DAC (номер ЦАП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bBits;		// Word Length (разрядность ЦАП (число бит))
	U08	bEncoding;	// type of code (тип кодировки)
	U32	dMinRate;	// minimal sampling rate (минимальная частота дискретизации (Гц))
	U32	dMaxRate;	// maximal sampling rate (максимальная частота дискретизации (Гц))
	U16	wAFRange;	// DAC range for active filter (диапазон ЦАП при работе активного фильтра (мВольты))
	U16	wPFRange;	// DAC range for passive filter (диапазон ЦАП при работе пассивного фильтра (мВольты))
	U16	wAFCutoff;	// cufoff frequence of active filter (частота среза активного фильтра (сотни Гц))
	U16	wPFCutoffLo;// cutoff lowest frequence of passive filter (нижняя частота среза пассивного фильтра (кГц))
	U16	wPFCutoffHi;// cutoff highest frequence of passive filter (верхняя частота среза пассивного фильтра (кГц))
} ICR_Cfg0300, *PICR_Cfg0300, ICR_CfgDac, *PICR_CfgDac;

// Submodule DAC configuration structure
// Конфигурационная структура ЦАП субмодуля
typedef struct _ICR_Cfg0310 {
	U16	wTag;		// tag of structure (DAC_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// id number of the DAC (номер ЦАП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bBits;		// Word Length (разрядность ЦАП (число бит))
	U08	bEncoding;	// type of code (тип кодировки)
	U32	dMinRate;	// minimal sampling rate (минимальная частота дискретизации (Гц))
	U32	dMaxRate;	// maximal sampling rate (максимальная частота дискретизации (Гц))
	U16	wRange;		// DAC range for active filter (диапазон ЦАП (мВольты))
} ICR_Cfg0310, *PICR_Cfg0310, ICR_CfgAdmDac, *PICR_CfgAdmDac;

//
// Submodule DAC configuration structure (once more)
// Конфигурационная структура ЦАП субмодуля (еще одна)
// Частоты выражены в килогерцах
//
typedef struct _ICR_Cfg0311 {
	U16	wTag;		// tag of structure (DAC_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// id number of the DAC (номер ЦАП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bBits;		// Word Length (разрядность ЦАП (число бит))
	U08	bEncoding;	// type of code (тип кодировки)
	U32	dMinRate;	// minimal sampling rate (минимальная тактовая частота (кГц))
	U32	dMaxRate;	// maximal sampling rate (максимальная тактовая частота (кГц))
	U32	dMaxInterpRate;	// maximal interpolation rate (максимальная тактовая частота с интерполяцией (кГц))
	U16	wRange;		// DAC range for active filter (диапазон ЦАП (мВольты))
} ICR_Cfg0311, *PICR_Cfg0311;

// DAC FIFO configuration structure
// Конфигурационная структура FIFO ЦАП
typedef struct _ICR_Cfg0400 {
	U16	wTag;		// tag of structure (DAC_FIFO_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// id number of the DAC FIFO (номер FIFO ЦАП)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bDepth;		// Depth of FIFO (глубина (размер) FIFO (слова))
	U08	bBitsWidth;	// Width of FIFO (ширина FIFO (число бит в слове))
	U08	bIsCycle;	// Is cycle mode (возможность циклического режима работы FIFO)
} ICR_Cfg0400, *PICR_Cfg0400, ICR_CfgDacFifo, *PICR_CfgDacFifo;

// ADM PLD configuration structure
// Конфигурационная структура ПЛИС интерфейса ADM
typedef struct _ICR_Cfg0500 {
	U16	wTag;		// tag of structure (PLD_CFG_TAG)
	U16	wSize;		// size of bytes (размер всех следующих полей структуры)
	U08	bNumber;	// id number of the PLD (номер ПЛИС)
	U08	bAdmNum;	// ADM-Interface number (номер интерфейса ADM)
	U08	bType;		// type of PLD (0-EP1K,1-EP1KA,2-EP10KE,...) (серия(тип) ПЛИС)
	U16	wVolume;	// volume of PLD (объем ПЛИС)
	U16	wPins;		// pins counter of PLD (число выводов)
	U08	bSpeedGrade;// быстродействие 1,2,3,...
} ICR_Cfg0500, *PICR_Cfg0500, ICR_CfgAdmPld, *PICR_CfgAdmPld;

// Host PLD configuration structure
// Конфигурационная структура ПЛИС интерфейса хоста
typedef struct _ICR_Cfg0510 {
	U16	wTag;		// tag of structure (PLD_CFG_TAG)
	U16	wSize;		// size of bytes (размер всех следующих полей структуры)
	U08	bNumber;	// id number of the PLD (номер ПЛИС)
	U08	bType;		// type of Host PLD (TYPE_HostPLD)
	U16	wVolume;	// volume of Host PLD 
	U16	wPins;		// pins counter of Host PLD 
	U08	bSpeedGrade;// быстродействие (6 or 7)
} ICR_Cfg0510, *PICR_Cfg0510, ICR_CfgHostPld, *PICR_CfgHostPld;

// ADM interface configuration structure
// Конфигурационная структура интерфейса ADM, расположенного на базовом модуле
typedef struct _ICR_Cfg0600 {
	U16	wTag;		// tag of structure (ADMIF_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// ADM-Interface number (номер интерфейса ADM)
	U08	bAdcFifoCnt;// ADC FIFO counter (количество FIFO АЦП)
	U08	bDacCnt;	// DAC counter (количество ЦАП)
	U08	bDacFifoCnt;// DAC FIFO counter (количество FIFO ЦАП)
	U32	dRefGen[2];	// frequency of generators (значения опорных генераторов (Гц))
	U08	bIsStartSync;// Is start syncronization (наличие узла стартовой синхронизации: 1 - есть, 0 - нет)
	U08	bPldCnt;	// PLD counter (количество ПЛИС)
	U16	wRefPVS;	// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	U08	bIsPio;		// Is digital port (PIO) (наличие порта ввода/вывода: 1 - есть, 0 - нет)
} ICR_Cfg0600, *PICR_Cfg0600, ICR_CfgAdmIf, *PICR_CfgAdmIf;

// ADM2 interface configuration structure
// Конфигурационная структура интерфейса ADM2, расположенного на базовом модуле
typedef struct _ICR_Cfg0610 {
	U16	wTag;		// tag of structure (ADMIF_CFG_TAG)
	U16	wSize;		// size of all following fields of structure (without wTag + wSize) = sizeof(ICR_IdBase) - 4
	U08	bNumber;	// ADM-Interface number (номер интерфейса ADM)
	U08	bAdcFifoCnt;// ADC FIFO counter (количество FIFO АЦП)
	U08	bDacCnt;	// DAC counter (количество ЦАП)
	U08	bDacFifoCnt;// DAC FIFO counter (количество FIFO ЦАП)
	U32	dRefGen[2];	// frequency of generators (значения опорных генераторов (Гц))
	U08	bIsStartSync;// Is start syncronization (наличие узла стартовой синхронизации: 1 - есть, 0 - нет)
	U08	bPldCnt;	// PLD counter (количество ПЛИС)
	U16	wRefPVS;	// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	U08	bPioType;	// type of PIO (0-non, 1-TTL, 2-LVDS)
	U08	bType;		// ADM2 interface type (0-TTL, 1-LVDS)
} ICR_Cfg0610, *PICR_Cfg0610, ICR_CfgAdm2If, *PICR_CfgAdm2If;

// SDRAM configuration
// Конфигурационные параметры динамической памяти
typedef struct _ICR_Cfg0700 {
	U16	wTag;			// тэг структуры (SDRAM_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08 bNumber;		// номер структуры
	U08 bSlotCnt;		// количество установленных слотов
	U08 bModuleCnt;		// количество установленных DIMM-модулей
	U08 bRowAddrBits;	// количество разрядов адреса строк
	U08 bColAddrBits;	// количество разрядов адреса столбцов
	U08 bModuleBanks;	// количество банков в DIMM-модуле
	U08 bChipBanks;		// количество банков в микросхемах DIMM-модуля
} ICR_Cfg0700, *PICR_Cfg0700, ICR_CfgSdram, *PICR_CfgSdram;

// DDR SDRAM configuration
// Конфигурационные параметры динамической памяти DDR
typedef struct _ICR_Cfg0710 {
	U16	wTag;			// тэг структуры (DDRSDRAM_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08 bNumber;		// номер структуры
	U08 bModuleCnt;		// количество установленных DIMM-модулей (занятых слотов)
	U08 bRowAddrBits;	// количество разрядов адреса строк
	U08 bColAddrBits;	// количество разрядов адреса столбцов
	U08 bModuleBanks;	// количество банков в DIMM-модуле
	U08 bChipBanks;		// количество банков в микросхемах DIMM-модуля
	U08 bPrimaryWidth;	// количество разрядов данных в микросхемах DIMM-модуля
	U08 bCasLatency;	// задержка выборки адреса
} ICR_Cfg0710, *PICR_Cfg0710, ICR_CfgDdrSdram, *PICR_CfgDdrSdram;

// SRAM configuration
// Конфигурационные параметры статической памяти SRAM
typedef struct _ICR_Cfg0720 {
	U16	wTag;			// тэг структуры (SRAM_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08	bNumber;		// номер структуры
	U08	bChipCnt;		// количество установленных микросхем
	U08	bChipDepth;		// Depth of Chip (глубина (размер) микросхемы (в словах))
	U08	bChipBitsWidth;	// Width of Chip (ширина микросхемы (число бит в слове))
} ICR_Cfg0720, *PICR_Cfg0720, ICR_CfgSram, *PICR_CfgSram;

// DDR3 SDRAM configuration
// Конфигурационные параметры динамической памяти DDR3
typedef struct _ICR_Cfg0730 {
	U16	wTag;			// тэг структуры (DDR3_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08 bNumber;		// номер структуры
	U08 bModuleCnt;		// количество установленных SODIMM-модулей (занятых слотов)
	U16	wCapacityMbits;	// объём микросхемы в мегабитах (256,512,1024,2048,4096,8192,16384)
	U08 bChipBanks;		// количество банков (8,16,32,64)
	U08 bRowAddrBits;	// количество разрядов адреса строк (12-16)
	U08 bColAddrBits;	// количество разрядов адреса столбцов (9-12)
	U08 bModuleBanks;	// количество рядов (Ranks)на SODIMM-модуле (1-4)
	U08 bChipWidth;		// ширина микросхемы (4,8,16,32)
	U08 bPrimaryWidth;	// ширина шины памяти (8,16,32,64)
} ICR_Cfg0730, *PICR_Cfg0730, ICR_CfgDdr3, *PICR_CfgDdr3;

// DDR4 SDRAM configuration
// Конфигурационные параметры динамической памяти DDR4
typedef struct _ICR_Cfg0740 {
	U16	wTag;			// тэг структуры (DDR4_CFG_TAG)
	U16	wSize;			// размер всех следующих полей структуры
	U08 bNumber;		// номер структуры
	U08 bModuleCnt;		// количество установленных SODIMM-модулей (занятых слотов)
	U16	wCapacityMbits;	// объём микросхемы в мегабитах (256,512,1024,2048,4096,8192,16384,32768)
	U08 bChipBanks;		// количество банков (4,8)
	U08 bRowAddrBits;	// количество разрядов адреса строк (12-18)
	U08 bColAddrBits;	// количество разрядов адреса столбцов (9-12)
	U08 bModuleBanks;	// количество рядов (Ranks)на SODIMM-модуле (1-8)
	U08 bChipWidth;		// ширина микросхемы (4,8,16,32)
	U08 bPrimaryWidth;	// ширина шины памяти (8,16,32,64)
} ICR_Cfg0740, *PICR_Cfg0740, ICR_CfgDdr4, *PICR_CfgDdr4;

// DSP node configuration structure
// Конфигурационная структура узла цифровой обработки сигналов
typedef struct _ICR_Cfg0800 {
	U16	wTag;			// tag of structure (DSPNODE_CFG_TAG)
	U16	wSize;			// size of bytes (размер всех следующих полей структуры)
	U08	bNumber;		// id number of the PLD (номер ПЛИС)
	U08	bPldType;		// type of PLD (0-EP1K,1-EP1KA,2-EP10KE,...) (серия(тип) ПЛИС)
	U16	wPldVolume;		// volume of PLD (объем ПЛИС)
	U16	wPldPins;		// pins counter of PLD (число выводов)
	U08	bPldSpeedGrade; // быстродействие 1,2,3,...
	U08	bSramCfgCnt;	// количество конфигурационных структур статической памяти SRAM
	U08	bPioType;		// type of PIO (0-non, 1-TTL, 2-LVDS)
	U08	bLoadRom;		// Is loading ROM (наличие загрузочного ПЗУ: 0 - нет, 1 - есть, 0xFF - снимаемое)
} ICR_Cfg0800, *PICR_Cfg0800, ICR_CfgDspNode, *PICR_CfgDspNode;

#pragma pack(pop)    

#endif // __ICR_H_

//***************** End of file icr.h *********************
