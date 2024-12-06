/*
 ***************** File basemodule.cpp ***********************
 *
 * BRD Driver for ...
 *
 * (C) InSys by Dorokhin A. Jan 2004
 *
 ******************************************************
*/

#define	DEVS_API_EXPORTS

#ifdef _WIN32
// #pragma warning(disable: 4786)
#endif
#include "basemodule.h"

IPC_handle _BRD_openLibraryEx(const IPC_str *baseName, unsigned param)
{
    IPC_handle hLib = 0;
#ifdef	__IPC_WIN__
	hLib = IPC_openLibraryEx(baseName, param);
#endif
#ifdef	__IPC_LINUX__
    char *libAbsNames[3] = {0,0,0};
    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        libAbsNames[i] = (char*)malloc(PATH_MAX);
    }

    // prepare first case if library placed in current directory
    if(libAbsNames[0]) {
        const char *currdir = getcwd(libAbsNames[0], PATH_MAX);
        if(currdir) {
            unsigned dirlen = strlen(libAbsNames[0]);
            snprintf(libAbsNames[0]+dirlen, PATH_MAX-dirlen, "%s%s%s", "/lib", baseName, ".so");
        }
    }

    // prepare second case if library placed in pointed BARDYLIB environment variable
    if(libAbsNames[1]) {
        const char *envpath = getenv("BARDYLIB");
        if(envpath) {
            snprintf(libAbsNames[1], PATH_MAX, "%s%s%s%s", envpath, "/lib", baseName, ".so");
        } else {
            free(libAbsNames[1]);
            libAbsNames[1] = 0;
        }
    }

    // prepare third absname in predefined directory "/usr/local/lib/bardy/lib"
    if(libAbsNames[2]) {
        snprintf(libAbsNames[2], PATH_MAX, "%s%s%s", "/usr/local/lib/bardy/lib", baseName, ".so");
    }

    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        //fprintf(stderr, "TRY LIBRARY: ---- %s\n", libAbsNames[i]);
        if(libAbsNames[i]) {
            hLib = IPC_openLibrary(libAbsNames[i], param);
            if(!hLib) {
                continue;
            } else {
                break;
            }
        }
    }

    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        if(libAbsNames[i]) {
            free(libAbsNames[i]);
        }
    }
#endif
    return hLib;
}

#define	CURRFILE _BRDC("BASEMODULE")

//***************************************************************************************
CBaseModule::CBaseModule(const U16 sizeICR)
{
	m_sizeICR = sizeICR;
	m_sidedllCnt = 0;
	m_AdmIfCnt = 1;
	m_curHandle = 0;
	m_isFMC2 = 0;

	BRDC_strcpy(subsectionName, _BRDC("SUBUNIT"));

	for(int idxSub = 0; idxSub < MAX_ADMIF; idxSub++)
	{
		m_Subunit[idxSub].pSubEEPROM = NULL;
		m_Subunit[idxSub].hFileMap = NULL;
	}
}

//***************************************************************************************
CBaseModule::~CBaseModule()
{
	int num = (int)m_ServInfoList.size();
	int i = 0;
	for(i = 0; i < num; i++)
	{
		if(m_ServInfoList[i]->pSrvOvUn)
		{
			if(m_ServInfoList[i]->isCaptured)
			{
				int iOver = m_ServInfoList[i]->pSrvOvUn->idxOverSrv;
				m_ServInfoList[i]->pSrvOvUn->idxOverSrv = -1;

				for(int is = 0; is < MAX_UNDERSRV; is++)
					if(m_ServInfoList[iOver]->pSrvOvUn->idxUnderSrv[is] == i)
					{
						m_ServInfoList[iOver]->pSrvOvUn->idxUnderSrv[is] = -1;
						break;
					}
			}
		}
	}
	for(i = 0; i < num; i++)
	{
		if(m_ServInfoList[i]->pSrvOvUn)
		{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			IPC_unmapSharedMemory(m_ServInfoList[i]->hFileMap);
#else
			UnmapViewOfFile(m_ServInfoList[i]->pSrvOvUn);
#endif
		}
		if(m_ServInfoList[i]->hFileMap)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			IPC_deleteSharedMemory(m_ServInfoList[i]->hFileMap);
#else
			CloseHandle(m_ServInfoList[i]->hFileMap);
#endif
	}
	for(i = 0; i < num; i++)
		delete m_ServInfoList[i];
		//m_ServInfoList.pop_back();
	for(int idxSub = 0; idxSub < m_AdmIfCnt; idxSub++)
	{
		if(m_Subunit[idxSub].pSubEEPROM)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			IPC_unmapSharedMemory(m_Subunit[idxSub].hFileMap);
#else
			UnmapViewOfFile(m_Subunit[idxSub].pSubEEPROM);
#endif
		if(m_Subunit[idxSub].hFileMap)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			IPC_deleteSharedMemory(m_Subunit[idxSub].hFileMap);
#else
			CloseHandle(m_Subunit[idxSub].hFileMap);
#endif
	}
	for(int iSub = 0; iSub < m_sidedllCnt; iSub++)
		(m_sidedll[iSub].pEntry)(m_sidedll[iSub].pSideDrv, NULL, SIDEcmd_CLEANUP, NULL);
}

//***************************************************************************************
ULONG CBaseModule::GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize)
{
	return BRDerr_OK;
}

typedef struct _LIBS_NAME {
	BRDCHAR *pBaseLibName;
	BRDCHAR *pBaseLibNameMore;
	BRDCHAR *pLibName;
} LIBS_NAME, *PLIBS_NAME;

