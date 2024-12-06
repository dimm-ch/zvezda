/*
 ****************** File adcsubmodule.h ************************
 *
 * Definitions of structures and constants
 * of derived class CAdcSubModule for Data Acquisition Boards.
 *
 * (C) InSys by Dorokhin A. Jan. 2007
 *
 *
 *********************************************************
*/

#ifndef _ADCSUBMODULE_H_
 #define _ADCSUBMODULE_H_

#include	"submodule.h"
#include	"adcsrv.h"

#pragma pack(push,1)

//typedef struct _ADC_INI {
//	U32	Bits;		// разрядность
//	U32	Encoding;	// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
//	U32	MinRate;	// минимальная частота дискретизации
//	U32	MaxRate;	// максимальная частота дискретизации
//	U32	InpRange;	// входной диапазон
//	U32	FifoSize;	// размер FIFO (в байтах)
//} ADC_INI, *PADC_INI;

#pragma pack(pop)

// CAdcSubModule base class definition
class CAdcSubModule : public CSubModule
{

protected:

//	void GetAdcIni(PADC_INI pAdcIni, PINIT_Data pInitData, long size);
	void GetAdcIni(PADC_CFG pAdcIni, PINIT_Data pInitData, long size);
//	ULONG SetAdcConfig(PADC_CFG pAdcCfg, PICR_CfgAdc pAdc, PICR_CfgAdcFifo pAdcFifo, int isTag, PADC_INI pAdcIni);
	ULONG SetAdcConfig(PADC_CFG pAdcCfg, PICR_CfgAdc pAdc, PICR_CfgAdcFifo pAdcFifo, int isTag, PADC_CFG pAdcIni);
	void SetAdcCfg(PADCSRV_CFG pAdcDst, PADC_CFG pAdcSrc);

  	const BRDCHAR* getClassName(void) { return _BRDC("CAdcSubModule"); }

public:

	CAdcSubModule(); // constructor
	CAdcSubModule(PBRD_InitData pBrdInitData, long sizeInitData); // one more constructor
	//virtual ~CSubModule(); // destructor

};

#endif	// _SUBMODULE_H_

// ****************** End of file submodule.h **********************


 