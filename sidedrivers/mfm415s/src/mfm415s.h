#ifndef _MFM415S_H_
	#define _MFM415S_H_

#include "brderr.h"
#include "brdapi.h"
#include "utypes.h"
#include "useful.h"
#include "submodule.h"
#include "icr.h"
#include "Icr00A5.h"

#include "fm415sSrv.h"

#define VER_MAJOR 0x00010000L
#define VER_MINOR 0x00000000L

const U32 MAX_SERVICES = 2;

#pragma pack(push,1)
typedef struct _FM415S_INI {
	U32 nGenAdr;
	U32 nGenRef;
	U32 nGenMax;
	U32 nGenType;
	U32 nDecim;
	U32 isPrintf;
} FM415S_INI, *PFM415S_INI;

typedef struct _FM415S_CFG {	
	U08 bSfpCnt;	//количество SFP
	U08 bGen1Type;	//тип генератора  0 - непрограммируемый, 1 - Si571, 2 - LMK61e2
	U08 bGen1Addr;	//адрес генератора
	U32 nGen1Ref;	//заводска€ установка √ц
	U32 nGen1RefMax;//максимальна€ частота √ц
	U32 nGen1Rate;	//частота дл€ установки √ц
	U32 nDecim;
	bool isPrintf;
} FM415S_CFG, *PFM415S_CFG;

const FM415S_CFG rFM415SCfg_dflt = {
	4,
	1,
	0x55,
	10000000,
	1000000000,
	156250000,
	0,
	false,
};
#pragma pack(pop)

class Cmfm415s : public CSubModule
{
protected:
	PFM415S_INI		m_pIniData;
	FM415S_CFG		m_SubCfg;
	ULONG			m_SubType;
	CFm415sSrv*		m_pFm415sSrv[MAX_SERVICES];

	const BRDCHAR* getClassName(void) { return _BRDC("CFm415s"); }
	void GetICR(PVOID pCfgMem, ULONG RealCfgSize, PICR_CfgAdm	pCfgAdm);
	ULONG SetConfig(PICR_CfgAdm pCfgAdm);
	void DeleteServices();

public:
	Cmfm415s();
	Cmfm415s(PBRD_InitData pInitData, long sizeInitData);
	~Cmfm415s();
	S32 Init(PUCHAR pSubmodEEPROM, BRDCHAR* pIniString = NULL);
	void GetDeviceInfo(BRD_Info* pInfo);
	void GetServ(PSERV_ListItem srv_list);
	ULONG SetServices();
	void SetSrvList(PSERV_ListItem srv_list);
};

#endif //_MFM415S_H_