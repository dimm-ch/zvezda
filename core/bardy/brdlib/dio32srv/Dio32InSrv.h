/*
 ****************** File Dio32InSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DIO32IN section
 *
 * (C) InSys by Dorokhin Andrey Sep, 2007
 *
 ************************************************************
*/

#ifndef _DIO32INSRV_H
 #define _DIO32INSRV_H

#include "ctrldio32in.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "Dio32InRegs.h"

#define DIO32IN_TETR_ID 5

#pragma pack(push,1)

typedef struct _DIO32INSRV_CFG {
	UCHAR	isAlreadyInit;				// ���� ������������� (����� ������ �� ����������)
	UCHAR	BufType;					// ��� ������ (0 - LVTTL, 1 - TTL)
	UCHAR	Res[2];						// ������
	//ULONG		ExtClk;						// external clock frequency (������� ������� ������������ (��))
	ULONG		FifoSize;					// ������ FIFO (� ������)
	//ULONG		SubExtClk;					// external clock frequency of submodule (������� ������������ (��))
} DIO32INSRV_CFG, *PDIO32INSRV_CFG;

#pragma pack(pop)

class CDio32InSrv: public CService
{

protected:

	long m_Dio32InTetrNum;
	long m_MainTetrNum;

public:

	CDio32InSrv(int idx, int srv_num, CModule* pModule, PDIO32INSRV_CFG pCfg); // constructor

	int CtrlIsAvailable(
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
	int CtrlMaster(
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
	int CtrlClkMode(
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
	int CtrlFlagClear(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);
};

#endif // _DIO32INSRV_H

//
// End of file
//