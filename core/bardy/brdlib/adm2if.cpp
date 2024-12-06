/*
 ***************** File Adm2If.cpp ***********************
 *
 * BRD Driver for ADM2-interface tetrades
 *
 * (C) InSys by Dorokhin A. Jan 2004
 *
 ******************************************************
*/

#include "brdapi.h"
#include "adm2if.h"

#define	CURRFILE "ADM2IF"

//***************************************************************************************
long GetTetrNum(PVOID module, ULONG srvMainIdx, ULONG tetrId)
{
	CModule* pModule = (CModule*)module;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = srvMainIdx;
	int i = 0;
	for(i = 0; i < MAX_TETRNUM; i++)
	{
		RegParam.tetr = i;
		RegParam.reg = ADM2IFnr_ID;
		ULONG Status = pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		if(Status != BRDerr_OK)
			return -1;
		if(tetrId == RegParam.val)
			break;
	}
	long tetrNum = i;
	if(i >= MAX_TETRNUM)
		tetrNum = -1;
	return tetrNum;
}

//***************************************************************************************
long GetTetrNumEx(PVOID module, ULONG srvMainIdx, ULONG tetrId, ULONG instantNum)
{
	CModule* pModule = (CModule*)module;
	DEVS_CMD_Reg RegParam;
	RegParam.idxMain = srvMainIdx;
	int i = 0;
	for(i = 0; i < MAX_TETRNUM; i++)
	{
		RegParam.tetr = i;
		RegParam.reg = ADM2IFnr_ID;
		ULONG Status = pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		if(Status != BRDerr_OK)
			return -1;
		if(tetrId == RegParam.val)
		{
			RegParam.reg = ADM2IFnr_IDNUM;
			ULONG Status = pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			if(Status != BRDerr_OK)
				return -1;
			if(instantNum == RegParam.val)
				break;
		}
	}
	long tetrNum = i;
	if(i >= MAX_TETRNUM)
		tetrNum = -1;
	return tetrNum;
}

