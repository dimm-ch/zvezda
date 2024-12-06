/*
 ****************** File DataInSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : Test section
 *
 * (C) InSys by Dorokhin Andrey Mar, 2007
 *
 ************************************************************
*/

#ifndef _DATAINSRV_H
 #define _DATAINSRV_H

#include "ctrldatainout.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"

#define DLGDLLNAME_SIZE 64

const int DATAIN_TETR_ID = 0x13; // tetrad id

#pragma pack(push,1)

typedef struct _DATAINSRV_CFG {
	TCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	UCHAR	isAlreadyInit;					// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[3];							// резерв
	U32		InFifoSize;						// размер FIFO тетрады IN (в байтах)
} DATAINSRV_CFG, *PDATAINSRV_CFG;

#pragma pack(pop)

class CDataInSrv : public CService
{

protected:

	long m_MainTetrNum;
	long m_DataInTetrNum;
	
	ULONG GetFifoSize(CModule* pModule, ULONG tetrNum);
	void TetradInit(CModule* pModule, ULONG tetrNum);

//	virtual int GetCfg(PBRD_SdramCfg pCfg);
//	virtual void* GetInfoForDialog(CModule* pModule);
//	virtual void FreeInfoForDialog(PVOID pInfo);
//	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

public:

	CDataInSrv(int idx, int srv_num, CModule* pModule, PDATAINSRV_CFG pCfg); // constructor

	int CtrlIsAvailable(
					void		*pSub,		// InfoSM or InfoBM
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
	int CtrlGetAddrData(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
};

#endif // _DATAINSRV_H

//
// End of file
//