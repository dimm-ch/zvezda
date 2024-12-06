/*
 ***************** File dacsubmodule.cpp ***********************
 *
 * BRD Driver for ...
 *
 * (C) InSys by Dorokhin A. Jun 2007
 *
 ******************************************************
*/

//#include <windows.h>
//#include <winioctl.h>
//#include "gipcy.h"
#include "dacsubmodule.h"

#define	CURRFILE "DACSUBMODULE"

//******************************************
CDacSubModule::CDacSubModule() :
	CSubModule()
{
}

//******************************************
CDacSubModule::CDacSubModule(PBRD_InitData pBrdInitData, long sizeInitData) :
	CSubModule(pBrdInitData, sizeInitData)
{
}

//******************************************
//void CDacSubModule::GetDacIni(PDAC_INI pDacIni, PINIT_Data pInitData,long size)
void CDacSubModule::GetDacIni(PDAC_CFG pDacIni, PINIT_Data pInitData,long size)
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
ULONG CDacSubModule::SetDacConfig(PDAC_CFG pDacCfg, PICR_CfgAdmDac pDac, PICR_CfgDacFifo pDacFifo, int isTag, PDAC_CFG pDacIni)
{
	pDacCfg->FifoSize = 0;
	if(pDacFifo->wTag == DAC_FIFO_TAG)
		pDacCfg->FifoSize = (1 << pDacFifo->bDepth) *
								   ((1 << (pDacFifo->bBitsWidth + 2)) / 8);
	if(isTag)
	{	
		if(pDac->wTag == ADMDAC_CFG_TAG)
		{	// получаем конфигурацию ЦАП из ППЗУ
			pDacCfg->Bits = pDac->bBits;			// разрядность ЦАП
			pDacCfg->Encoding = pDac->bEncoding; // тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
			pDacCfg->MinRate = pDac->dMinRate;	// минимальная частота дискретизации
			pDacCfg->MaxRate = pDac->dMaxRate;	// максимальная частота дискретизации
			pDacCfg->OutRange = pDac->wRange;	// выходной диапазон
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
void CDacSubModule::SetDacCfg(PDACSRV_CFG pDacDst, PDAC_CFG pDacSrc)
{
	pDacDst->FifoSize	=	pDacSrc->FifoSize;
	pDacDst->Bits		=	pDacSrc->Bits;
	pDacDst->Encoding	=	pDacSrc->Encoding;
	pDacDst->MinRate	=	pDacSrc->MinRate;
	pDacDst->MaxRate	=	pDacSrc->MaxRate;
	pDacDst->OutRange	=	pDacSrc->OutRange;
	pDacDst->TetrNum	=	pDacSrc->TetrNum;
}

//***************** End of file dacsubmodule.cpp *****************
