/*
 ****************** File AdcSrv.h *******************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : ADC section
 *
 * (C) InSys by Dorokhin Andrey Apr 2004
 *
 ************************************************************
*/

#ifndef _ADC_SRV_H
 #define _ADC_SRV_H

#include "ctrladc.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "adcregs.h"

#include "adcsrvinfo.h"

#define DLGDLLNAME_SIZE 64

//#define BASE_MAXREFS 2
//const int BASE_MAXREFS = 2; // Number of referens generators on base module

extern const double BRD_ADC_MAXGAINTUN; // max gain tuning = %
//#define BRD_ADC_MAXGAINTUN	1. // max gain tuning = 1%

const int THRDACCNT = 8; // Number of threshold DACs

// ADC Threshold DAC number
enum {
	BRDtdn_ADC_BIAS0		= 3,	// bias of channel 0
	BRDtdn_ADC_BIAS1		= 4,	// bias of channel 1
	BRDtdn_ADC_GAINTUN0		= 5,	// gain tuning of channel 0
	BRDtdn_ADC_GAINTUN1		= 6,	// gain tuning of channel 1
	BRDtdn_ADC_CLKPHASE0	= 7,	// clock phase control of channel 0
	BRDtdn_ADC_CLKPHASE1	= 8		// clock phase control of channel 1
};

#pragma pack(push,1)

typedef struct _ADSRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];// название диалоговой dll
//	DEVS_API_PropDlg* pPropDlg;			// адрес функции, котрая будет вызываться из диалоговой dll для установки параметров
	U08		isAlreadyInit;				// флаг инициализации (чтобы делать ее однократно)
	U08		Res[2];						// резерв
	S08		TetrNum;					// номер тетрады (перебивает ID тетрады)
	U32		BaseRefGen[BASE_MAXREFS];	// frequency of generators (значения опорных генераторов (Гц))
	U32		SysRefGen;					// frequency of system generator (частота системного генератора (Гц))
	U32		BaseExtClk;					// external frequency of clock (внешняя частота тактирования (Гц))
	U32		RefPVS;						// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	U08		Bits;						// разрядность
	U08		Encoding;					// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
	U32		MinRate;					// минимальная частота дискретизации
	U32		MaxRate;					// максимальная частота дискретизации
	U16		InpRange;					// входной диапазон
	U32		FifoSize;					// размер FIFO АЦП (в байтах)
	U08		MaxChan;					// максимальное число каналов
	U08		ThrDac[THRDACCNT];			// значения 8-битовых пороговых ЦАП
	U16		ThrDacEx[THRDACCNT];		// значения 12/14/16-битовых пороговых ЦАП
} ADCSRV_CFG, *PADCSRV_CFG;

#pragma pack(pop)

class CAdcSrv: public CService
{

protected:

	HINSTANCE m_hLib;
	
