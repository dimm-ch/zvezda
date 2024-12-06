/*
 ****************** File devreg.h ************************
 *
 * Definitions of structures and constants
 *
 * (C) InSys by Dorokhin A. Nov 2007
 *
 ******************************************************
*/

#ifndef _DEVREG_H_
 #define _DEVREG_H_

#include	"brderr.h"
#include	"brdapi.h"

#include	"useful.h"
#include	"module.h"
#include	"adm2ifcfg.h"
#include	"icr.h"

#include	"regsrv.h"

#include	<vector>

using namespace std;

// Constants
#define	VER_MAJOR		0x00010000L
#define	VER_MINOR		0x00000000L

#pragma pack(push,1)

// Struct info about base module
typedef struct {
	S32				type;		// Base module Type
	S32				version;	// Base module Version
    BRDCHAR			boardName[BOARDNAME_SIZE];	// Device Name
	HINSTANCE		hLib;		// Dll Handle
	DEVS_API_entry	*pEntry;	// DEVS API Entry Point
	void			*pDev;		// DEV Driver Extension
} BASE_Info, *PBASE_Info;

#pragma pack(pop)

// BaseRes Class definition
class CDevReg : public CModule
{

protected:

	U32		m_DevBaseAdr;

	U32		m_Type;				//
	U32		m_Version;			//
	BASE_Info m_baseUnit;		// Total Base unit Information

	CRegSrv*	m_pRegSrv;
	void DeleteServices();

  	const BRDCHAR* getClassName(void) { return _BRDC("CDevReg"); }

public:

	CDevReg();
	CDevReg(PBRD_InitData pInitData, long sizeInitData);
	~CDevReg();

	S32 Init(PUCHAR pSubmodEEPROM, BRDCHAR *pIniString = NULL);

	void HardwareInit();

	void BaseInfoInit(PVOID pDev, DEVS_API_entry* pEntry, BRDCHAR* pName);
	void GetDeviceInfo(BRD_Info* info);

	void GetPuList(BRD_PuList* list) const;
	ULONG PuFileLoad(ULONG id, const BRDCHAR *fileName, PUINT pState);
	ULONG GetPuState(ULONG id, PUINT pState);

	ULONG SetServices();
	void GetServ(PSERV_ListItem srv_list);
	void SetSrvList(PSERV_ListItem srv_list);
	LONG RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam); // CModule also
	ULONG SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext); // CModule also
};

#endif	// _DEVREG_H_

// ****************** End of file devreg.h **********************