//********************** FindSubDLLName *****************
// PLIBS_NAME pLibsName - названия библиотек (OUT)
// int& iBegin - индекс в массиве pInitData, с которого начинается субмодульный блок данных (OUT)
// PBRD_InitData pInitData - указатель на массив данных инициализации вида "ключ-значение" (IN)
// S32& initDataSize - число оставшихся элементов массива pInitData (IN/OUT)
//*******************************************************
static void FindSubDLLName(PLIBS_NAME pLibsName, int& iBegin, PBRD_InitData pInitData, S32& initDataSize, BRDCHAR* subName, int& flag)
{
	pLibsName->pBaseLibName = NULL;
	pLibsName->pBaseLibNameMore = NULL;
	pLibsName->pLibName = NULL;
	// Search SubSections
	for(int i = 0; i < initDataSize; i++)
	{
		if(!BRDC_stricmp(pInitData[i].key, _BRDC("#begin")))	// Found keyword "#begin"
		{	// Begin of SubSections
			int		beginCnt = 1;
			int		isSUBUNIT = 0;

			iBegin = i+1;								// Position of keyword "#begin"
			
			// Check SubSection Name
			//if(!_tcsnicmp(pInitData[i].val, "SUBUNIT", 7))     
			if(!BRDC_strnicmp(pInitData[i].val, subName, 7))     
				isSUBUNIT = 1;
			if(!BRDC_strnicmp(pInitData[i].val, _BRDC("SUBUNIT#2"), 9))     
				flag = 1;

			// Go through #begin/#end block and Search "type" keywords 
			while(++i < initDataSize)
			{
				if(!BRDC_stricmp(pInitData[i].key, _BRDC("#begin")))		// Begin of nested SubSection
					beginCnt++;
				if(!BRDC_stricmp(pInitData[i].key, _BRDC("#end")))			// End of this or nested SubSection
					if(--beginCnt == 0)
						break;
				if(isSUBUNIT && (beginCnt == 1))							// The SUBUNIT SubSection & The Main #begin/#end block
				{
					if(!BRDC_stricmp(pInitData[i].key, _BRDC("btype0")))	// Keyword "btype0"
						if(!pLibsName->pBaseLibName) 							// The First "btype0" keyword
							pLibsName->pBaseLibName = pInitData[i].val;
					if(!BRDC_stricmp(pInitData[i].key, _BRDC("btype1")))	// Keyword "btype1"
						if(!pLibsName->pBaseLibNameMore) 						// The First "btype1" keyword
							pLibsName->pBaseLibNameMore = pInitData[i].val;
					if(!BRDC_stricmp(pInitData[i].key, _BRDC("type")))		// Keyword "type"
						if(!pLibsName->pLibName) 								// The First "type" keyword
							pLibsName->pLibName = pInitData[i].val;
				}
			}
		}
		if(pLibsName->pLibName || pLibsName->pBaseLibName || pLibsName->pBaseLibNameMore)
		{
			initDataSize = i - iBegin;
//			initDataSize = i;
			break;
		}
	}
}

//*************** InitDataSubDll ************************
//*******************************************************
void CBaseModule::InitDataSubDll(PBRD_InitData pInitData, S32 initDataSize)
{
	LIBS_NAME LibsName = { NULL, NULL, NULL};
	int	idxOffset = 0;
	for(int idxSubDriver = 0; idxSubDriver < MAX_ADMIF; idxSubDriver++)
	{
		int	iiBegin = 0;
		S32	errorCode = 0;
		S32 subInitDataSize = initDataSize - idxOffset;
		m_isFMC2 = 0;
		FindSubDLLName(&LibsName, iiBegin, pInitData + idxOffset, subInitDataSize, subsectionName, m_isFMC2);
//		if(LibsName.pLibName == NULL)
//			return;
		iiBegin += idxOffset;
		// DLL Library Name was founded
		if(LibsName.pBaseLibName != NULL)
		{
			errorCode = BaseDllInit(LibsName.pBaseLibName, idxSubDriver, pInitData + iiBegin, subInitDataSize, 0);
			if(errorCode != BRDerr_OK)
				break;
		}
		if(LibsName.pBaseLibNameMore != NULL)
		{
			errorCode = BaseDllInit(LibsName.pBaseLibNameMore, idxSubDriver, pInitData + iiBegin, subInitDataSize, 0);
			if(errorCode != BRDerr_OK)
				break;
		}
		if(LibsName.pLibName != NULL)
		{
			errorCode = SubunitInit(LibsName.pLibName, idxSubDriver, pInitData + iiBegin, subInitDataSize, 0);
			if(errorCode != BRDerr_OK)
				break;
		}
		idxOffset += (iiBegin + subInitDataSize + 1);
	}
}

BRDCHAR Base2AdmDllName[] = _BRDC("base2adm.dll");
BRDCHAR BaseresDllName[] = _BRDC("baseres.dll");
BRDCHAR Base2fmcDllName[] = _BRDC("base2fmc.dll");

//************** InitAutoSubDll ***********************
//*****************************************************
//void CBaseModule::InitAutoSubDll(PTSTR pInitStr, long InitStrSize, PBRD_InitEnum pSubEnum, ULONG sizeSubEnum)
void CBaseModule::InitAutoSubDll(BRDCHAR *pInitStr, long InitStrSize, PBRD_SubEnum pSubEnum, ULONG sizeSubEnum)
{
	ULONG SubunitType;		// SUBUNIT Type Code from the EEPROM
	BRDCHAR *pLibName;			// SUB Driver DLL Name
	for(int idxAdmIf = 0; idxAdmIf < m_AdmIfCnt; idxAdmIf++)
	{
		int	FillInitStrSize = (int)BRDC_strlen(pInitStr);
		BRDCHAR *pBaseLibName = NULL;
		S32	errorCode = BaseDllInit(pBaseLibName, idxAdmIf, pInitStr + FillInitStrSize, InitStrSize - FillInitStrSize, 1);
		//if(errorCode != BRDerr_OK)
		//	ErrorPrint( DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
		//				CURRFILE, 
		//				_BRDC("<InitAutoSubDll> DLL Library for register service was NOT founded"));
//			break;
		if(m_DeviceID == AMBPCD_DEVID || 
			m_DeviceID == AMBPCX_DEVID)
				pBaseLibName = Base2AdmDllName;
		if(m_DeviceID == AMBPEX1_DEVID || 
			m_DeviceID == AMBPEX5_DEVID || 
			m_DeviceID == AMBPEX8_DEVID || 
			m_DeviceID == ADP201X1AMB_DEVID ||
			m_DeviceID == ADP201X1DSP_DEVID)
				pBaseLibName = BaseresDllName;
		if( m_DeviceID == FMC105P_DEVID || 
			m_DeviceID == FMC106P_DEVID || 
			m_DeviceID == FMC107P_DEVID || 
			m_DeviceID == FMC103E_DEVID ||
			m_DeviceID == FMC110P_DEVID ||
			m_DeviceID == FMC114V_DEVID ||
			m_DeviceID == FMC113E_DEVID ||
			m_DeviceID == FMC108V_DEVID ||
			m_DeviceID == FMC115CP_DEVID ||
			m_DeviceID == FMC117CP_DEVID ||
			m_DeviceID == FMC112CP_DEVID ||
			m_DeviceID == FMC121CP_DEVID
			)
				pBaseLibName = Base2fmcDllName;
		errorCode = BaseDllInit(pBaseLibName, idxAdmIf, pInitStr + FillInitStrSize, InitStrSize - FillInitStrSize, 1);
		//if(errorCode != BRDerr_OK)
		//	ErrorPrint( DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
		//				CURRFILE, 
		//				_BRDC("<InitAutoSubDll> DLL Library for base services was NOT founded"));

		errorCode = GetSubunitTypeAuto(idxAdmIf, SubunitType);
		if(errorCode != BRDerr_OK)
			break;
		// Try to Link SUB Driver that Associated with this SubUnit Type Code
		pLibName = NULL;
		for(int i = 0; i < (int)sizeSubEnum; i++)
		{
			if(SubunitType == pSubEnum[i].val)
			{
				pLibName = pSubEnum[i].name;
				break;
			}
		}
		if(pLibName == NULL)
		{
			//ErrorPrint( DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA),
			//			CURRFILE, 
			//			_BRDC("<InitAutoSubDll> DLL Library Name for Submodule was NOT founded"));
			return; // для данного типа субмодуля имя библиотеки в перечне не найдено !!!
		}
		// DLL Library Name was founded
		FillInitStrSize = (int)BRDC_strlen(pInitStr);
		errorCode = SubunitInit(pLibName, idxAdmIf, pInitStr + FillInitStrSize, InitStrSize - FillInitStrSize, 1);
		if(errorCode != BRDerr_OK)
			break;
	}

}

