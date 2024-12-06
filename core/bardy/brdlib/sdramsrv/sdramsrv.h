/*
 ****************** File SdramSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : Sdram section
 *
 * (C) InSys by Dorokhin Andrey May, 2004
 *
 ************************************************************
*/

#ifndef _SDRAMSRV_H
 #define _SDRAMSRV_H

#include "ctrlsdram.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "sdramregs.h"
#include "sdramsrvinfo.h"

#define DLGDLLNAME_SIZE 64

const int SDRAM_TETR_ID = 0x18; // tetrad id

#pragma pack(push,1)

typedef struct _SDRAMSRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
//	DEVS_API_PropDlg* pPropDlg;			// адрес функции, котрая будет вызываться из диалоговой dll для установки параметров
	UCHAR	isAlreadyInit;	// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Mode;			// флаги возможностей ввода из памяти в хост (0x01)/ вывода в память из хоста (0x02)
	UCHAR	Res;			// резерв
	UCHAR	MemType;		// тип памяти
	UCHAR	SlotCnt;		// количество установленных слотов
	UCHAR	ModuleCnt;		// количество установленных DIMM-модулей (занятых слотов)
	UCHAR	RowAddrBits;	// количество разрядов адреса строк
	UCHAR	ColAddrBits;	// количество разрядов адреса столбцов
	UCHAR	ModuleBanks;	// количество банков в DIMM-модуле (Number of DIMM Ranks)
	UCHAR	ChipBanks;		// количество банков в микросхемах DIMM-модуля (Number of Banks on SDRAM Device)
} SDRAMSRV_CFG, *PSDRAMSRV_CFG;

#pragma pack(pop)

class CSdramSrv : public CService
{

protected:

	long m_MainTetrNum;
	long m_MemTetrNum;

	long m_MemTetrModif;
	
	uint64_t m_dwPhysMemSize; // physical memory size on device in 32-bit words
//	ULONG m_dwActMemSize; // active zone size in 32-bit words

#ifdef _WIN32
	HANDLE m_hAcqCompleteEvent;	// событие завершения сбора данных в память
	//IPC_handle m_hAcqCompleteEvent;	// событие завершения сбора данных в память
#endif

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	virtual int GetCfg(PBRD_SdramCfg pCfg);
	virtual int GetCfgEx(PBRD_SdramCfgEx pCfg);
	virtual void GetSdramTetrNum(CModule* pModule);
	virtual void GetSdramTetrNumEx(CModule* pModule, ULONG TetrInstNum);
	virtual void* GetInfoForDialog(CModule* pModule);
	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	virtual ULONG CheckCfg(CModule* pModule);
	virtual void MemInit(CModule* pModule, ULONG init);
	virtual int ExtraInit(CModule* pModule);

	ULONG GetStartAddr(CModule* pModule);
	ULONG GetEndAddr(CModule* pModule);
	ULONG GetTrigCnt(CModule* pModule);

	void GetStartStopMode(CModule* pModule, PBRD_StartMode pStartStopMode);
	void SetStartStopMode(CModule* pModule, PBRD_StartMode pStartStopMode);

	int SetReadMode(CModule* pModule, ULONG mode);
	int GetReadMode(CModule* pModule, ULONG& mode);
	int SetFifoMode(CModule* pModule, ULONG mode);
	int GetFifoMode(CModule* pModule, ULONG& mode);

	virtual int SetSel(CModule* pModule, ULONG& sel, ULONG cmd);
	virtual int SetTestSeq(CModule* pModule, ULONG mode);
	virtual int GetTestSeq(CModule* pModule, ULONG& mode);

public:

//	CSdramSrv(int idx, int srv_num, CModule* pModule, PSDRAMSRV_CFG pCfg); // constructor
	CSdramSrv(int idx, const BRDCHAR *name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize);
	~CSdramSrv(); // destructor

	int CtrlIsAvailable(
					void		*pSub,		// InfoSM or InfoBM
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
	int CtrlCfg(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlCfgEx(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlSetEnable(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetEnable(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlReadMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlSetMemSize(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetMemSize(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlSetStartAddr(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetStartAddr(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlSetReadAddr(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetReadAddr(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlSetPostTrig(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetPostTrig(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlSetStartMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetStartMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlGetAcqSize(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlIsAcqComplete(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlIsPassMemEnd(
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
	int CtrlPutData(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);
	int CtrlReadEnable(
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
	int CtrlIrqAcqComplete(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlWaitAcqComplete(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlWaitAcqCompleteEx(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
	int CtrlFlagClear(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlFifoMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	int CtrlSel(
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
};

#endif // _SDRAMSRV_H

//
// End of file
//