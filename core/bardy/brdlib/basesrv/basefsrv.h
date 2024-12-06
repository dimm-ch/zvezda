/*
 ****************** File BasefSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : BASE of FMC-modules section
 *
 * (C) InSys by Dorokhin Andrey Mar 2012
 *
 ************************************************************
*/

#ifndef _BASEFSRV_H
 #define _BASEFSRV_H

#include "ctrlbasef.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"

#include "si571.h"

#define GENERATOR_DEVID 0
#define SWITCH_DEVID 1
#define SYNC_SWITCH_DEVID 2

#define GENERATOR_NUMB 0x49
#define SWITCH_NUMB 0x48

#pragma pack(push,1)

typedef struct _BASEFSRV_CFG {
//	U32		SysGen;				// system generator frequency (частота системного генератора (√ц))
	U08		SwitchType;			// type of switch (0-non, 1-тип1(FMC105P), 2-тип2(FMC106P))
	U08		AdrSwitch;			// адресный код коммутатора: 0x48 по умолчанию
	U08		Gen0Type;			// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si571)
	U32		RefGen0;			// частота генератора 0, если он непрограммируемый, или заводска€ частота (default 50 MHz)
	U32		RefMaxGen0;			// максимальна€ частота внутр. генератора (√ц)
	U08		AdrGen0;			// адресный код внутр. генератора: 0x49 по умолчанию
	REAL64	dGenFxtal;			// частота кварца (Fxtal) внутреннего генератора Si570/571 (√ц)
	UCHAR	isAlreadyInit;		// флаг инициализации (чтобы делать ее однократно)
	U08		SwitchDevId;		// номер устройства при программировании коммутатора
} BASEFSRV_CFG, *PBASEFSRV_CFG;

//const BASEFSRV_CFG BaseSrvCfg_dflt = {
//	2,			// SwitchType
//	0x48,		// AdrSwitch
//	0,			// Gen0Type
//	50000000,	// RefGen0
//	50000000,	// RefMaxGen0
//	0x49,		// AdrGen0
//	50000000.,	// dGenFxtal
//	0			// 
//};

#pragma pack(pop)

class CBasefSrv: public CService
{
protected:

	long m_MainTetrNum;
	int writeSpdDev(CModule* pModule, ULONG dev, ULONG num, ULONG syncr, ULONG reg, ULONG val);
	int readSpdDev(CModule* pModule, ULONG dev, ULONG num, ULONG syncr, ULONG reg, ULONG* pVal);

	int	Si571SetClkVal( CModule* pModule, double *pRate );
	int	Si571GetClkVal( CModule* pModule, double *pRate );

public:

	CBasefSrv(int idx, int srv_num, CModule* pModule, PBASEFSRV_CFG pCfg); // constructor

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
	int CtrlSwitchDev(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlOnOffSwitch(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlReadStatus(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlWriteSwitch(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
			   );
	int CtrlReadSwitch(
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
};

#endif // _BASEFSRV_H

//
// End of file
//