//***************************************************************************************
void CBaseModule::Open()
{
}

//***************************************************************************************
void CBaseModule::Close()
{
}

//***************************************************************************************
ULONG CBaseModule::SetSubEeprom(int idxSub, ULONG& SubCfgSize)
{
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBaseModule::GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize)
{
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBaseModule::GetPower(PBRDextn_FMCPOWER pow)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	pow->value = 0;
	pow->slot = 0;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::PowerOnOff(PBRDextn_FMCPOWER pow, int force)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	pow->value = 0;
	pow->slot = 0;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::ReadFmcExt(PBRDextn_FMCEXT ext)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::WriteFmcExt(PBRDextn_FMCEXT ext)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::GetSensors(void* pSensors)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::PuVerify(PBRDextn_PuVerify arg)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::GetIrqMinTime(ULONG* pTimeVal)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;

	return Status;
}

//***************************************************************************************
ULONG CBaseModule::SetIrqMinTime(ULONG* pTimeVal)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;

	return Status;
}

//***************************************************************************************
ULONG CBaseModule::WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_INVALID_FUNCTION;

	return Status;
}

//***************************************************************************************
ULONG CBaseModule::ReadSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;
	if(!submod)
		Status = ReadAdmIdROM(pBuffer, BufferSize, Offset);
	else
		Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::WriteSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	ULONG Status = BRDerr_OK;
	if(!submod)
		Status = WriteAdmIdROM(pBuffer, BufferSize, Offset);
	else
		Status = BRDerr_INVALID_FUNCTION;
	return Status;
}

//***************************************************************************************
ULONG CBaseModule::ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	return BRDerr_OK;
}
ULONG CBaseModule::WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset)
{
	return BRDerr_OK;
}

ULONG CBaseModule::RegisterCallBack(PVOID funcName)
{
	return BRDerr_FUNC_UNIMPLEMENTED;
}

ULONG CBaseModule::PldReadToFile(U32 puId, const BRDCHAR *fileName)
{
	return BRDerr_FUNC_UNIMPLEMENTED;
}

BRDCHAR RegDllName[] = _BRDC("devreg.dll");

//***************************************************************************************
long CBaseModule::BaseDllInit(BRDCHAR *pLibName, int idxSub, void* pSubInitData, long subInitDataSize, int Auto)
{
	S32					type;				// SIDE Driver Type ID
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle			hLib;				// SIDE Driver DLL Handle
#else
	HINSTANCE			hLib;				// SIDE Driver DLL Handle
#endif
	SIDE_API_initData	*pSIDE_initData;	// SIDE Driver Entry Point
	SIDE_API_initAuto	*pSIDE_initAuto;	// SIDE Driver Entry Point
	SIDE_API_entry		*pSIDE_Entry;		// SIDE Driver Entry Point
	void				*pSideDrv;			// SIDE Driver Extension

	if(!pLibName)
		pLibName = RegDllName;
	// Open Library
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	hLib = _BRD_openLibraryEx(pLibName, 0);
    if( !hLib )
#else
  #ifdef _WIN64
	BRDCHAR libname[MAX_PATH];
	BRDCHAR libname0[MAX_PATH];
	BRDC_strcpy(libname, pLibName);
	BRDCHAR* pchar = BRDC_strstr(libname, _BRDC(".dll"));
	if(pchar != NULL)
		*pchar = 0;
	size_t len_name =  BRDC_strlen(libname);
	BRDC_strcpy(libname0, libname);
	if(libname[len_name-2] != '6' && libname[len_name-1] != '4')
	{
		BRDC_strcat(libname, _BRDC("64"));
		BRDC_strcat(libname0, _BRDC("-64"));
	}
	hLib = LoadLibrary( libname );
	if( hLib <= (HINSTANCE)HINSTANCE_ERROR )
	{
		hLib = LoadLibrary( libname0 );
	}
  #else
	hLib = LoadLibrary( pLibName );
  #endif
	if(hLib <= (HINSTANCE)HINSTANCE_ERROR)
#endif
	{
		ErrorPrintf(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA), CURRFILE,
						  _BRDC("<Init> Error load library %s.dll !!!"), pLibName);
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	}
	// Get Entry Point
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	pSIDE_initData = (SIDE_API_initData*)IPC_getEntry(hLib, "SIDE_initData");
	pSIDE_initAuto = (SIDE_API_initAuto*)IPC_getEntry(hLib, "SIDE_initAuto");
	pSIDE_Entry	  = (SIDE_API_entry*)IPC_getEntry(hLib, "SIDE_entry");
#else
	pSIDE_initData = (SIDE_API_initData*)GetProcAddress( hLib, "SIDE_initData" );
	pSIDE_initAuto = (SIDE_API_initAuto*)GetProcAddress(hLib, "SIDE_initAuto" );
	pSIDE_Entry	  = (SIDE_API_entry*)GetProcAddress( hLib, "SIDE_entry" );
#endif
	if(!pSIDE_initData || !pSIDE_initAuto || !pSIDE_Entry )
	{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_closeLibrary(hLib);
#else
		FreeLibrary(hLib);
#endif
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	}

	PUCHAR pBaseCfgMem = new UCHAR[m_sizeICR];
	ULONG BaseCfgSize = m_sizeICR;
	if(GetBaseEEPROM(pBaseCfgMem, BaseCfgSize) != BRDerr_OK)
	{
		delete[] pBaseCfgMem;
		pBaseCfgMem = NULL;
	}

	// Parse the rest of the record
	if(Auto)
		type = (pSIDE_initAuto)( &pSideDrv, (void*)pBaseCfgMem, (BRDCHAR *)pSubInitData, subInitDataSize);