	long m_AdcTetrNum;
	long m_MainTetrNum;

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
//	virtual int GetFifoParam(CModule* pModule, USHORT& FifoDeep, USHORT& FifoWidth);
	virtual void GetAdcTetrNum(CModule* pModule);
	virtual void GetAdcTetrNumEx(CModule* pModule, ULONG TetrInstNum);
	virtual int GetTraceText(CModule* pModule, U32 traceId, U32 sizeb, BRDCHAR *pText);
	virtual int GetTraceParam(CModule* pModule, U32 traceId, U32 sizeb, U32 *pRealSizeb, void *pParam );
	virtual void* GetInfoForDialog(CModule* pModule);
	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	virtual int SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int GetCfg(PBRD_AdcCfg pCfg);
	virtual int SetMaster(CModule* pModule, ULONG mode);
	virtual int GetMaster(CModule* pModule, ULONG& mode);
	virtual int SetChanMask(CModule* pModule, ULONG mask);
	virtual int GetChanMask(CModule* pModule, ULONG& mask);
	virtual int SetCode(CModule* pModule, ULONG type);
	virtual int GetCode(CModule* pModule, ULONG& type);
	//virtual int SetClkMode(CModule* pModule, PVOID pClkMode);
	//virtual int GetClkMode(CModule* pModule, PVOID pClkMode);
	//virtual int SetSyncMode(CModule* pModule, PVOID pSyncMode);
	//virtual int GetSyncMode(CModule* pModule, PVOID pSyncMode);
	virtual int SetClkSource(CModule* pModule, ULONG ClkSrc);
	virtual int GetClkSource(CModule* pModule, ULONG& ClkSrc);
	virtual int SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue);
	virtual int GetRate(CModule* pModule, double& Rate, double ClkValue);
	virtual int SetBias(CModule* pModule, double& Bias, ULONG Chan);
	virtual int GetBias(CModule* pModule, double& Bias, ULONG Chan);
	virtual int SetClkPhase(CModule* pModule, double& Phase, ULONG Chan);
	virtual int GetClkPhase(CModule* pModule, double& Phase, ULONG Chan);
	virtual int SetGainTuning(CModule* pModule, double& GainTuning, ULONG Chan);
	virtual int GetGainTuning(CModule* pModule, double& GainTuning, ULONG Chan);
	virtual int SetDcCoupling(CModule* pModule, ULONG InpType, ULONG Chan);
	virtual int GetDcCoupling(CModule* pModule, ULONG& InpType, ULONG Chan);
	virtual int SetInpFilter(CModule* pModule, ULONG isLpfOn, ULONG Chan);
	virtual int GetInpFilter(CModule* pModule, ULONG& refIsLpfOn, ULONG Chan);
	virtual int SetInpResist(CModule* pModule, ULONG InpRes, ULONG Chan);
	virtual int GetInpResist(CModule* pModule, ULONG& InpRes, ULONG Chan);
	virtual int SetStartMode(CModule* pModule, PVOID pStartMode);
	virtual int GetStartMode(CModule* pModule, PVOID pStartMode);
	virtual int SetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode);
	virtual int GetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode);
	virtual int SetPretrigMode(CModule* pModule, PBRD_PretrigMode pPreMode);
	virtual int GetPretrigMode(CModule* pModule, PBRD_PretrigMode pPreMode);
	virtual int GetPretrigEvent(CModule* pModule, ULONG& EventStart);
	virtual int SetDblClk(CModule* pModule, ULONG& mode);
	virtual int GetDblClk(CModule* pModule, ULONG& mode);
	virtual int SetClkLocation(CModule* pModule, ULONG& mode);
	virtual int GetClkLocation(CModule* pModule, ULONG& mode);
	virtual int SetClkThr(CModule* pModule, double& ClkThr);
	virtual int GetClkThr(CModule* pModule, double& ClkThr);
	virtual int SetExtClkThr(CModule* pModule, double& ExtClkThr);
	virtual int GetExtClkThr(CModule* pModule, double& ExtClkThr);
	virtual int SetClkInv(CModule* pModule, ULONG ClkInv);
	virtual int GetClkInv(CModule* pModule, ULONG& ClkInv);
	virtual int SetMu(CModule* pModule, void *args );
	virtual int GetMu(CModule* pModule, void *args );
	virtual int SetStartSlave(CModule* pModule, ULONG StartSlave);
	virtual int GetStartSlave(CModule* pModule, ULONG& refStartSlave);
	virtual int SetClockSlave(CModule* pModule, ULONG ClockSlave);
	virtual int GetClockSlave(CModule* pModule, ULONG& refClockSlave);
	virtual int GetChanOrder(CModule* pModule, BRD_ItemArray *pItemArray );
	virtual int SetInpSrc(CModule* pModule, PVOID pCtrl);
	virtual int GetInpSrc(CModule* pModule, PVOID pCtrl);
	virtual int SetGain(CModule* pModule, double& Gain, ULONG Chan);
	virtual int GetGain(CModule* pModule, double& Gain, ULONG Chan);
	virtual int SetInpRange(CModule* pModule, double& InpRange, ULONG Chan);
	virtual int GetInpRange(CModule* pModule, double& InpRange, ULONG Chan);
	virtual int SetTestMode(CModule* pModule, ULONG mode);
	virtual int GetTestMode(CModule* pModule, ULONG& mode);
	virtual int SetTestSeq(CModule* pModule, ULONG seq);
	virtual int GetTestSeq(CModule* pModule, ULONG& seq);
	virtual int StdDelay(CModule* pModule, int cmd, PBRD_StdDelay delay);
	virtual int SetSpecific(CModule* pModule, PBRD_AdcSpec pSpec);
	virtual int GetSpecific(CModule* pModule, PBRD_AdcSpec pSpec);
	virtual int SetTarget(CModule* pModule, ULONG target);
	virtual int GetTarget(CModule* pModule, ULONG& target);
	virtual int SetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal);
	virtual int GetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal);
	virtual int SelfClbr(CModule* pModule);
	
	virtual int SetTitleData(CModule* pModule, PULONG pTitleData);
	virtual int GetTitleData(CModule* pModule, PULONG pTitleData);

	virtual int ExtraInit(CModule* pModule);

	virtual int	InHalf(CModule* pModule, ULONG mode);

	virtual int FifoReset(CModule* pModule);
	virtual int ClrBitsOverflow(CModule* pModule, ULONG flags);
	virtual int IsBitsOverflow(CModule* pModule, ULONG& OverBits);
	virtual int AdcEnable(CModule* pModule, ULONG enable);
	virtual int PrepareStart(CModule *pModule, void *arg);
	virtual int IsComplex(CModule *pModule, U32 *pIsComplex );
	virtual int SetFc(CModule *pModule, double *pFc, U32 nChan);
	virtual int GetFc(CModule *pModule, double *pFc, U32 nChan);

	virtual int SetStartResist(CModule* pModule, ULONG InpRes );
	virtual int GetStartResist(CModule* pModule, ULONG& InpRes );
	virtual int GetAddrData(CModule* pModule, U32 *pAddrData );
	virtual int GetFifoStatus(CModule* pModule, U32 *pFifoStatus );
	virtual int GetFormat(CModule* pModule, U32 *pFormat );

	//
	// Read/Write Tetrada's Registers
	//
	S32	IndRegRead( CModule* pModule, S32 tetrNo, S32 regNo, U32 *pVal );
	//S32	IndRegRead( CModule* pModule, S32 tetrNo, S32 regNo, ULONG *pVal );
	S32	IndRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, U32 val );
	//S32	IndRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, ULONG val );

	S32	DirRegRead( CModule* pModule, S32 tetrNo, S32 regNo, U32 *pVal );
	//S32	DirRegRead( CModule* pModule, S32 tetrNo, S32 regNo, ULONG *pVal );
	S32	DirRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, U32 val );
	//S32	DirRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, ULONG val );

