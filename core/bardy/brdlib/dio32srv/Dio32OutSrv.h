/*
 ****************** File Dio32OutSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DIO32OUT section
 *
 * (C) InSys by Dorokhin Andrey Sep, 2007
 *
 ************************************************************
*/

#ifndef _DIO32OUTSRV_H
 #define _DIO32OUTSRV_H

#include "ctrldio32out.h"
#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "Dio32OutRegs.h"

#define BASE_MAXREFS 2

#define DIO32OUT_TETR_ID 4

#pragma pack(push,1)

typedef struct _DIO32OUTSRV_CFG {
	UCHAR	isAlreadyInit;				// флаг инициализации (чтобы делать ее однократно)
	UCHAR	BufType;					// тип буфера (0 - LVTTL, 1 - TTL)
	UCHAR	Res[2];						// резерв
	ULONG	BaseRefGen[BASE_MAXREFS];	// frequency of generators (значени€ опорных генераторов (√ц))
	ULONG	SysRefGen;					// frequency of bus clock (частота тактировани€ шины (√ц))
	ULONG	BaseExtClk;					// external frequency of clock (внешн€€ частота тактировани€ (√ц))
	//ULONG	SubExtClk;					// frequency of bus clock (частота тактировани€ (√ц))
	ULONG	FifoSize;					// размер FIFO (в байтах)
} DIO32OUTSRV_CFG, *PDIO32OUTSRV_CFG;

#pragma pack(pop)

class CDio32OutSrv: public CService
{

protected:

	long m_Dio32OutTetrNum;
	long m_MainTetrNum;

	int SetClkSource(CModule* pModule, ULONG ClkSrc);
	int GetClkSource(CModule* pModule, ULONG& ClkSrc);
	int SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	int GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	int SetStartMode(CModule* pModule, PVOID pStartMode);
	int GetStartMode(CModule* pModule, PVOID pStartMode);

public:

	CDio32OutSrv(int idx, int srv_num, CModule* pModule, PDIO32OUTSRV_CFG pCfg); // constructor

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
	int CtrlStartMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlPnpkMode(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlInitMode(
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