//		type = (pSIDE_initAuto)( &pSideDrv, pBaseCfgMem, NULL, (PTSTR)pSubInitData, subInitDataSize);
//		type = (pSIDE_initAuto)( &pSideDrv, pBaseCfgMem, &m_AdmIfCfg[idxSub], (PTSTR)pSubInitData, subInitDataSize);
	else
	{
		type = (pSIDE_initData)( &pSideDrv, (void*)pBaseCfgMem, (PBRD_InitData)pSubInitData, subInitDataSize);
//		type = (pSIDE_initData)( &pSideDrv, pBaseCfgMem, NULL, (PBRD_InitData)pSubInitData, subInitDataSize);
		m_AdmIfCnt++;
	}
	delete[] pBaseCfgMem;
	if(type < 0)
	{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_closeLibrary(hLib);
#else
		FreeLibrary(hLib);
#endif
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	}

	// Save SIDE Driver Information into DEV Extension
	m_sidedll[m_sidedllCnt].type		= type;
	m_sidedll[m_sidedllCnt].hLib		= hLib;
	m_sidedll[m_sidedllCnt].pEntry		= pSIDE_Entry;
	m_sidedll[m_sidedllCnt].pSideDrv	= pSideDrv;
	m_sidedllCnt++;

	SIDE_CMD_Init sub_init;
	sub_init.pDev = this;
	sub_init.pEntry = DEVS_entry;
	BRDC_strcpy(sub_init.boardName, m_name);
	(pSIDE_Entry)(pSideDrv, NULL, SIDEcmd_INIT, &sub_init);

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBaseModule::SetSubunitAuto()
{
	m_SubunitCnt = 0;
	for(int i = 0; i < m_AdmIfCnt; i++)
	{	
		PICR_IdAdm pAdmId = new ICR_IdAdm;
		ULONG SubCfgSize = sizeof(ICR_IdAdm);
		ULONG status = GetSubEEPROM(i, (PUCHAR)pAdmId, SubCfgSize);
		if(status != BRDerr_OK)
			return status;
		if(pAdmId->wTag == ADM_ID_TAG)
		{
			m_Subunit[m_SubunitCnt].type = pAdmId->wType;
			m_SubunitCnt++;
		}
		else
			return BRDerr_BAD_DEVICE_VITAL_DATA;
		delete pAdmId;
	}
	return BRDerr_OK;
}

//***************************************************************************************
S32 CBaseModule::GetSubunitTypeAuto(int idxSubunit, ULONG& type)
{
	if(idxSubunit >= m_SubunitCnt)
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	type = m_Subunit[idxSubunit].type;
	return BRDerr_OK;
}

//***************************************************************************************
S32 CBaseModule::SubunitInit(BRDCHAR *pLibName, int idxSub, void* pSubInitData, S32 subInitDataSize, int Auto)
{
	S32					type;				// SIDE Driver Type ID
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle			hLib;				// SIDE Driver DLL Handle
#else
	HINSTANCE			hLib;				// SIDE Driver DLL Handle
#endif
	SIDE_API_initData	*pSIDE_initData;	// SIDE Driver Entry Point
	SIDE_API_initAuto	*pSIDE_initAuto;	// SIDE Driver Entry Point
	SIDE_API_entry		*pSIDE_Entry;		// SIDE Driver Entry Point
	void				*pSideDrv;			// SIDE Driver Extension

	// Open Library
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	hLib = _BRD_openLibraryEx(pLibName, 0);
    if( !hLib )
#else
  #ifdef _WIN64
	BRDCHAR libname[MAX_PATH];
	BRDCHAR libname0[MAX_PATH];
	BRDC_strcpy(libname, pLibName);
	BRDCHAR* pchar = BRDC_strstr(libname, _BRDC(".dll"));
	if(pchar != NULL)
		*pchar = 0;
	size_t len_name =  BRDC_strlen(libname);
	BRDC_strcpy(libname0, libname);
	if(libname[len_name-2] != '6' && libname[len_name-1] != '4')
	{
		BRDC_strcat(libname, _BRDC("64"));
		BRDC_strcat(libname0, _BRDC("-64"));
	}
	hLib = LoadLibrary( libname );
	if( hLib <= (HINSTANCE)HINSTANCE_ERROR )
	{
		hLib = LoadLibrary( libname0 );
	}
  #else
	hLib = LoadLibrary( pLibName );
  #endif
	if(hLib <= (HINSTANCE)HINSTANCE_ERROR)
#endif
	{
		ErrorPrintf(DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA), CURRFILE,
						  _BRDC("<Init> Error load library %s.dll !!!"), pLibName);
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	}

	// Get Entry Point
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	pSIDE_initData = (SIDE_API_initData*)IPC_getEntry(hLib, "SIDE_initData");
	pSIDE_initAuto = (SIDE_API_initAuto*)IPC_getEntry(hLib, "SIDE_initAuto");
	pSIDE_Entry	  = (SIDE_API_entry*)IPC_getEntry(hLib, "SIDE_entry");
#else
	pSIDE_initData = (SIDE_API_initData*)GetProcAddress( hLib, "SIDE_initData" );
	pSIDE_initAuto = (SIDE_API_initAuto*)GetProcAddress(hLib, "SIDE_initAuto");
	pSIDE_Entry	  = (SIDE_API_entry*)GetProcAddress( hLib, "SIDE_entry" );
