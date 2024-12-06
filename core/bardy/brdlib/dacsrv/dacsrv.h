/*
 ****************** File DacSrv.h ***************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DAC section
 *
 * (C) InSys by Dorokhin Andrey Oct 2005
 *
 ************************************************************
*/

#ifndef _DAC_SRV_H
 #define _DAC_SRV_H

#include "ctrldac.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "dacregs.h"

#include "dacsrvinfo.h"

#define DLGDLLNAME_SIZE 64

#define BASE_MAXREFS 2

#pragma pack(push,1)

typedef struct _DASRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];// название диалоговой dll
//	DEVS_API_PropDlg* pPropDlg;			// адрес функции, котрая будет вызываться из диалоговой dll для установки параметров
	UCHAR	isAlreadyInit;				// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[1];						// резерв
	S08		TetrNum;					// номер тетрады (перебивает ID тетрады)
	UCHAR	FifoType;					// тип FIFO: 0 - внутреннее, 1 - внешнее
	U32		BaseRefGen[BASE_MAXREFS];	// frequency of generators (значения опорных генераторов (Гц))
	U32		SysRefGen;					// frequency of system generator (частота системного генератора (Гц))
	U32		BaseExtClk;					// external frequency of clock (внешняя частота тактирования (Гц))
	U32		RefPVS;						// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	U08		Bits;						// разрядность
	U08		Encoding;					// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
	U32		MinRate;					// минимальная частота дискретизации
	U32		MaxRate;					// максимальная частота дискретизации
	U16		OutRange;					// выходной диапазон
	U32		FifoSize;					// размер FIFO ЦАП (в байтах)
	U08		MaxChan;					// максимальное число каналов
} DACSRV_CFG, *PDACSRV_CFG;

#pragma pack(pop)

class CDacSrv: public CService
{

protected:

	HINSTANCE m_hLib;
	
	long m_DacTetrNum;
	long m_MainTetrNum;

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	virtual ULONG GetParamForStream(CModule* pModule);
	virtual void GetDacTetrNum(CModule* pModule);
	virtual int GetTraceText(CModule* pModule, U32 traceId, U32 sizeb, BRDCHAR *pText);
	virtual int GetTraceParam(CModule* pModule, U32 traceId, U32 sizeb, U32 *pRealSizeb, void *pParam );
	virtual void* GetInfoForDialog(CModule* pModule);
	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	virtual int SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	//virtual int GetCfg(PBRD_DacCfg pCfg);
	virtual int GetCfg(PVOID pCfg);
	virtual int SetChanMask(CModule* pModule, ULONG mask);
	virtual int GetChanMask(CModule* pModule, ULONG& mask);
	virtual int SetCode(CModule* pModule, ULONG type);
	virtual int GetCode(CModule* pModule, ULONG& type);
	virtual int SetClkSource(CModule* pModule, ULONG ClkSrc);
	virtual int GetClkSource(CModule* pModule, ULONG& ClkSrc);
	virtual int SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int SetClkRefSource(CModule* pModule, ULONG ClkSrc,ULONG RefSrc);
	virtual int GetClkRefSource(CModule* pModule, ULONG& ClkSrc,ULONG& RefSrc);
	virtual int SetClkRefValue(CModule* pModule, ULONG ClkSrc,ULONG RefSrc, double& ClkValue,double& RefValue);
	virtual int GetClkRefValue(CModule* pModule, ULONG ClkSrc,ULONG RefSrc, double& ClkValue,double& RefValue);
	virtual int SetDivClk(CModule* pModule, ULONG& DivClk, double& rate);
	virtual int GetDivClk(CModule* pModule, ULONG& DivClk,double& rate);
	virtual int SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue);
	virtual int GetRate(CModule* pModule, double& Rate, double ClkValue);
	virtual int SetBias(CModule* pModule, double& Bias, ULONG Chan);
	virtual int GetBias(CModule* pModule, double& Bias, ULONG Chan);
	virtual int SetStartMode(CModule* pModule, PVOID pStartMode);
	virtual int GetStartMode(CModule* pModule, PVOID pStartMode);
	virtual int SetGain(CModule* pModule, double& Gain, ULONG Chan);
	virtual int GetGain(CModule* pModule, double& Gain, ULONG Chan);
	virtual int SetOutRange(CModule* pModule, double& OutRange, ULONG Chan);
	virtual int GetOutRange(CModule* pModule, double& OutRange, ULONG Chan);
	virtual int SetTestMode(CModule* pModule, ULONG mode);
	virtual int GetTestMode(CModule* pModule, ULONG& mode);
	virtual int StdDelay(CModule* pModule, int cmd, PBRD_StdDelay delay);
	virtual int SetSpecific(CModule* pModule, PBRD_DacSpec pSpec);
	virtual int GetSpecific(CModule* pModule, PBRD_DacSpec pSpec);
	virtual int SetResist(CModule* pModule, ULONG nInpRes, ULONG nChan );
	virtual int GetResist(CModule* pModule, ULONG &nInpRes, ULONG nChan );
	virtual int SetSource(CModule* pModule, ULONG source);
	virtual int GetSource(CModule* pModule, ULONG& source);
	virtual int SelfClbr(CModule* pModule);

	virtual int SetInterpFactor(CModule* pModule, ULONG InterpFactor );
	virtual int GetInterpFactor(CModule* pModule, ULONG &InterpFactor );
	virtual int DacEnable(CModule* pModule, ULONG enable);
	virtual int PrepareStart(CModule* pModule, void *arg);

	virtual int ExtraInit(CModule* pModule);

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

	CDacSrv(int idx, const BRDCHAR* name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize); // constructor

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
	int CtrlClkRefMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlDivClk(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlInterp(
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
	int CtrlDelay(
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
	int CtrlSelfClbr(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlIsBitsUnderflow(
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
	int CtrlEnableDisable(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlPrepareStart(void *pDev, void *pServData, ULONG cmd, void *args);
	int CtrlResist(void *pDev, void *pServData, ULONG cmd, void *args);
	int CtrlFifoStatus(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlPutData(
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
	int CtrlSource(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlStDelay(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlOutData(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlSkipData(
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
	int CtrlOutRange(
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
	int CtrlCyclingMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlOutSync(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);

};

#endif // _DAC_SRV_H

//
// End of file
//
