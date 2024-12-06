/*
 ****************** File submodule.h ************************
 *
 * Definitions of structures and constants
 * of derived class CSubModule for Data Acquisition Boards.
 *
 * (C) InSys by Dorokhin A. Mar. 2004
 *
 *
 *********************************************************
*/

#ifndef _SUBMODULE_H_
 #define _SUBMODULE_H_

#include	"service.h"
#include	"module.h"
#include	"adm2ifcfg.h"
#include	"icr.h"
#include	"IcrAmbp.h"
#include	"IcrAmbpex.h"
#include	"icramb3uv.h"
//#include	"Icrfmc105p.h"

#define DLGDLLNAME_SIZE 64

#pragma pack(push,1)

// Struct info about base module
typedef struct _BASE_Info {
	S32				type;		// Base module Type
	S32				version;	// Base module Version
    BRDCHAR			boardName[BOARDNAME_SIZE];	// Device Name
	HINSTANCE		hLib;		// Dll Handle 
	DEVS_API_entry	*pEntry;	// DEVS API Entry Point
	void			*pDev;		// DEV Driver Extension
} BASE_Info, *PBASE_Info;

// общие инициализационные параметры для всех субмодулей
typedef struct _SUB_INI {
	U32		PID;			// серийный (физический) номер
	U32		Type;			// тип ADM
	U32		Version;		// версия ADM
	U32		BaseRefGen[BASE_MAXREFS];	// frequency of generators (значения опорных генераторов (Гц))
	U32		SysRefGen;		// frequency of system generator (частота системного генератора (Гц))
	U32		BaseExtClk;		// external frequency of clock (внешняя частота тактирования (Гц))
	U32		RefPVS;			// Reference Voltage (опорное напряжение источников программируемых напряжений (мВольт))
} SUB_INI, *PSUB_INI;

#pragma pack(pop)

// SubModule base class definition
class CSubModule : public CModule
{

protected:

	U32		m_Type;				// 
	U32		m_Version;			// 
	U32		m_SysGen;

	PSUB_INI	m_pSubIniData;		// Data from Registry or INI File
	ADMIF_CFG	m_AdmIfCfg;		// ADM-interace configuration
	BASE_Info	m_baseUnit;		// Total Base unit Information

	void GetICR(PVOID pCfgMem, ULONG RealCfgSize, PICR_IdAdm pIcr, PICR_CfgAdm2If pAdmIf);
	ULONG SetAdmIfConfig(PICR_CfgAdm2If pAdmIf);

  	const BRDCHAR* getClassName(void) { return _BRDC("CSubModule"); }

public:

	CSubModule(); // constructor
	CSubModule(PBRD_InitData pBrdInitData, long sizeInitData); // one more constructor
	virtual ~CSubModule(); // destructor

	S32 Init(PUCHAR pSubmodEEPROM, BRDCHAR *pIniString);
	void BaseInfoInit(PVOID pDev, DEVS_API_entry* pEntry, BRDCHAR* pName);

//	void HardwareInit();
	void GetPuList(BRD_PuList* list) const;
	ULONG PuFileLoad(ULONG id, const BRDCHAR *fileName, PUINT pState);
	ULONG GetPuState(ULONG id, PUINT pState);

//	void GetServ(PSIDE_SRV_ListItem srv_list);
	inline PBASE_Info GetBaseInfo() {return &m_baseUnit; };
	void GetServ(PSERV_ListItem srv_list);
	LONG RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam); // CModule also
	ULONG SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext); // CModule also
};

void FindDlgDLLName(BRDCHAR **pLibName, PBRD_InitData pInitData, long initDataSize);
void SetDlgDLLName(BRDCHAR* DlgDllName, PBRD_InitData pInitData, long initDataSize);

#endif	// _SUBMODULE_H_

// ****************** End of file submodule.h **********************


 
