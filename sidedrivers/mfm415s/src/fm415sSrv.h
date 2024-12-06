#ifndef _FM415SSRV_H
	#define _FM415SSRV_H

#include "service.h"
#include "adm2if.h"
#include "fm415sRegs.h"
#include "ctrlfm415s.h"
#include "si571.h"
#include "lmk61e2.h"

#define FM415S_TETR_ID 0x135
#define FM415S_AURORA_TETR_ID 0x136
#define DBG_printf	if(g_isPrintf)fprintf

extern int g_isPrintf;

#pragma pack(push,1)
typedef struct _FM415SSRV_CFG {
	UCHAR isAlreadyInit;
	U08 bSfpCnt;	//количество SFP
	U08 bGen1Type;	//тип генератора 0 - непрограммируемый, 1 - Si571, 2 - LMK61e2
	U08 bGen1Addr;	//адрес генератора
	U32 nGen1Ref;	//заводска€ установка √ц
	U32 nGen1RefMax;//максимальна€ частота √ц
	U32 nGen1Rate;	//частота дл€ установки √ц
	double dGenFxtal;// частота кварца (Fxtal) внутреннего генератора (√ц)
	U32 nChanMask;
	U32 nDirChanMask; //0-Rx, 1-Tx
	U32 nDecim;
	bool isPrintf;
} FM415SSRV_CFG, *PFM415SSRV_CFG;

#pragma pack(pop)

class CFm415sSrv : public CService
{
protected:
	long m_Fm415sTetrNum;
	long m_AuroraTetrNum[4];
	long m_MainTetrNum;
	long m_SdramTetrNum;
	long m_SrvNum;
	long m_nAuroras;

	int ReadErrData16(void* pDev, U08 nChan, U32 nReg, U32& nVal);
	int ReadErrData32(void* pDev, U08 nChan, U32 nReg, U32& nVal);
	int ReadErrData64(void* pDev, U08 nChan, U32 nReg, unsigned long long& nVal);
public:

	CFm415sSrv(int idx, int srv_num, CModule* pModule, PFM415SSRV_CFG pCfg);

	int CtrlIsAvailable(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlChanMask(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlClkMode(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlStatus(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlTestMode(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlStart(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlTestResult(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlDir(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlRxTx(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlRate(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlPrepare(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlDecim(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	virtual int CtrlRelease(
		void* pDev,		// InfoSM or InfoBM
		void* pServData,	// Specific Service Data
		ULONG cmd,		// Command Code (from BRD_ctrl())
		void* args 		// Command Arguments (from BRD_ctrl())
	);

	int	SpdRead(CModule* pModule, U32 spdType, U32 regAdr, U32* pRegVal);
	int	SpdWrite(CModule* pModule, U32 spdType, U32 regAdr, U32 regVal);
	int	Si571SetRate(CModule* pModule, double* pRate);
	int	Si571GetRate(CModule* pModule, double* pRate);
	int Si571GetFxTal(CModule* pModule, double* dGenFxTal);
	int	Lmk61e2SetRate(CModule* pModule, double* pRate);
	int	Lmk61e2GetRate(CModule* pModule, double* pRate);
};

#endif //_FM415SSRV_H