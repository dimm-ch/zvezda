/*
 ****************** File RegSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : COMMON section (register operations)
 *
 * (C) InSys by Dorokhin A. Nov 2007
 *
 ************************************************************
*/

#ifndef _REGSRV_H
 #define _REGSRV_H

#include "ctrlreg.h"
#include "service.h"
#include "adm2if.h"

#pragma pack(push,1)

typedef struct _REGSRV_CFG {
	UCHAR	isAlreadyInit;		// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Res[3];				// reserved
	USHORT	baseAdr;			// DEVBASEADR = 0x203 (address of SPD_DEVICE)
	USHORT	Res1;				// reserved
	//PULONG	Bar0;				// виртуальный базовый адрес пространства блоков устройства
} REGSRV_CFG, *PREGSRV_CFG;

#pragma pack(pop)

class CRegSrv: public CService
{

protected:

	long m_MainTetrNum;

	PULONG m_BaseAdrOper;	// виртуальный базовый адрес пространства блоков устройства
	PULONG m_BaseAdrTetr;	// виртуальный базовый адрес пространства тетрад устройства

public:

	CRegSrv(int idx, int srv_num, CModule* pModule, PREGSRV_CFG pCfg); // constructor

	int CtrlIsAvailable(
					void		*pDev,		// InfoSM or InfoBM
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
	int CtrlReadDir(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlReadsDir(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlReadInd(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlReadsInd(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlWriteDir(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlWritesDir(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlWriteInd(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlWritesInd(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlSetStatIrq(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlClearStatIrq(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	
	int CtrlWaitStatIrq(
		void		*pDev,		// InfoSM or InfoBM
		void		*pServData,	// Specific Service Data
		ULONG		cmd,		// Command Code (from BRD_ctrl())
		void		*args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlPackExec(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	
	int CtrlSpd(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );

	int CtrlReadHost(
		void		*pDev,		// InfoSM or InfoBM
		void		*pServData,	// Specific Service Data
		ULONG		cmd,		// Command Code (from BRD_ctrl())
		void		*args 		// Command Arguments (from BRD_ctrl())
	);

	int CtrlWriteHost(
		void		*pDev,		// InfoSM or InfoBM
		void		*pServData,	// Specific Service Data
		ULONG		cmd,		// Command Code (from BRD_ctrl())
		void		*args 		// Command Arguments (from BRD_ctrl())
	);

};

#endif // _REGSRV_H

//
// End of file
//