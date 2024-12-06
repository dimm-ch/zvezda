/*
 ***************** File SdramAmbpcxSrv.cpp ***********************
 *
 * BRD Driver for ADM-interface SdramAmbpcx service
 *
 * (C) InSys by Dorokhin A. Feb 2005
 *
 ******************************************************
*/

#include "module.h"
#include "sdramambpcxsrv.h"

#define	CURRFILE "SDRAMAMBPCXSRV"

//***************************************************************************************
CSdramAmbpcxSrv::CSdramAmbpcxSrv(int idx, int srv_num, CModule* pModule, PSDRAMSRV_CFG pCfg) :
	CSdramSrv(idx, _BRDC("BASESDRAM"), srv_num, pModule, pCfg, sizeof(SDRAMSRV_CFG))
{
}

//***************************************************************************************
void CSdramAmbpcxSrv::GetSdramTetrNum(CModule* pModule)
{
	m_MemTetrNum = GetTetrNum(pModule, m_index, SDRAMAMBPCX_TETR_ID);
}


// ***************** End of file SdramAmbpcxSrv.cpp ***********************