#endif
	if(!pSIDE_initData || !pSIDE_initAuto || !pSIDE_Entry )
	{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_closeLibrary(hLib);
#else
		FreeLibrary(hLib);
#endif
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	}

	PUCHAR pCfgMem = NULL;
	if(m_sizeICR)
	{
		pCfgMem = new UCHAR[m_sizeICR + SUBMOD_CFGMEM_SIZE];
		ULONG BaseCfgSize = m_sizeICR;
                GetBaseEEPROM(pCfgMem, BaseCfgSize);
	//	if(GetBaseEEPROM(pCfgMem, BaseCfgSize) != BRDerr_OK)
	//	{
	//		delete pCfgMem;
	//		pCfgMem = NULL;
	//	}
		PUCHAR pSubCfgMem = pCfgMem + BaseCfgSize;
		ULONG SubCfgSize = SUBMOD_CFGMEM_SIZE;
		if(GetSubEEPROM(idxSub, pSubCfgMem, SubCfgSize) != BRDerr_OK)
	//	if(GetAdmIdROM(pSubunitCfgMem, SubCfgSize) != BRDerr_OK)
		{
	//		delete pSubmodCfgMem;
	//		pSubCfgMem = NULL;
			//pCfgMem = NULL; // 07.12.2007
			SubCfgSize = 0;
		}
		if(pCfgMem)
			*((PUSHORT)pCfgMem + 2) = USHORT(BaseCfgSize + SubCfgSize);

	/*	PUCHAR pSubmodCfgMem = new UCHAR[SUBMOD_CFGMEM_SIZE];
		ULONG BufferSize = SUBMOD_CFGMEM_SIZE;
		if(GetAdmIdROM(pSubmodCfgMem, BufferSize) != BRDerr_OK)
		{
			delete pSubmodCfgMem;
			pSubmodCfgMem = NULL;
		}
	*/
	}
	else
	{
		ErrorPrintf(DRVERR(BRDerr_WARN),
		//ErrorPrintf(DRVERR(BRDerr_INSUFFICIENT_RESOURCES), 
					CURRFILE,
					_BRDC("<Init> ICR of base module is empty !!!"));
		//return BRDerr_INSUFFICIENT_RESOURCES;
		//return BRDerr_OK;
	}

	// Parse the rest of the record
	if(Auto)
	{
		type = (pSIDE_initAuto)( &pSideDrv, (void*)pCfgMem, (BRDCHAR *)pSubInitData, subInitDataSize);
//		type = (pSIDE_initAuto)( &pSideDrv, pCfgMem, NULL, (PTSTR)pSubInitData, subInitDataSize);
//		type = (pSIDE_initAuto)( &pSideDrv, pCfgMem, &m_AdmIfCfg[idxSub], (PTSTR)pSubInitData, subInitDataSize);
	}
	else
	{
		type = (pSIDE_initData)( &pSideDrv, (void*)pCfgMem, (PBRD_InitData)pSubInitData, subInitDataSize);
//		type = (pSIDE_initData)( &pSideDrv, pCfgMem, NULL, (PBRD_InitData)pSubInitData, subInitDataSize);
//		type = (pSIDE_initData)( &pSideDrv, pCfgMem, &m_AdmIfCfg[idxSub], (PBRD_InitData)pSubInitData, subInitDataSize);
		m_SubunitCnt++;
	}
	if(pCfgMem)
		delete pCfgMem;
//	delete pSubmodCfgMem;
	if(type < 0)
	{
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_closeLibrary(hLib);
#else
		FreeLibrary(hLib);
#endif
		return BRDerr_BAD_DEVICE_VITAL_DATA;
	}

	// Save SIDE Driver Information into DEV Extension
	m_Subunit[idxSub].type		= type;
//	m_Subunit[idxSub].hLib		= hLib;
//	m_Subunit[idxSub].pEntry	= pSIDE_Entry;
//	m_Subunit[idxSub].pSideDrv	= pSideDrv;
	m_sidedll[m_sidedllCnt].type		= type;
	m_sidedll[m_sidedllCnt].hLib		= hLib;
	m_sidedll[m_sidedllCnt].pEntry		= pSIDE_Entry;
	m_sidedll[m_sidedllCnt].pSideDrv	= pSideDrv;
	m_sidedllCnt++;

	SIDE_CMD_Init sub_init;
	sub_init.pDev = this;
	sub_init.pEntry = DEVS_entry;
	BRDC_strcpy(sub_init.boardName, m_name);
	(pSIDE_Entry)(pSideDrv, NULL, SIDEcmd_INIT, &sub_init);

	return BRDerr_OK;
}

//***************************************************************************************
ULONG CBaseModule::SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext)
{
	int idx	 = (handle >> 16) & 0x3F;
	ULONG status;
	UNDERSERV_Cmd new_cmd = {0, NULL};
	int base_srv_num = (int)m_SrvList.size();
	if(idx < base_srv_num)
	{
		status = CModule::SrvCtrl(handle, cmd, arg, &new_cmd);
		if(new_cmd.code)
		{
			cmd = new_cmd.code;
			arg = new_cmd.pParams;
		}
	}
	else
	{
		DEV_CMD_Ctrl param;
		param.handle = handle;
		param.cmd = cmd;
		param.arg = arg;

		if(m_ServInfoList[idx]->baseORsideDLL)
		{
			SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idx]->pSideEntry;
			status = pSIDE_Entry(m_ServInfoList[idx]->pSideDll, NULL, SIDEcmd_CTRL, &param);
		}
		else
		{
			DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idx]->pSideEntry;
			status = pDEV_Entry(m_ServInfoList[idx]->pSideDll, DEVcmd_CTRL, &param);
		}
		cmd = param.cmd;
		arg = param.arg;
	}
	if(status == BRDerr_CMD_UNSUPPORTED)
	{
		if(cmd == SERVcmd_SYS_RELEASE)
		{
			int i = 0;
//			while(m_SrvList[idx]->GetUnderIndex(i) != -1)
			//while(m_ServInfoList[idx]->pSrvOvUn->idxUnderSrv[i] != -1)
			for(i = 0; i < MAX_UNDERSRV; i++)
				if(m_ServInfoList[idx]->pSrvOvUn->idxUnderSrv[i] != -1)
				{ // идем по подслужбам (уже захваченным)
					//long ius = m_ServInfoList[idx]->pSrvOvUn->idxUnderSrv[i++];
					long ius = m_ServInfoList[idx]->pSrvOvUn->idxUnderSrv[i];
					status = CmdQuery(handle, ius, cmd);
					if(status == BRDerr_OK)
					{
						UnderSrvRelease(handle, idx, ius);
	//					break;
					}
				}
		}
		else
		{
			m_curHandle = handle;
			status = UnderSrvCtrl(handle, cmd, arg, NULL);
			if(status == BRDerr_CMD_UNSUPPORTED)
				status = CandidSrvCtrl(handle, cmd, arg, NULL);
		}
	}
	return status;
}

//***************************************************************************************
ULONG CBaseModule::GetBaseSrvCnt() const
{
	int srv_num = 0;
	return srv_num;
}

