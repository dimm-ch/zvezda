/*
 ****************** File Gc5016Srv.h *******************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : GC5016 section
 *
 * (C) InSys by Sklyarov A. Nov. 2006
 *
 ************************************************************
*/

#ifndef _GC5016SRV_H
 #define _GC5016SRV_H

#include "ctrlgc5016.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "gc5016regs.h"


#define SINGLE_DIVIDER 512

const int DDC5016_TETR_ID = 0x30; // Tetrad ID ADMDDC5016
const int DDC416_TETR_ID = 0x53;  // Tetrad ID ADMDDC416x100M
const int DDC214_TETR_ID = 0x6A;  // Tetrad ID ADMDDC214x400M
const int DDCDR16_TETR_ID = 0x6B;  // Tetrad ID DR-16

enum {
	ADMDDC5016		= 0x0A0B,
	ADMDDC416x100M	= 0x0A0C,
	ADMDDC214x400M	= 0x0A0D
};

#pragma pack(push,1)

typedef struct _GC5016SRV_CFG {
	U32		isAlreadyInit;
	U32		FifoSize;
	U32		SubIntClk;
	U32		SubExtClk;
	U32		AdcBits;
	U32		AdcEncoding;
	U32		AdcMinRate;
	U32		AdcMaxRate;
	U32		AdcInpRange;
	U32		AdcCnt;
	U32		DdcBits;
	U32		DdcCnt;
	U32		SubType;	// тип субмодуля (ADMDDC5016 - 0x0A0B, ADMDDC416x100M - 0x0A0C, ADMDDC214x400M - 0x0A0D)
	U32		SubVer;

} GC5016SRV_CFG, *PGC5016SRV_CFG;

#pragma pack(pop)

class CGc5016Srv: public CService 
{

protected:

	long m_MainTetrNum;
	long m_GcTetrNum;
	U32	 m_SystemClock;
	DDC_CONST m_AdmConst;

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
//-------------------------------------------------------------------------------------
	virtual int  SetClkMode(CModule* pModule, PBRD_ClkMode pClkMode);
	virtual int  GetClkMode(CModule* pModule, PBRD_ClkMode pClkMode);
	virtual void GetGcTetrNum(CModule* pModule);
 	virtual int  GetCfg( PBRD_GC5016Cfg pCfg);
	virtual int	 WriteGcReg(CModule* pModule,void *args);
	virtual int  ReadGcReg(CModule* pModule, void *args);
	virtual int	 SetAdcGain(CModule* pModule,void *args);
	virtual int  GetAdcOver(CModule* pModule,void *args);
	virtual int  AdcEnable(CModule* pModule,void *args);
	virtual int  SetDdcFc(CModule* pModule, void *args);
	virtual int  SetDdcFcExt(CModule* pModule, void *args);
	virtual __int64 CalcCodeNcoFreq(int ddcmode,U32 freq, unsigned int *codeDphase);
	virtual int  SetAdmMode(CModule* pModule, void *args);
	virtual int  SetDdcMode(CModule* pModule, void *args);
	virtual int  SetStartMode(CModule* pModule, PVOID pStMode);
	virtual	int  GetStartMode(CModule* pModule, PVOID pStMode);
	virtual int  SetTarget(CModule* pModule, ULONG target);
	virtual int  GetTarget(CModule* pModule, ULONG& target);
	virtual int	 SetTestMode(CModule* pModule, ULONG mode);
	virtual int	 GetTestMode(CModule* pModule, ULONG& mode);
	 //-------------------------------------------------------------------------------------
	virtual int  SetChanMask(CModule* pModule, ULONG mask);
	virtual int  GetChanMask(CModule* pModule, ULONG& mask);
	virtual int  SetDdcSync(CModule* pModule, void *args);
	virtual int  ProgramDdc(CModule* pModule, void *args);
	virtual int  ProgramDdcExt(CModule* pModule, void *args);
	virtual int  VerifyProgDdc(CModule* pModule,U32 *regs,int nItems);
	virtual int  VerifyProgDdcExt(CModule* pModule,U32 *regs,int nItems,U32 maskDdc);
	virtual int  StartDdc(CModule* pModule);
	virtual int  StopDdc(CModule* pModule);
	virtual int  StartDdcExt(CModule* pModule,U32 maskDdc);
	virtual int  StopDdcExt(CModule* pModule,U32 maskDdc);
	virtual int  SetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal);
	virtual int  GetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal);
	virtual int  SetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode);
	virtual int  GetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode);
	virtual int  SetTitleData(CModule* pModule, PULONG pTitleData);
	virtual int  GetTitleData(CModule* pModule, PULONG pTitleData);
	virtual int  SetSpecific(CModule* pModule, PBRD_GC5016Spec pSpec);
	virtual int  GetSpecific(CModule* pModule, PBRD_GC5016Spec pSpec);
	virtual int  SetFlt(CModule* pModule, PBRD_SetFlt pSetFlt);
	virtual int  SetFft(CModule* pModule, PBRD_SetFft pSetFft);
	virtual int  SetFrame(CModule* pModule, PBRD_SetFrame pSetFrame);
	virtual int  SetNchans(CModule* pModule,void *args);
public:

	CGc5016Srv(int idx, int srv_num, CModule* pModule, PGC5016SRV_CFG pCfg); // constructor

	int CtrlMaster(
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
	int CtrlClkMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlChanMask(
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
	int CtrlFifoReset(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStart(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStop(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlFifoStatus(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlGetData(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlGetAddrData(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlTarget(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlTestMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlGcSetReg(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlGcGetReg(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetAdcGain(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlGetAdcOver(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlAdcEnable(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetDdcFc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetDdcFcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetAdmMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetDdcMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetDdcSync(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlProgramDdc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlProgramDdcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStartDdc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStopDdc(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStartDdcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlStopDdcExt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlCnt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlTitleMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlTitleData(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
    int  CtrlSpecific(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
    int  CtrlGetCfg(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetFlt(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetFft(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSetFrame(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
    int CtrlSetNchans(
                void		*pDev,		// InfoSM or InfoBM
                void		*pServData,	// Specific Service Data
                ULONG		cmd,		// Command Code (from BRD_ctrl())
                void		*args 		// Command Arguments (from BRD_ctrl())
                );


};

#endif // _GC5016SRV_H

//
// End of file
//
