/*
 ****************** File CtrlFm214x1gtrf.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : FM214x1GTRF section
 *
 * (C) InSys by Ekkore Feb, 2016
 *
 *
 ************************************************************
*/

#ifndef _CTRL_FM214x1GTRF_H
 #define _CTRL_FM214x1GTRF_H

#include "ctrladc.h"

#pragma pack(push, 1)    

#define BRD_ADCCHANCNT			2	// Number of channels
#define BRD_DDCCHANCNT			4	// Number of DDC channels
//#define	BRD_GAINCNT			1	// Number of gains

//const int BRD_CLKDIVCNT	= 1; // Number of clock dividers = 1 (1)

// FM214x1GTRF Clock sources
enum {
	BRDclks_ADC_DISABLED	= 0,	// 
	BRDclks_ADC_SUBGEN		= 1,	// SubModule Internal Generator 
	BRDclks_ADC_SUBGEN_PLL	= 2,	// SubModule Internal Generator + PLL
	BRDclks_ADC_EXTCLK		= 0x81,	// External SubModule Clock
	BRDclks_ADC_EXTCLK_PLL	= 3,	// External SubModule Clock + PLL
	BRDclks_ADC_BASEGEN		= 0x85,	// Base Module Generator

	BRDclks_DAC_DISABLED	= 0,	// 
	BRDclks_DAC_SUBGEN		= 1,	// SubModule Generator
	BRDclks_DAC_SUBGEN_PLL	= 2,	// SubModule Internal Generator + PLL
	BRDclks_DAC_EXTCLK		= 0x81,	// External SubModule Clock
	BRDclks_DAC_EXTCLK_PLL	= 3,	// External SubModule Clock + PLL
	BRDclks_DAC_BASEGEN		= 0x85,	// Base Module Generator
};

// FM214x1GTRF start sources
enum {
	BRDsts_ADC_CHAN0	= 0,	// Start from channel 0
	BRDsts_ADC_EXT		= 2,	// Start from START IN (external)
	BRDsts_ADC_PRG		= 3,	// Program start
};



// Numbers of Specific Command
typedef enum _FM214X1GTRF_NUM_CMD
{
	FM214X1GTRFcmd_SETMU		= 64,
	FM214X1GTRFcmd_GETMU		= 65,
	FM214X1GTRFcmd_SETDDCPARAM		= 66,
	FM214X1GTRFcmd_DAC_SETLMFCCALIBRATION	= 80,
	FM214X1GTRFcmd_DAC_GETLMFCCALIBRATION	= 81,
} FM214X1GTRF_NUM_CMD;

typedef struct _FM214X1GTRF_DDCPARAM {
	U32		isDdcEnable;		// Разрешение DDC: 0-работает АЦП, 1-работает DDC
	U32		nDdcChannelMask;	// Маска каналов DDC: 0x1, 0x3, 0xF
	U32		nDdcDecimation;		// Коэффициент децимации DDC: 2, 4, 8, 16
	U32		nDdcMixerSelect;	// Тип сигнала от АЦП: 0-действительный, 1-комплексный

	U32		anDdcInpI[BRD_DDCCHANCNT];	// Номер АЦП, к которому подключены Input I DDC: 0, 1
	U32		anDdcInpQ[BRD_DDCCHANCNT];	// Номер АЦП, к которому подключены Input Q DDC: 0, 1
	U32		anDdcGain[BRD_DDCCHANCNT];	// Коэф. усиления для каналов DDC: 1, 2
	double	adDdcFc[BRD_DDCCHANCNT];	// Центральная частота для каналов DDC (Гц)
	double	adDdcPhase[BRD_DDCCHANCNT];	// Начальная фаза для каналов DDC (град): 0.0 - 360.0
} FM214X1GTRFSRV_DDCPARAM, *PF214X1GTRFSRV_DDCPARAM;