public:

    CAdcSrv(int idx, const BRDCHAR *name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize); // constructor
//	CAdcSrv(int idx, int srv_num, CModule* pModule, PADC212X200MSRV_CFG pCfg); // constructor

	int CtrlIsAvailable(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   );
	virtual int CtrlCapture(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetTraceText( void *pDev, void *pServData, ULONG cmd, void *args );
	int CtrlGetTraceParam( void *pDev, void *pServData, ULONG cmd, void *args );
	int CtrlGetPages(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);
	//int CtrlDlgProperty(
	//				void		*pDev,		// InfoSM or InfoBM
	//				void		*pServData,	// Specific Service Data
	//				ULONG		cmd,		// Command Code (from BRD_ctrl())
	//				void		*args 		// Command Arguments (from BRD_ctrl())
	//				);
	int CtrlIniFile(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlCfg(
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
	int CtrlBias(
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
	int CtrlStartMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlSyncMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlIsBitsOverflow(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlClkPhase(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlGainTuning(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlGetPretrigEvent(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetPreEventPrec(
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
	int CtrlEnable(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlPrepareStart( void *pDev, void *pServData, ULONG cmd, void *args );
	int CtrlIsComplex( void *pDev, void *pServData, ULONG cmd, void *args );
	int CtrlFc( void *pDev, void *pServData, ULONG cmd, void *args );
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

	int CtrlRate(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlFormat(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlCode(
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
	int CtrlDblClk(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlClkLocation(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlClkThr(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlExtClkThr(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlClkInv(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlMu(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlStartSlave(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlClockSlave(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlChanOrder(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlInpResist(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlDcCoupling(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlInpFilter(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlInpSrc(
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
	int CtrlPretrigMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlGain(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlInpRange(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlMaster(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );

	int CtrlInpMux(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlDifMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSelfClbr(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSampleHold(
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
	int CtrlTestMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlTestSeq(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlDelay(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlSpecific(
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
	int CtrlGetBlkCnt(
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
	int CtrlClrBitsOverflow(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   );
};

#endif // _ADC_SRV_H

//
// End of file
//
