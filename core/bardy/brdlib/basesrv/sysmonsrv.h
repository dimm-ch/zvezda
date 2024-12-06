/*
 ****************** File sysmonsrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : SYSMON service
 *
 * (C) InSys by Dorokhin A. Mar 2012
 *
 ************************************************************
*/

#ifndef _SYSMONSRV_H
 #define _SYSMONSRV_H

#include "ctrlsysmon.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"

#pragma pack(push,1)

typedef struct _SYSMONSRV_CFG {
	UCHAR	isAlreadyInit;		// флаг инициализации (чтобы делать ее однократно)
	UCHAR	PldType;			// тип ПЛИС
	USHORT	DeviceID;			// идентификатор базового модуля
	UCHAR	PldSpeedGrade;		// скорость ПЛИС в ее наименовании
} SYSMONSRV_CFG, *PSYSMONSRV_CFG;

#pragma pack(pop)

class CSysMonSrv: public CService
{

protected:

	long m_MainTetrNum;
	HANDLE m_hWarningEvent;	// событие: выход предупреждения системного монитора перешел в состояние 1

	U32 readData(CModule* pModule, int reg);
	U32 writeData(CModule* pModule, int reg, U32 val);

public:

	CSysMonSrv(int idx, int srv_num, CModule* pModule, PSYSMONSRV_CFG pCfg); // constructor

	int CtrlIsAvailable(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   );
	int CtrlTemp(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVccint(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVccaux(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVrefp(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVrefn(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlStatus(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVoltNominal(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVoltNominal7s(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlVccbram(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
};

#endif // _SYSMONSRV_H

//
// End of file
//