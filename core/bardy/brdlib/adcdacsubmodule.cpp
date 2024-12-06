/*
 ***************** File adcdacsubmodule.cpp ***********************
 *
 * BRD Driver for ...
 *
 * (C) InSys by Dorokhin A. Jun 2007
 *
 ******************************************************
*/

#include "adcdacsubmodule.h"

#define	CURRFILE "ADCDACSUBMODULE"

//=******************************************
CAdcDacSubModule::CAdcDacSubModule() :
	CSubModule()
{
}

//=******************************************
CAdcDacSubModule::CAdcDacSubModule(PBRD_InitData pBrdInitData, long sizeInitData) :
	CSubModule(pBrdInitData, sizeInitData)
{
}

//=******************************************
void CAdcDacSubModule::GetAdcIni(PADC_CFG pAdcIni, PINIT_Data pInitData,long size)
{
	FindKeyWord(_BRDC("bits"), pAdcIni->Bits, pInitData, size);
	FindKeyWord(_BRDC("encoding"), pAdcIni->Encoding, pInitData, size);
	FindKeyWord(_BRDC("minrate"), pAdcIni->MinRate, pInitData, size);
	FindKeyWord(_BRDC("maxrate"), pAdcIni->MaxRate, pInitData, size);
	FindKeyWord(_BRDC("inprange"), pAdcIni->InpRange, pInitData, size);
	FindKeyWord(_BRDC("adcfifosize"), pAdcIni->FifoSize, pInitData, size);
	FindKeyWord(_BRDC("adctetrnum"), pAdcIni->TetrNum, pInitData, size);
}

//=***************************************************************************************
ULONG CAdcDacSubModule::SetAdcConfig(PADC_CFG pAdcCfg, PICR_CfgAdc pAdc, PICR_CfgAdcFifo pAdcFifo, int isTag, PADC_CFG pAdcIni)
{
	if(isTag)
	{	
		pAdcCfg->FifoSize = 0;
		if(pAdc->wTag == ADC_CFG_TAG)
		{	// получаем конфигурацию АЦП из ППЗУ
			pAdcCfg->Bits = pAdc->bBits;			// разрядность АЦП
			pAdcCfg->Encoding = pAdc->bEncoding; // тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
			pAdcCfg->MinRate = pAdc->dMinRate;	// минимальная частота дискретизации
			pAdcCfg->MaxRate = pAdc->dMaxRate;	// максимальная частота дискретизации
			pAdcCfg->InpRange = pAdc->wRange;	// входной диапазон
			if(pAdcFifo->wTag == ADC_FIFO_TAG)
				pAdcCfg->FifoSize = (1 << pAdcFifo->bDepth) *
										   ((1 << (pAdcFifo->bBitsWidth + 2)) / 8);
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
void CAdcDacSubModule::SetAdcCfg(PADCSRV_CFG pAdcDst, PADC_CFG pAdcSrc)
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
//	pAdcDst->Encoding	=	pAdcSrc->Encoding;
	pAdcDst->MinRate	=	pAdcSrc->MinRate;
	pAdcDst->MaxRate	=	pAdcSrc->MaxRate;
	pAdcDst->InpRange	=	pAdcSrc->InpRange;
	pAdcDst->TetrNum	=	pAdcSrc->TetrNum;
}

//******************************************
//void CDacSubModule::GetDacIni(PDAC_INI pDacIni, PINIT_Data pInitData,long size)
void CAdcDacSubModule::GetDacIni(PDAC_CFG pDacIni, PINIT_Data pInitData,long size)
{
	FindKeyWord(_BRDC("bits"), pDacIni->Bits, pInitData, size);
	FindKeyWord(_BRDC("encoding"), pDacIni->Encoding, pInitData, size);
	FindKeyWord(_BRDC("minrate"), pDacIni->MinRate, pInitData, size);
	FindKeyWord(_BRDC("maxrate"), pDacIni->MaxRate, pInitData, size);
	FindKeyWord(_BRDC("outrange"), pDacIni->OutRange, pInitData, size);
	FindKeyWord(_BRDC("dacfifosize"), pDacIni->FifoSize, pInitData, size);
	FindKeyWord(_BRDC("dactetrnum"), pDacIni->TetrNum, pInitData, size);
}

//***************************************************************************************
//ULONG CDacSubModule::SetDacConfig(PDAC_CFG pDacCfg, PICR_CfgDac pDac, PICR_CfgDacFifo pDacFifo, int isTag, PDAC_INI pDacIni)
ULONG CAdcDacSubModule::SetDacConfig(PDAC_CFG pDacCfg, PICR_CfgAdmDac pDac, PICR_CfgDacFifo pDacFifo, int isTag, PDAC_CFG pDacIni)
{
	if(isTag)
	{	
		pDacCfg->FifoSize = 0;
		if(pDac->wTag == ADMDAC_CFG_TAG)
		{	// получаем конфигурацию ЦАП из ППЗУ
			pDacCfg->Bits = pDac->bBits;			// разрядность ЦАП
			pDacCfg->Encoding = pDac->bEncoding; // тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
			pDacCfg->MinRate = pDac->dMinRate;	// минимальная частота дискретизации
			pDacCfg->MaxRate = pDac->dMaxRate;	// максимальная частота дискретизации
			pDacCfg->OutRange = pDac->wRange;	// выходной диапазон
			if(pDacFifo->wTag == DAC_FIFO_TAG)
				pDacCfg->FifoSize = (1 << pDacFifo->bDepth) *
										   ((1 << (pDacFifo->bBitsWidth + 2)) / 8);
		}
	}
	if(pDacIni)
	{	// уточняем конфигурацию из источника инициализации
		if(pDacIni->Bits != 0xffffffff)
			pDacCfg->Bits = pDacIni->Bits;
		if(pDacIni->Encoding != 0xffffffff)
			pDacCfg->Encoding = pDacIni->Encoding;
		if(pDacIni->MinRate != 0xffffffff)
			pDacCfg->MinRate = pDacIni->MinRate;
		if(pDacIni->MaxRate != 0xffffffff)
			pDacCfg->MaxRate = pDacIni->MaxRate;
		if(pDacIni->OutRange != 0xffffffff)
			pDacCfg->OutRange = pDacIni->OutRange;
		if(pDacIni->FifoSize != 0xffffffff)
			pDacCfg->FifoSize = pDacIni->FifoSize;
		//if(pDacIni->TetrNum != 0xffffffff)
			pDacCfg->TetrNum = pDacIni->TetrNum;
	}

	return BRDerr_OK;
}

//******************************************
void CAdcDacSubModule::SetDacCfg(PDACSRV_CFG pDacDst, PDAC_CFG pDacSrc)
{
	pDacDst->FifoSize	=	pDacSrc->FifoSize;
	pDacDst->Bits		=	pDacSrc->Bits;
	switch(pDacSrc->Encoding)
	{
	case 0:
		pDacDst->Encoding	= 2;
		break;
	case 1:
		pDacDst->Encoding	= 0;
		break;
	case 2:
		pDacDst->Encoding	= 7;
		break;
	default:
		pDacDst->Encoding	=	pDacSrc->Encoding;
	}
//	pDacDst->Encoding	=	pDacSrc->Encoding;
	pDacDst->MinRate	=	pDacSrc->MinRate;
	pDacDst->MaxRate	=	pDacSrc->MaxRate;
	pDacDst->OutRange	=	pDacSrc->OutRange;
	pDacDst->TetrNum	=	pDacSrc->TetrNum;
}

//***************** End of file adcdacsubmodule.cpp *****************