//***************************************************************************************
ULONG CBaseModule::GetSrvCnt() const
{
	int srv_num = (int)m_SrvList.size();
	srv_num += GetBaseSrvCnt();
//	for(int iSub = 0; iSub < m_SubmodCnt; iSub++)
	for(int iSub = 0; iSub < m_sidedllCnt; iSub++)
	{
		DEV_CMD_GetServList sub_srv_list;
		sub_srv_list.item = 0;
		sub_srv_list.pSLI = NULL;
//		(m_subunit[iSub].pEntry)(m_subunit[iSub].pSideDrv, NULL, SIDEcmd_GETSERVLIST, &sub_srv_list);
		(m_sidedll[iSub].pEntry)(m_sidedll[iSub].pSideDrv, NULL, SIDEcmd_GETSERVLIST, &sub_srv_list);
		srv_num += sub_srv_list.itemReal;
	}
	return srv_num;
}

//***************************************************************************************
void CBaseModule::GetBaseSrvList(PSERV_ListItem srv_list, int& idx_srv_list, set <string> *nameSet)
{
}

//***************************************************************************************
void CBaseModule::GetBaseSrvList(PSERV_ListItem srv_list, int& idx_srv_list, set <wstring> *nameSet)
{
}

//***************************************************************************************
void CBaseModule::GetSrvList(PSERV_ListItem srv_list)
{
	//int srv_num = (int)m_ServInfoList.size();
	//if(srv_num)
	//{
	//	for(int i = 0; i < srv_num; i++)
	//	{
	//		lstrcpy(srv_list->name, m_ServInfoList[i]->name);
	//		srv_list->attr = m_ServInfoList[i]->attribute;
	//		srv_list->idxMain = m_ServInfoList[i]->index;
	//	}
	//	return;
	//}

	CModule::GetSrvList(srv_list);
	int base_srv_num = (int)m_SrvList.size();
	int idx_srv_list = base_srv_num;

#ifdef _UNICODE
	set <wstring> nameSet; // множество неповторяющихся строк(имен служб)
	wstring str_name; // строка, заносимая в множество
#else
	set <string> nameSet; // множество неповторяющихся строк(имен служб)
	string str_name; // строка, заносимая в множество
#endif
	BRDCHAR num_name[SERVNAME_SIZE]; // нумерованное имя службы

	for(int i = 0; i < base_srv_num; i++)
	{
		AddServInfoList(&srv_list[i], 0, NULL, NULL);
		BRDC_strcpy(num_name, m_SrvList[i]->GetName());
		str_name = num_name;
		// nameSet.insert(str_name).second;
		nameSet.insert(str_name);
	}

	GetBaseSrvList(srv_list, idx_srv_list, &nameSet);

//	for(int iSub = 0; iSub < m_SubmodCnt; iSub++)
	for(int iSub = 0; iSub < m_sidedllCnt; iSub++)
	{
//		DEV_CMD_GetServList sub_srv_list;
		SIDE_CMD_GetServList sub_srv_list;
		sub_srv_list.item = 0;
		sub_srv_list.pSLI = NULL;
//		(m_subunit[iSub].pEntry)(m_subunit[iSub].pSideDrv, NULL, SIDEcmd_GETSERVLIST, &sub_srv_list);
		(m_sidedll[iSub].pEntry)(m_sidedll[iSub].pSideDrv, NULL, SIDEcmd_GETSERVLIST, &sub_srv_list);
//		sub_srv_list.pList = new SIDE_SRV_ListItem[sub_srv_list.itemReal];
		sub_srv_list.pSLI = new SERV_ListItem[sub_srv_list.itemReal];
		sub_srv_list.item = sub_srv_list.itemReal;
//		(m_subunit[iSub].pEntry)(m_subunit[iSub].pSideDrv, NULL, SIDEcmd_GETSERVLIST, &sub_srv_list);
		(m_sidedll[iSub].pEntry)(m_sidedll[iSub].pSideDrv, NULL, SIDEcmd_GETSERVLIST, &sub_srv_list);
		for(UINT i = 0; i < sub_srv_list.itemReal; i++)
		{
			int numeric = 0;
			do {
				BRDC_sprintf(num_name, _BRDC("%s%d"), sub_srv_list.pSLI[i].name, numeric);
				BRDC_snprintf(num_name, sizeof(num_name), _BRDC("%s%d"), sub_srv_list.pSLI[i].name, numeric);
				str_name = num_name;
				numeric++;
			} while(!nameSet.insert(str_name).second);
//			_tcscpy(srv_list[base_srv_num+i].name, sub_srv_list.pList[i].name);
//			LPCTSTR pStr = str.c_str();
			BRDC_strcpy(srv_list[idx_srv_list].name, num_name);
			BRDC_strcpy(sub_srv_list.pSLI[i].name, num_name);
			srv_list[idx_srv_list].attr = sub_srv_list.pSLI[i].attr;
			srv_list[idx_srv_list].idxMain = sub_srv_list.pSLI[i].idxMain = idx_srv_list;
			AddServInfoList(&srv_list[idx_srv_list], 1, m_sidedll[iSub].pSideDrv, (void*)m_sidedll[iSub].pEntry);
			idx_srv_list++;
//			CService* pServ = (CService*)sub_srv_list.pList[i].srv;
//			m_SrvList.push_back(pServ);
		}
//		(m_subunit[iSub].pEntry)(m_subunit[iSub].pSideDrv, NULL, SIDEcmd_SETSERVLIST, &sub_srv_list);
		(m_sidedll[iSub].pEntry)(m_sidedll[iSub].pSideDrv, NULL, SIDEcmd_SETSERVLIST, &sub_srv_list);
		delete sub_srv_list.pSLI;
	}
}