typedef struct _FM214X1GTRF_MU {
	U32		size;					// sizeof(FM214X1GTRFSRV_MU)
	U32		chanMask;				// маска выбранных каналов
	U32		syncMode;				// режим синхронизации (2 - Master, 1 - Single, 0 - Slave) всегда 1
	double	samplingRate;			// частота дискретизации (Гц): 10 МГц .. 125 МГц
	double	clockValue;				// значение частоты выбранного внешнего источника (Гц)
	U32		clockSrc;				// источник тактовой частоты (0 - выключен, 1 - генератор на субмодуле, 0x81 - внешний)
	U32		format;					// 0 - 16 бит, 1 - 8 бит

	U32		pretrigEnable;			// включение режима претриггера
	U32		pretrigExernal;			// включение режима внешнего претриггера
	U32		pretrigAssur;			// включение режима гарантированного претриггера
	U32		pretrigSamples;			// число отсчётов претриггера
	U32		startDelayCnt;			// счетчик начальной задержки
	U32		daqCnt;					// счетчик принимаемых данных
	U32		skipCnt;				// счетчик пропущенных данных
	U32		cnt0Enable;				// разрешение счетчика начальной задержки
	U32		cnt1Enable;				// разрешение счетчика принимаемых данных
	U32		cnt2Enable;				// разрешение счетчика пропущенных данных

	U32		baseStartSrc;			// базовый источник старта: 0 - программный, 7 - от субмодуля, 10 - от DDS базового модуля
	U32		baseStartInv;			// 1 - инверсия базового источника старта
	U32		baseStopSrc;			// базовый источник останова
	U32		baseStopInv;			// 1 - инверсия базового источника останова
	U32		startStopMode;			// 1 - режим триггерного старта-останова, 0 - режим потенциального старта-останова
	U32		reStart;				// 1 - режим сбора с автоматическим перезапуском
	U32		startSrc;				// субмодульный источник старта: 0 - канал 0, 1 - канал 1, 2 - внешний, 3 - программный
	U32		invStart;				// 1 - инверсия субмодульного источника старта
	double	startLevel;				// уровень старта в вольтах (от -2.5 В до +2.5 В – при внешнем старте)
	U32		startResist;			// вх.сопротивление для сигнала внешнего старта: 0 - 2.5 кОм, 1 - 50 Ом

// Специфические параметря для 1GTRF
	U32		nOffsetAdjust;		//  Цифровая коррекция пост. составляющей для обоих каналов АЦП: -128..+127
	U32		nInputSignalBand;	// Частота входного сигнала: 0-неизвестна, 1-10..150МГц, 2-10..500МГц, 3-500..1000МГц, 4- свыше 1ГГц

	U32		nDemodMode;		// Аналоговый демодулятор: 0-выкл, 1-вкл.
	double	dDemodFreqLo;	// Центральная частота аналогового демодулятора

	S32		nJesdSerAdjust;	// -1 или Для кристалла ADC Уровень сигнала на GT-линиях JESD'а: 0x0-0xF (см. табл.)
	S32		aJesdDeemph[4];	// -1 или Для кристалла ADC JESD De-emphasis select 

	FM214X1GTRFSRV_DDCPARAM		rDdcParam;
	U32		isPrintf;		// 1-печатать в консольное окно отладочную информацию

	//double	range[BRD_ADCCHANCNT];		// шкала преобразования для каждого канала (Вольт): 4.0, 2.0, 1.0, 0.5, 0.25
	//double	bias[BRD_ADCCHANCNT];		// смещение нуля для каждого канала (Вольт): -2*ШП .. +2*ШП
	//U32		inpResist[BRD_ADCCHANCNT];	// входное сопротивление для каждого канала (0 - 1 МOм, 1 - 50 Oм)
	//U32		dcCoupling[BRD_ADCCHANCNT];	// открытость (1) / закрытость (0) входа по постоянной составляющей для каждого канала
} FM214X1GTRFSRV_MU, *PF214X1GTRFSRV_MU;

typedef struct  _FM214X1GTRFSRV_DacLmfcCalibration {
	S32		idx;			// не используется, должно быть 0
	S32		aux;			// не используется, должно быть 0
	S32		nPClockFactor;	// (только GET)
	S32		nPClockPerMF;	// (только GET)
	S32		nK;				// (только GET)
	S32		nCount;			// (только GET) Количество заполненных ячеек в aRegVal[]
	S32		aRegVal[32];	// GET: значения регистров 0x302, 0x303 для всех м/с AD9144
							// SET: aRegVal[0] значения для 0x304/0x305, [1] - для 0x306/0x307
} FM214X1GTRFSRV_DacLmfcCalibration, *PFM214X1GTRFSRV_DacLmfcCalibration;

#pragma pack(pop)    

#endif // _CTRL_FM214x1GTRF_H

//
// End of file
//