/*
 ****************** File DdsSrv.h ***************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DDS section
 *
 * (C) InSys by Sklyarov A. Mar 2007
 *
 ************************************************************
*/

#ifndef _DDS_SRV_H
 #define _DDS_SRV_H

#include "ctrldds.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "ddsregs.h"
#include "ad9956.h"

#include "ddssrvinfo.h"

#define DLGDLLNAME_SIZE 64

const int BASEDDS_TETR_ID = 0x41; 
const int BDDSV2_TETR_ID = 0xCF; 

#pragma pack(push,1)

typedef struct _DDSSRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];// название диалоговой dll
	UCHAR	isAlreadyInit;				// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[2];						// резерв
	UCHAR	DdsType;					// type of DDS (0-non, 1-PLL, 2-DDS)
	double	BaseRefClk;
	double	ExtRefClk;
} DDSSRV_CFG, *PDDSSRV_CFG;

#pragma pack(pop)

class CDdsSrv: public CService
{

protected:

	HINSTANCE m_hLib;
	
	long m_DdsTetrNum;
	long m_MainTetrNum;
	AD9956_CFG CfgAD9956;

	UCHAR	m_DdsVer;						// 0 - TETR_ID=0x41, 1 - TETR_ID=0xCF

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	virtual int SetClkMode(CModule* pModule, PBRD_ClkMode pClkMode);
	virtual int GetClkMode(CModule* pModule, PBRD_ClkMode pClkMode);
	virtual int SetStartMode(CModule* pModule, PVOID pStMode);
	virtual int GetStartMode(CModule* pModule, PVOID pStMode);
	virtual int SetOutFreq(CModule* pModule, double  *OutFreq,int *ScaleCP,int *dividerM);
	virtual int GetOutFreq(CModule* pModule, double  *OutFreq,int *ScaleCP,int *dividerM);
	virtual int SetStbMode(CModule* pModule, PBRD_StbMode pStbMode);
	virtual int GetStbMode(CModule* pModule, PBRD_StbMode pStbMode);
	virtual int StartStb(CModule* pModule);
	virtual int StopStb(CModule* pModule);
	virtual int EnableDds(CModule* pModule);
	virtual int DisableDds(CModule* pModule);
	virtual int	SetCfgParamAD9956(PAD9956_CFG pcfg, double refClk, double *outFreq, int*clkSel, int *resDiv0,int *div0,int *resDiv1,int *div1,int *scale,int *divM);
	virtual int	ProgAD9956(CModule* pModule,PAD9956_CFG pcfg);
	virtual	int ReadRegsAD9956(CModule* pModule,PAD9956_CFG pcfg);
	virtual	int	CompareRegsAD9956(PAD9956_CFG pcfg1,PAD9956_CFG pcfg2);

	virtual int	SetCfgParamAD9956_V2(PAD9956_CFG pcfg, double refClk, double *outFreq, int *resDiv0,int *div0,int *resDiv1,int *div1,int *scale,int *divM);
	virtual int GetOutFreq_V2(CModule* pModule, double  *OutFreq,int *ScaleCP,int *dividerM);

	virtual int SetStartClk(CModule* pModule, U32* clksel);
	virtual int GetStartClk(CModule* pModule, U32* clksel);
	virtual int SetMuxOut(CModule* pModule, U32* output);
	virtual int GetMuxOut(CModule* pModule, U32* output);
	virtual int SetDdsPower(CModule* pModule, U32* power);
	virtual int GetDdsPower(CModule* pModule, U32* power);

public:

	CDdsSrv(int idx, int srv_num, CModule* pModule, PDDSSRV_CFG pCfg); // constructor

	int CtrlMaster(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlClkMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlOutFreq(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStbMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	
	int CtrlStartStopStb(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							);
	int CtrlEnableDisable(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStartMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlIsAvailable(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlCapture(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlGetStatus(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStartClk(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlMuxOut(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlDdsPower(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
};
#endif // _DDS_SRV_H

//
// End of file
//