//***************************************************************************************
//void CBaseModule::AddServInfoList(PSERV_ListItem pServItem, long index, void* pSideDll, void* pFunc)
void CBaseModule::AddServInfoList(PSERV_ListItem pServItem, UCHAR flagDll, void* pSideDll, void* pFunc)
{
	PSERV_Info psrv_info = new SERV_Info;
	psrv_info->pSrvOvUn = NULL;
	BRDCHAR nameFileMap[MAX_PATH];
	BRDC_sprintf(nameFileMap, _BRDC("suofm_%s_%d"), pServItem->name, GetPID());
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	int isAlreadyCreated;
	psrv_info->hFileMap = IPC_createSharedMemoryEx(nameFileMap, sizeof(OVUN_SRV), &isAlreadyCreated);
	psrv_info->pSrvOvUn = (PSRV_OVUN)IPC_mapSharedMemory(psrv_info->hFileMap);
#else
	psrv_info->hFileMap = CreateFileMapping( INVALID_HANDLE_VALUE,
									NULL, PAGE_READWRITE,
									0, sizeof(SRV_OVUN),
									nameFileMap);
	int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
	psrv_info->pSrvOvUn = (PSRV_OVUN)MapViewOfFile(psrv_info->hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif
	if(psrv_info->pSrvOvUn && !isAlreadyCreated)
	{
		psrv_info->pSrvOvUn->idxOverSrv = -1;
		for(int i = 0; i < MAX_UNDERSRV; i++)
			psrv_info->pSrvOvUn->idxUnderSrv[i] = -1;
	}
//	psrv_info->idxOverSrv = -1;
//	for(iu = 0; iu < MAX_UNDERSRV; iu++)
//		psrv_info->idxUnderSrv[iu] = -1;
	BRDC_strcpy(psrv_info->name, pServItem->name);
	psrv_info->attribute = pServItem->attr;
	psrv_info->index =  pServItem->idxMain;
//	psrv_info->index =  index;
	for(int iu = 0; iu < MAX_UNDERSRV; iu++)
		psrv_info->idxCandidSrv[iu] = -1;
	psrv_info->baseORsideDLL = flagDll;
	psrv_info->pSideDll = pSideDll;
	psrv_info->pSideEntry = pFunc;
	psrv_info->isCaptured = 0;
	m_ServInfoList.push_back(psrv_info);
}
/*
// ***************************************************************************************
void CBaseModule::GetSrvList(PSERV_ListItem srv_list)
{
	int srv_num = m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		_tcscpy(srv_list[i].name, m_SrvList[i]->GetName());
		srv_list[i].attr = m_SrvList[i]->GetAttribute();
		srv_list[i].idxMain = m_SrvList[i]->GetIndex();
		AddServInfoList(&srv_list[i], NULL, NULL);
	}
}
*/
//********************** CmdQuery ************************************************
//	long idx - индекс подслужбы
//	ULONG cmd - код команды
//*************************************************************************************
ULONG CBaseModule::CmdQuery(BRD_Handle handle, long idx, ULONG cmd)
{
	ULONG status = BRDerr_CMD_UNSUPPORTED;
	if(idx < (long)m_SrvList.size())
		status = CModule::CmdQuery(idx, cmd);
	else
	{
		DEV_CMD_Ctrl param;
//		param.handle = handle;
		param.handle = idx;
//		param.handle = m_ServInfoList[idx]->index;
		param.cmd = cmd;
//		param.arg = arg;
		if(m_ServInfoList[idx]->baseORsideDLL)
		{
			SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idx]->pSideEntry;
			status = pSIDE_Entry(m_ServInfoList[idx]->pSideDll, NULL, SIDEcmd_SERVCMDQUERY, &param);
		}
		else
		{
			DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idx]->pSideEntry;
			status = pDEV_Entry(m_ServInfoList[idx]->pSideDll, DEVcmd_SERVCMDQUERY, &param);
		}
	}
	return status;
}

//********************** UnderSrvCapture ************************************************
// захват подслужбы
//	long idxOvSrv - индекс надслужбы
//	long idxUnSrv - индекс подслужбы
//***************************************************************************************
ULONG CBaseModule::UnderSrvCapture(BRD_Handle handle, long idxOvSrv, long idxUnSrv)
{
	ULONG status = BRDerr_INSUFFICIENT_SERVICES;
//	long srv_over_idx = m_SrvList[idxUnSrv]->GetOverIndex();
	long srv_over_idx = m_ServInfoList[idxUnSrv]->pSrvOvUn->idxOverSrv;
	if(srv_over_idx == -1)
	{
		SERV_CMD_IsAvailable srvAvail;
		srvAvail.attr = m_ServInfoList[idxOvSrv]->attribute;
		srvAvail.isAvailable = 0;
		if(idxUnSrv < (long)m_SrvList.size())
			status = m_SrvList[idxUnSrv]->DoCmd(this, SERVcmd_SYS_ISAVAILABLE, 0, &srvAvail, NULL);
		else
		{
			DEV_CMD_Ctrl param;
			param.handle = (idxUnSrv << 16); // подставляем новый
//			param.handle = (m_ServInfoList[idxUnSrv]->index << 16); // подставляем новый
			param.cmd = SERVcmd_SYS_ISAVAILABLE;
			param.arg = &srvAvail;
			if(m_ServInfoList[idxUnSrv]->baseORsideDLL)
			{
				SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[idxUnSrv]->pSideEntry;
				status = pSIDE_Entry(m_ServInfoList[idxUnSrv]->pSideDll, NULL, SIDEcmd_CTRL, &param);
			}
			else
			{
				DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[idxUnSrv]->pSideEntry;
				status = pDEV_Entry(m_ServInfoList[idxUnSrv]->pSideDll, DEVcmd_CTRL, &param);
			}
		}
		if(status != BRDerr_OK)
			return status;
		ULONG avai = srvAvail.isAvailable;
		if(avai)
		{
//			m_SrvList[idxUnSrv]->SetOverIndex(idxOvSrv);
			m_ServInfoList[idxUnSrv]->pSrvOvUn->idxOverSrv = idxOvSrv;
			m_ServInfoList[idxUnSrv]->isCaptured = 1;
			int i = 0;
//			while((m_SrvList[idxOvSrv]->GetUnderIndex(i) != -1) && (i < MAX_UNDERSRV)) i++;
			while((m_ServInfoList[idxOvSrv]->pSrvOvUn->idxUnderSrv[i] != -1)  && (i < MAX_UNDERSRV)) i++;
			if(i >= MAX_UNDERSRV)
				return BRDerr_INSUFFICIENT_SERVICES;
//			m_SrvList[idxOvSrv]->SetUnderIndex(i, idxUnSrv);
			m_ServInfoList[idxOvSrv]->pSrvOvUn->idxUnderSrv[i] = idxUnSrv;
//			srv_over_idx = m_SrvList[idxUnSrv]->GetOverIndex();
			srv_over_idx = m_ServInfoList[idxUnSrv]->pSrvOvUn->idxOverSrv;
			handle &= 0xFF80FFFF; // обнуляем индекс службы
			handle |= (idxUnSrv << 16); // подставляем новый
			ULONG mode = (handle & 0x01800000) >> 23; // выделяем режим захвата из дескриптора
			BRDI_SUP_Capture CaptureStruc = { handle, 0, (S32)idxUnSrv, mode};
			BRDI_support(BRDIsup_CAPTURE, &CaptureStruc);
		}
		else
			status = BRDerr_CMD_UNSUPPORTED;
	}
	if(srv_over_idx == idxOvSrv)
		status = BRDerr_OK;
	return status;
}

