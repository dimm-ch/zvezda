/*
 ****************** File SdramAmbpcdSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : SdramAmbpcd section
 *
 * (C) InSys by Dorokhin Andrey May, 2004
 *
 ************************************************************
*/

#ifndef _SDRAMAMBPCDSRV_H
 #define _SDRAMAMBPCDSRV_H

#include "ctrlsdram.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "sdramregs.h"
#include "sdramsrv.h"

#include "sdramambpcdsrvinfo.h"

const int SDRAMAMBPCD_TETR_ID = 0x25; // tetrad id
const int SDRAMAMBPCDRD_TETR_ID = 0x4B; // tetrad id
const int SDRAMVK3RD_TETR_ID = 0x67; // tetrad id SDRAM of VK3
const int SDRAMAMBPEX2_TETR_ID = 0x6F; // tetrad id SDRAM of AMBPEX2
const int SDRAMAMBPEX5_TETR_ID = 0x70; // tetrad id SDRAM of AMBPEX5
const int SDRAMAMBPEX2RD_TETR_ID = 0x77; // tetrad id SDRAM of AMBPEX2
//const int SDRAMFMC106P_TETR_ID = 0x8F; // tetrad id SDRAM of AMBPEX5
//const int SDRAMDDR3X_TETR_ID = 0x9B;  // tetrad id SDRAM of FMC106P - autor DSMV

#pragma pack(push,1)

typedef struct _SDRAMAMBPCDSRV_CFG {
	SDRAMSRV_CFG SdramCfg;	//
	UCHAR	PrimWidth;		// Primary DDR SDRAM Width
	UCHAR	CasLatency;		// задержка сигнала выборки по столбцам
	UCHAR	Attributes;		// bit 1 = 1 - registered memory
} SDRAMAMBPCDSRV_CFG, *PSDRAMAMBPCDSRV_CFG;

#pragma pack(pop)

class CSdramAmbpcdSrv : public CSdramSrv
{

protected:

	virtual void GetSdramTetrNum(CModule* pModule);
	virtual void* GetInfoForDialog(CModule* pModule);
	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	UCHAR ReadSpdByte(CModule* pModule, ULONG OffsetSPD, ULONG CtrlSPD);
	ULONG GetCfgFromSpd(CModule* pModule, PSDRAMAMBPCDSRV_CFG pCfgSPD);
	virtual ULONG CheckCfg(CModule* pModule);
	virtual void MemInit(CModule* pModule, ULONG init);

public:

	CSdramAmbpcdSrv(int idx, int srv_num, CModule* pModule, PSDRAMAMBPCDSRV_CFG pCfg); // constructor
	
};

#endif // _SDRAMAMBPCDSRV_H

//
// End of file
//