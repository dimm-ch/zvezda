/*
 ****************** File Dac1624x192Srv.h *******************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DAC1624X192 section
 *
 * (C) InSys by Dorokhin Andrey Jun 2007
 *
 ************************************************************
*/

#ifndef _DAC1624X192SRV_H
 #define _DAC1624X192SRV_H

#include "ctrldac1624x192.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "dacsrv.h"
#include "mainregs.h"
#include "dac1624x192regs.h"

#include "dac1624x192srvinfo.h"

const int DAC1624X192_TETR_ID = 0x3E; // tetrad id

#pragma pack(push,1)

typedef struct _DAC1624X192SRV_CFG {
	DACSRV_CFG DacCfg;
	U32		ChipCnt;		// число установленных микросхем ЦАП
} DAC1624X192SRV_CFG, *PDAC1624X192SRV_CFG;

#pragma pack(pop)

class CDac1624x192Srv: public CDacSrv
{

protected:

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	virtual void GetDacTetrNum(CModule* pModule);
	virtual void* GetInfoForDialog(CModule* pModule);
//	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	int SyncPulse(CModule* pModule, double msPeriod);
	int SetMute(CModule* pModule, ULONG mode);
	int GetMute(CModule* pModule, ULONG& mode);

	virtual int SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue);
	virtual int GetRate(CModule* pModule, double& Rate, double ClkValue);
	virtual int SetCode(CModule* pModule, ULONG type);
	virtual int GetCode(CModule* pModule, ULONG& type);
	virtual int SelfClbr(CModule* pModule);

	virtual int SetSpecific(CModule* pModule, PBRD_DacSpec pSpec);

public:

	CDac1624x192Srv(int idx, int srv_num, CModule* pModule, PDAC1624X192SRV_CFG pCfg); // constructor

};

#endif // _DAC1624X192SRV_H

//
// End of file
//