//********************** UnderSrvRelease ************************************************
// освобождение подслужбы
//	long idxOvSrv - индекс надслужбы
//	long idxUnSrv - индекс подслужбы
//***************************************************************************************
ULONG CBaseModule::UnderSrvRelease(BRD_Handle handle, long idxOvSrv, long idxUnSrv)
{
	ULONG status = BRDerr_OK;
//	if(m_SrvList[idxUnSrv]->GetOverIndex() == idxOvSrv)
	if(m_ServInfoList[idxUnSrv]->pSrvOvUn->idxOverSrv == idxOvSrv)
	{
		handle &= 0xFF80FFFF; // обнуляем индекс службы
		handle |= (idxUnSrv << 16); // подставляем новый
		ULONG mode = (handle & 0x01800000) >> 23; // выделяем режим захвата из дескриптора
		BRDI_SUP_Release ReleaseStruc = {handle, 0, (S32)idxUnSrv, mode};
		BRDI_support(BRDIsup_RELEASE, &ReleaseStruc);
//		m_SrvList[idxUnSrv]->SetOverIndex(-1);
		m_ServInfoList[idxUnSrv]->pSrvOvUn->idxOverSrv = -1;
		m_ServInfoList[idxUnSrv]->isCaptured = 0;
		int i = 0;
//		while((m_SrvList[idxOvSrv]->GetUnderIndex(i) != idxUnSrv) && (i < MAX_UNDERSRV)) i++;
		while((m_ServInfoList[idxOvSrv]->pSrvOvUn->idxUnderSrv[i] != idxUnSrv) && (i < MAX_UNDERSRV)) i++;
		if(i < MAX_UNDERSRV)
//			m_SrvList[idxOvSrv]->SetUnderIndex(i, -1);
			m_ServInfoList[idxOvSrv]->pSrvOvUn->idxUnderSrv[i] = -1;
		else
			status = BRDerr_INSUFFICIENT_SERVICES;
	}
	return status;
}

//********************** UnderSrvCtrl ************************************************
// Выполнение команды в подслужбе
//	BRD_Handle handle - дескриптор надслужбы
//***************************************************************************************
ULONG CBaseModule::UnderSrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext)
{
	ULONG status = BRDerr_CMD_UNSUPPORTED;
	long idxOvSrv = (handle >> 16) & 0x3F;
	int i = 0;
//	while(m_SrvList[idxOvSrv]->GetUnderIndex(i) != -1)
	while(m_ServInfoList[idxOvSrv]->pSrvOvUn->idxUnderSrv[i] != -1)
	{ // сначала идем по подслужбам (уже захваченным)
//		long ius = m_SrvList[idxOvSrv]->GetUnderIndex(i++);
		long ius = m_ServInfoList[idxOvSrv]->pSrvOvUn->idxUnderSrv[i++];
		handle &= 0xFF80FFFF; // обнуляем индекс службы
		handle |= (ius << 16); // подставляем новый
		UNDERSERV_Cmd new_cmd = {0, NULL};
		if(ius < (long)m_SrvList.size())
		{
//			status = CModule::SrvCtrl(handle, cmd, arg, NULL);
			status = CModule::SrvCtrl(handle, cmd, arg, &new_cmd);
			if(new_cmd.code == SERVcmd_SYS_RELEASE)
				UnderSrvRelease(handle, idxOvSrv, ius);
		}
		else
		{
			DEV_CMD_Ctrl param;
//			handle |= (m_ServInfoList[ius]->index << 16); // подставляем новый
			param.handle = handle;
			param.cmd = cmd;
			param.arg = arg;
			if(m_ServInfoList[ius]->baseORsideDLL)
			{
				SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[ius]->pSideEntry;
				status = pSIDE_Entry(m_ServInfoList[ius]->pSideDll, NULL, SIDEcmd_CTRL, &param);
			}
			else
			{
				DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[ius]->pSideEntry;
				status = pDEV_Entry(m_ServInfoList[ius]->pSideDll, DEVcmd_CTRL, &param);
			}
		}
		if((status != BRDerr_CMD_UNSUPPORTED) || (i >= MAX_UNDERSRV))
			break;
	}
	return status;
}

//********************** CandidSrvCtrl ************************************************
// дозахват подслужбы и выполнение команды в ней
//	BRD_Handle handle - дескриптор надслужбы
//***************************************************************************************
ULONG CBaseModule::CandidSrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext)
{
	ULONG status = BRDerr_CMD_UNSUPPORTED;
	long idxOvSrv = (handle >> 16) & 0x3F;
	int i = 0;
//	while(m_SrvList[idxOvSrv]->GetCandidIndex(i) != -1)
	while(m_ServInfoList[idxOvSrv]->idxCandidSrv[i] != -1)
	{ // теперь идем по конечным службам (они потенциально могут стать подслужбами этой службы)
//		long ius = m_SrvList[idxOvSrv]->GetCandidIndex(i++);
		long ius = m_ServInfoList[idxOvSrv]->idxCandidSrv[i++];
		status = CmdQuery(handle, ius, cmd);
		if(status == BRDerr_OK)
		{
			status = UnderSrvCapture(handle, idxOvSrv, ius);
			if(status == BRDerr_OK)
			{
				handle &= 0xFF80FFFF; // обнуляем индекс службы
				handle |= (ius << 16); // подставляем новый
				if(ius < (long)m_SrvList.size())
					status = CModule::SrvCtrl(handle, cmd, arg, NULL);
				else
				{
					DEV_CMD_Ctrl param;
//					handle |= (m_ServInfoList[ius]->index << 16); // подставляем новый
					param.handle = handle;
					param.cmd = cmd;
					param.arg = arg;
					if(m_ServInfoList[ius]->baseORsideDLL)
					{
						SIDE_API_entry* pSIDE_Entry = (SIDE_API_entry*)m_ServInfoList[ius]->pSideEntry;
						status = pSIDE_Entry(m_ServInfoList[ius]->pSideDll, NULL, SIDEcmd_CTRL, &param);
					}
					else
					{
						DEV_API_entry* pDEV_Entry = (DEV_API_entry*)m_ServInfoList[ius]->pSideEntry;
						status = pDEV_Entry(m_ServInfoList[ius]->pSideDll, DEVcmd_CTRL, &param);
					}
				}
				break;
			}
			else
				status = BRDerr_INSUFFICIENT_SERVICES;
		}
		else
			status = BRDerr_INSUFFICIENT_SERVICES;
	}
	return status;
}

//********************** StreamCtrl *****************************************************
// управление стримами
//***************************************************************************************
ULONG CBaseModule::StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap)
{
	return BRDerr_OK;
}

//***************** End of file basemodule.cpp *****************
