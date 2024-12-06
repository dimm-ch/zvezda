/*
 ***************** File adcsubmodule.cpp ***********************
 *
 * BRD Driver for ...
 *
 * (C) InSys by Dorokhin A. Jan 2007
 *
 ******************************************************
*/

#include "adcsubmodule.h"

#define	CURRFILE "ADCSUBMODULE"

//******************************************
CAdcSubModule::CAdcSubModule() :
	CSubModule()
{
}

//******************************************
CAdcSubModule::CAdcSubModule(PBRD_InitData pBrdInitData, long sizeInitData) :
	CSubModule(pBrdInitData, sizeInitData)
{
}

//******************************************
//void CAdcSubModule::GetAdcIni(PADC_INI pAdcIni, PINIT_Data pInitData,long size)
void CAdcSubModule::GetAdcIni(PADC_CFG pAdcIni, PINIT_Data pInitData,long size)
{
	FindKeyWord(_BRDC("bits"), pAdcIni->Bits, pInitData, size);
	FindKeyWord(_BRDC("encoding"), pAdcIni->Encoding, pInitData, size);
	FindKeyWord(_BRDC("minrate"), pAdcIni->MinRate, pInitData, size);
	FindKeyWord(_BRDC("maxrate"), pAdcIni->MaxRate, pInitData, size);
	FindKeyWord(_BRDC("inprange"), pAdcIni->InpRange, pInitData, size);
	FindKeyWord(_BRDC("adcfifosize"), pAdcIni->FifoSize, pInitData, size);
	FindKeyWord(_BRDC("adctetrnum"), pAdcIni->TetrNum, pInitData, size);
}

//***************************************************************************************
//ULONG CAdcSubModule::SetAdcConfig(PADC_CFG pAdcCfg, PICR_CfgAdc pAdc, PICR_CfgAdcFifo pAdcFifo, int isTag, PADC_INI pAdcIni)
ULONG CAdcSubModule::SetAdcConfig(PADC_CFG pAdcCfg, PICR_CfgAdc pAdc, PICR_CfgAdcFifo pAdcFifo, int isTag, PADC_CFG pAdcIni)
{
	pAdcCfg->FifoSize = 0;
	if(pAdcFifo->wTag == ADC_FIFO_TAG)
		pAdcCfg->FifoSize = (1 << pAdcFifo->bDepth) *
								   ((1 << (pAdcFifo->bBitsWidth + 2)) / 8);
	if(isTag)
	{	
		//pAdcCfg->FifoSize = 0;
		if(pAdc->wTag == ADC_CFG_TAG)
		{	// получаем конфигурацию АЦП из ППЗУ
			pAdcCfg->Bits = pAdc->bBits;			// разрядность АЦП
			pAdcCfg->Encoding = pAdc->bEncoding; // тип кодировки в ICR (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
			pAdcCfg->MinRate = pAdc->dMinRate;	// минимальная частота дискретизации
			pAdcCfg->MaxRate = pAdc->dMaxRate;	// максимальная частота дискретизации
			pAdcCfg->InpRange = pAdc->wRange;	// входной диапазон
		}
	}
	if(pAdcIni)
	{	// уточняем конфигурацию из источника инициализации
		if(pAdcIni->Bits != 0xffffffff)
			pAdcCfg->Bits = pAdcIni->Bits;
		if(pAdcIni->Encoding != 0xffffffff)
			pAdcCfg->Encoding = pAdcIni->Encoding;
		if(pAdcIni->MinRate != 0xffffffff)
			pAdcCfg->MinRate = pAdcIni->MinRate;
		if(pAdcIni->MaxRate != 0xffffffff)
			pAdcCfg->MaxRate = pAdcIni->MaxRate;
		if(pAdcIni->InpRange != 0xffffffff)
			pAdcCfg->InpRange = pAdcIni->InpRange;
		if(pAdcIni->FifoSize != 0xffffffff)
			pAdcCfg->FifoSize = pAdcIni->FifoSize;
		//if(pAdcIni->TetrNum != 0xffffffff)
			pAdcCfg->TetrNum = pAdcIni->TetrNum;
	}

	return BRDerr_OK;
}

//******************************************
void CAdcSubModule::SetAdcCfg(PADCSRV_CFG pAdcDst, PADC_CFG pAdcSrc)
{
	pAdcDst->FifoSize	=	pAdcSrc->FifoSize;
	pAdcDst->Bits		=	pAdcSrc->Bits;
	switch(pAdcSrc->Encoding)
	{
	case 0:
		pAdcDst->Encoding	= 2;
		break;
	case 1:
		pAdcDst->Encoding	= 0;
		break;
	case 2:
		pAdcDst->Encoding	= 7;
		break;
	default:
		pAdcDst->Encoding	=	pAdcSrc->Encoding;
	}
//	pAdcDst->Encoding	=	pAdcSrc->Encoding == 1 ? 0 : pAdcSrc->Encoding; // тип кодировки в прошивке (0 - two's complement, 1 - floating point, 2 - straight binary, 7 - Gray code)
	pAdcDst->MinRate	=	pAdcSrc->MinRate;
	pAdcDst->MaxRate	=	pAdcSrc->MaxRate;
	pAdcDst->InpRange	=	pAdcSrc->InpRange;
	pAdcDst->TetrNum	=	pAdcSrc->TetrNum;
}

//***************** End of file adcsubmodule.cpp *****************
