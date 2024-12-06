/*
 ****************** File DataOutSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : Test section
 *
 * (C) InSys by Dorokhin Andrey Mar, 2007
 *
 ************************************************************
*/

#ifndef _DATAOUTSRV_H
 #define _DATAOUTSRV_H

#include "ctrldatainout.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"

#define DLGDLLNAME_SIZE 64

const int DATAOUT_TETR_ID = 0x12; // tetrad id

#pragma pack(push,1)

typedef struct _DATAOUTSRV_CFG {
	TCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	UCHAR	isAlreadyInit;					// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[3];							// резерв
	U32		OutFifoSize;					// размер FIFO тетрады OUT (в байтах)
} DATAOUTSRV_CFG, *PDATAOUTSRV_CFG;

#pragma pack(pop)

class CDataOutSrv : public CService
{

protected:

	long m_MainTetrNum;
	long m_DataOutTetrNum;
	
	ULONG GetFifoSize(CModule* pModule, ULONG tetrNum);
	void TetradInit(CModule* pModule, ULONG tetrNum);

//	virtual int GetCfg(PBRD_SdramCfg pCfg);
//	virtual void* GetInfoForDialog(CModule* pModule);
//	virtual void FreeInfoForDialog(PVOID pInfo);
//	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

public:

	CDataOutSrv(int idx, int srv_num, CModule* pModule, PDATAOUTSRV_CFG pCfg); // constructor

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
	int CtrlPutData(
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

#endif // _DATAOUTSRV_H

//
// End of file
//