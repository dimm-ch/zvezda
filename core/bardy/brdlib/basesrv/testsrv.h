/*
 ****************** File TestSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : Test section
 *
 * (C) InSys by Dorokhin Andrey Aug, 2005
 *
 * 02.10.2007 - add TESTIN
 *
 ************************************************************
*/

#ifndef _TESTSRV_H
 #define _TESTSRV_H

#include "ctrltest.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"

#define DLGDLLNAME_SIZE 64

const int TESTOUT_TETR_ID = 0x2D; // tetrad id
const int TESTIN_TETR_ID = 0x4C; // tetrad id

#pragma pack(push,1)

typedef struct _TESTSRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	UCHAR	isAlreadyInit;	// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[3];			// резерв
	U32		InFifoSize;						// размер FIFO тетрады IN (в байтах)
	U32		OutFifoSize;					// размер FIFO тетрады OUT (в байтах)
} TESTSRV_CFG, *PTESTSRV_CFG;

#pragma pack(pop)

class CTestSrv : public CService
{

protected:

	long m_MainTetrNum;
	long m_TestOutTetrNum;
	long m_TestInTetrNum;
	
	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	ULONG GetFifoSize(CModule* pModule, ULONG tetrNum);
	void TetradInit(CModule* pModule, ULONG tetrNum);

//	virtual int GetCfg(PBRD_SdramCfg pCfg);
//	virtual void* GetInfoForDialog(CModule* pModule);
//	virtual void FreeInfoForDialog(PVOID pInfo);
//	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

public:

	CTestSrv(int idx, int srv_num, CModule* pModule, PTESTSRV_CFG pCfg); // constructor

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
	//int CtrlGetPages(
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
	int CtrlMode(
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
	int CtrlSetData(
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
};

#endif // _TESTSRV_H

//
// End of file
//