/*
 ****************** File DdcSrv.h *******************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DDC section
 *
 * (C) InSys by Dorokhin Andrey Jan 2006
 *
 * 12.10.2006 - submodule ver 3.0 - synchronous mode
 *
 ************************************************************
*/

#ifndef _DDC_SRV_H
 #define _DDC_SRV_H

#include "ctrlddc.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "ddcregs.h"

#include "ddcsrvinfo.h"

const int ADMFPGA_TETR_ID = 0x20; // ADMFPGA tetrad id

#define DLGDLLNAME_SIZE 64

//#define BASE_MAXREFS 2
//const int THRDACCNT = 8; // Number of threshold DACs
// ADC Threshold DAC number
//enum {
//	BRDtdn_ADC_BIAS0		= 3,	// bias of channel 0
//	BRDtdn_ADC_BIAS1		= 4,	// bias of channel 1
//	BRDtdn_ADC_GAINTUN0		= 5,	// gain tuning of channel 0
//	BRDtdn_ADC_GAINTUN1		= 6,	// gain tuning of channel 1
//	BRDtdn_ADC_CLKPHASE0	= 7,	// clock phase control of channel 0
//	BRDtdn_ADC_CLKPHASE1	= 8		// clock phase control of channel 1
//};

#pragma pack(push,1)

typedef struct _DDCSRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];// название диалоговой dll
//	DEVS_API_PropDlg* pPropDlg;			// адрес функции, которая будет вызываться из диалоговой dll для установки параметров
	UCHAR	isAlreadyInit;				// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[3];						// резерв
	U32		FifoSize;					// размер FIFO DDC (в байтах)
	U08		MaxChan;					// максимальное число каналов
//	U08		ThrDac[THRDACCNT];			// значения пороговых ЦАП
} DDCSRV_CFG, *PDDCSRV_CFG;

#pragma pack(pop)

class CDdcSrv: public CService
{

protected:

	HINSTANCE m_hLib;

	long m_DdcTetrNum;
	long m_MainTetrNum;
	DDC_CONST m_AdmConst;

	virtual void GetDdcTetrNum(CModule* pModule);
	virtual void* GetInfoForDialog(CModule* pModule);
	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	virtual int SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int GetCfg(PBRD_DdcCfg pCfg);
	virtual int SetChanMask(CModule* pModule, ULONG mask);
	virtual int GetChanMask(CModule* pModule, ULONG& mask);
	virtual int SetFormat(CModule* pModule, ULONG format);
	virtual int GetFormat(CModule* pModule, ULONG& format);
	virtual int SetClkSource(CModule* pModule, ULONG ClkSrc);
	virtual int GetClkSource(CModule* pModule, ULONG& ClkSrc);
	virtual int SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue);
	virtual int GetRate(CModule* pModule, double& Rate, double ClkValue);
	virtual int SetStartMode(CModule* pModule, PVOID pStartMode);
	virtual int GetStartMode(CModule* pModule, PVOID pStartMode);
	virtual int SetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode);
	virtual int GetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode);
	virtual int SetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal);
	virtual int GetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal);

	virtual int SetTitleData(CModule* pModule, PULONG pTitleData);
	virtual int GetTitleData(CModule* pModule, PULONG pTitleData);

	virtual int SetInpSrc(CModule* pModule, ULONG& AdcNum, ULONG DdcChan);
	virtual int GetInpSrc(CModule* pModule, ULONG& AdcNum, ULONG DdcChan);
	virtual int SetFC(CModule* pModule, double& Freq, ULONG DdcChan);
	virtual int GetFC(CModule* pModule, double& Freq, ULONG DdcChan);
	virtual int SetDecim(CModule* pModule, double& Decim, ULONG DdcChan);
	virtual int GetDecim(CModule* pModule, double& Decim, ULONG DdcChan);
	virtual int SetFrame(CModule* pModule, ULONG Len);
	virtual int GetFrame(CModule* pModule, ULONG& Len);
	virtual int SetProgram(CModule* pModule, PVOID args);
	virtual int SetDDCSync(CModule* pModule, ULONG mode, ULONG prog, ULONG async);
	virtual int GetDDCSync(CModule* pModule, ULONG& mode, ULONG& async);

	virtual int StartEnable(CModule* pModule, ULONG Enbl);
	virtual int PrepareStart(CModule* pModule, void *arg);

	virtual int SetTestMode(CModule* pModule, ULONG mode);
	virtual int GetTestMode(CModule* pModule, ULONG& mode);
	virtual int SetTarget(CModule* pModule, ULONG target);
	virtual int GetTarget(CModule* pModule, ULONG& target);
	virtual int SetSpecific(CModule* pModule, PBRD_DdcSpec pSpec);
	virtual int GetSpecific(CModule* pModule, PBRD_DdcSpec pSpec);
	
public:

	CDdcSrv(int idx, const BRDCHAR* name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize); // constructor
//	CDdcSrv(int idx, int srv_num, CModule* pModule, PADC212X200MSRV_CFG pCfg); // constructor

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
	int CtrlGetPages(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);
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
	int CtrlFormat(
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
	int CtrlClkMode(
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
	int CtrlMaster(
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
	int CtrlFC(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlFrame(
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
	int CtrlProgram(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlDDCSync(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlDecim(
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
	int CtrlPrepareStart(void *pDev, void *pServData, ULONG cmd, void *args);
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
	int CtrlTarget(
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
	int CtrlCnt(
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
};

#endif // _DDC_SRV_H

//
// End of file
//