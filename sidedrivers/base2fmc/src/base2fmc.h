/*
 ****************** File base2fmc.h ************************
 *
 * Definitions of structures and constants
 *
 * (C) InSys by Dorokhin A. Mar. 2012
 *
 ******************************************************
*/

#ifndef _BASE2FMC_H_
 #define _BASE2FMC_H_

#include	"brderr.h"
#include	"brdapi.h"

#include	"useful.h"
#include	"module.h"
#include	"adm2ifcfg.h"
#include	"icr.h"
#include	"Icrfmc105p.h"
#include	"Icrfmc112cp.h"
#include	"Icrfmc119e.h"

const USHORT FMC132P_CFG_TAG = 0x5523; // тэг для структуры конфигурационных параметров базового модуля FMC132P
const USHORT FMC131VZQ_CFG_TAG = 0x5528; // тэг для структуры конфигурационных параметров базового модуля FMC131V Zync
const USHORT FMC139P_CFG_TAG = 0x552A; // тэг для структуры конфигурационных параметров базового модуля FMC139P
const USHORT FMC141VZQ_CFG_TAG = 0x552B; // тэг для структуры конфигурационных параметров базового модуля FMC141V Zync
const USHORT FMC143VZQ_CFG_TAG = 0x552D; // тэг для структуры конфигурационных параметров базового модуля FMC143V Zync

#include	"basefsrv.h"
#include	"ddssrv.h"
#include	"piosrv.h"

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

typedef struct _BASE2FMC_ICR {
	ICR_CfgAdm2If	AdmIf;
	ICR_CfgAdmPld	AdmPld[MAX_PLD];
	ICR_CfgFmc105p	Ambfmc;
} BASE2FMC_ICR, *PBASE2FMC_ICR;

typedef struct _BASEF_INI {
	U32		SwitchType;			// тип коммутатора (0-non, 1-type1(FMC105P), 2-type2(FMC106P))
	U32		AdrSwitch;			// адресный код коммутатора: 0x48 по умолчанию
	U32		Gen0Type;			// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si571)
	U32		RefGen0;			// частота генератора 0, если он непрограммируемый, или заводская частота (default 50 MHz)
	U32		RefMaxGen0;			// максимальная частота внутр. генератора (Гц)
	U32		AdrGen0;			// адресный код внутр. генератора: 0x49 по умолчанию
	U32		SysRefGen;			// frequency of system generator (частота системного генератора (Гц))
	U32		ExtClk;				// external frequency of clock (внешняя частота тактирования (Гц))
	U32		DdsType;			// type of DDS (0-non, 1-PLL, 2-DDS)
	U32		AdmPldCnt;			// PLD counter (количество ПЛИС)
	U32		PioType;			// type of PIO (0-non, 1-TTL, 2-LVDS)
} BASEF_INI, *PBASEF_INI;

typedef struct _BASEF_CFG {
	U08		SwitchType;		// тип коммутатора (0-non, 1-type1(FMC105P), 2-type2(FMC106P))
	U08		AdrSwitch;		// адресный код коммутатора: 0x48 по умолчанию
	U08		Gen0Type;		// тип внутреннего генератора 0 (0-непрограммируемый, 1-Si571)
	U32		RefGen0;		// частота генератора 0, если он непрограммируемый, или заводская частота (default 50 MHz)
	U32		RefMaxGen0;		// максимальная частота внутр. генератора (Гц)
	U08		AdrGen0;		// адресный код внутр. генератора: 0x49 по умолчанию
	U32		SysRefGen;		// frequency of system generator (частота системного генератора (Гц))
	U32		ExtClk;			// external frequency of clock (внешняя частота тактирования (Гц))
	U08		DdsType;		// type of DDS (0-non, 1-PLL, 2-DDS)
	U08		AdmPldCnt;		// PLD counter (количество ПЛИС)
	U08		PioType;		// type of PIO (0-non, 1-TTL, 2-LVDS)
	U32		RefGen5;		// частота генератора 5 (default 156.25 MHz) или такт для DDS на модуле FMC132P
} BASEF_CFG, *PBASEF_CFG;

const BASEF_CFG BaseFmcCfg_dflt = {
	1,			// switch type
	0x48,		// switch address
	0,			// generator 0 type
	19200000,	// generator 0 frequency
	280000000,	// generator 0 max frequency
	0x49,		// generator 0 address
	250000000,	// system clock
	30000000,	// external clock
	1,			// DDS type
	1,			// PLD counter
	1,			// PIO type
};

#pragma pack(pop)

// CBase2fmc Class definition
class CBase2fmc : public CModule
{

protected:

	PBASEF_INI	m_pIniData;		// Data from Registry or INI File

	U32		m_Type;				// 
	U32		m_Version;			// 
	BASEF_CFG m_FmcIfCfg;		// 
	BASE_Info m_baseUnit;		// Total Base unit Information

	CBasefSrv*	 m_pBasefSrv;
	CDdsSrv*	m_pDdsSrv;
	CPioSrv*	 m_pPioSrv;
	void DeleteServices();

  	const BRDCHAR* getClassName(void) { return  _BRDC("CBase2fmc"); }

	void GetICR(PVOID pCfgMem, ULONG RealCfgSize, PBASE2FMC_ICR pBaseResICR);
	ULONG SetConfig(PBASEF_INI pIniData, PBASE2FMC_ICR pBaseResICR);

public:

	CBase2fmc();
	CBase2fmc(PBRD_InitData pInitData, long sizeInitData);
	~CBase2fmc();

	S32 Init(PUCHAR pSubmodEEPROM, BRDCHAR* pIniString = NULL);

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

#endif	// _BASE2FMC_H_

// ****************** End of file base2fmc.h **********************


 