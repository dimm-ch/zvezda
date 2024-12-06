/*
 ***************** File service.cpp ***********************
 *
 * (C) InSys by Dorokhin A. Nov 2003
 *
 ******************************************************
*/

#include "module.h"
#include "service.h"

#define	CURRFILE "SERVICE"

CMD_Info ISAVAILABLE_CMD	= { SERVcmd_SYS_ISAVAILABLE, 1, 0, 0, NULL};
CMD_Info CAPTURE_CMD		= { SERVcmd_SYS_CAPTURE,	 1, 0, 0, NULL};
CMD_Info RELEASE_CMD		= { SERVcmd_SYS_RELEASE,	 0, 0, 0, NULL};
CMD_Info GETADDRDATA_CMD	= { SERVcmd_SYS_GETADDRDATA, 1, 0, 0, NULL};

//***************************************************************************************
CService::CService(int idx, const BRDCHAR *name, int num, PVOID pModule, PVOID pCfg, ULONG cfgSize)
{
	m_pModule = pModule;
	m_pOvUnSrv = NULL;
	m_pConfig = NULL;
	m_hFileMap = NULL;
	m_hMutex = NULL;
	if(num != -1)
	{
		BRDC_sprintf(m_name, _BRDC("%s%d"), name, num);
		SetCfgMem(pCfg, cfgSize);
	}
	else
		BRDC_strcpy(m_name, name);
	m_attribute = 0;
	m_index = idx;
	for(int i = 0; i < MAX_UNDERSRV; i++)
		m_idxCandidSrv[i] = -1;
	m_isAvailable = 0;

	Init(&ISAVAILABLE_CMD, (CmdEntry)&CService::CtrlIsAvailable);
	Init(&CAPTURE_CMD, (CmdEntry)&CService::CtrlCapture);
	Init(&RELEASE_CMD, (CmdEntry)&CService::CtrlRelease);
	Init(&GETADDRDATA_CMD, (CmdEntry)&CService::CtrlGetAddrData);
}

//***************************************************************************************
CService::~CService()
{
	if(m_hMutex)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_deleteMutex(m_hMutex);
#else
		CloseHandle(m_hMutex);
#endif
	if(m_pConfig)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_unmapSharedMemory(m_hFileMap);
#else
		UnmapViewOfFile(m_pConfig);
#endif
	if(m_hFileMap)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_deleteSharedMemory(m_hFileMap);
#else
		CloseHandle(m_hFileMap);
#endif
}

//***************************************************************************************
void CService::SetCfgMem(PVOID pCfg, ULONG cfgSize)
{
	CModule* pModule = (CModule*)m_pModule;
	if(!m_hMutex)
	{
		BRDCHAR nameMutex[MAX_PATH];
		BRDC_sprintf(nameMutex, _BRDC("mutex_%s_%d"), m_name, pModule->GetPID());
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		m_hMutex = IPC_createMutex(nameMutex, FALSE);
#else
		m_hMutex = CreateMutex(NULL, FALSE, nameMutex);
#endif
	}
	if(!m_pConfig)
	{
		BRDCHAR nameFileMap[MAX_PATH];
		BRDC_sprintf(nameFileMap, _BRDC("fm_%s_%d"), m_name, pModule->GetPID());
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		int isAlreadyCreated;
		m_hFileMap = IPC_createSharedMemoryEx(nameFileMap, cfgSize + sizeof(OVUN_SRV), &isAlreadyCreated);
		m_pConfig = IPC_mapSharedMemory(m_hFileMap);
#else
		m_hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE,
											NULL, PAGE_READWRITE,
											0, cfgSize + sizeof(OVUN_SRV),
											nameFileMap);
		int isAlreadyCreated = ( GetLastError() == ERROR_ALREADY_EXISTS ) ? 1 : 0;
		m_pConfig = (PVOID)MapViewOfFile(m_hFileMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
#endif


		m_pOvUnSrv = (POVUN_SRV)((PUCHAR)m_pConfig + cfgSize);
		if(m_pConfig && !isAlreadyCreated)
		{
			memcpy(m_pConfig, pCfg, cfgSize);
			m_pOvUnSrv->idxOverSrv = -1;
			for(int i = 0; i < MAX_UNDERSRV; i++)
				m_pOvUnSrv->idxUnderSrv[i] = -1;
		}
	}
}

//***************************************************************************************
void CService::SetPropDlg(PVOID pPropDlg)
{
	m_pPropDlg = (DEVS_API_PropDlg*)pPropDlg;
}

//***************************************************************************************
void CService::Init(PCMD_Info pCmdInfo, CmdEntry pEntry)
{
	pCmdInfo->pEntry = pEntry;
	m_Cmd.push_back(pCmdInfo);
}

//***************************************************************************************
int CService::CtrlIsAvailable(
						  void			*pDev,		// InfoSM or InfoBM
						  void			*pServData,	// Specific Service Data
						  ULONG			cmd,		// Command Code (from BRD_ctrl())
						  void			*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	pServAvailable->isAvailable = m_isAvailable;

	return BRDerr_OK;
}

//***************************************************************************************
int CService::SetPropertyFromDialog(
									void*	pDev,
									void*	args
									)
{
	//CModule* pModule = (CModule*)pDev;
	return BRDerr_OK;
}

//***************************************************************************************
int CService::CtrlCapture(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CService::CtrlRelease(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CService::CtrlGetAddrData(
							void			*pDev,		// InfoSM or InfoBM
							void			*pServData,	// Specific Service Data
							ULONG			cmd,		// Command Code (from BRD_ctrl())
							void			*args 		// Command Arguments (from BRD_ctrl())
							)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
ULONG CService::DoCmd(PVOID pMod, ULONG cmd, int mode, PVOID pData, void* pServData)
{
	int cmd_num = (int)m_Cmd.size();
	int i = 0;
	for(i = 0; i < cmd_num; i++)
		if(m_Cmd[i]->code == cmd)
			break;
	if(i == cmd_num)
		return BRDerr_CMD_UNSUPPORTED;
	if(mode == BRDcapt_SPY && !m_Cmd[i]->isSpy)
		return BRDerr_BAD_MODE;
	if(m_hMutex)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_captureMutex(m_hMutex, INFINITE);
#else
		WaitForSingleObject(m_hMutex, INFINITE);
#endif
	ULONG status = (this->*(m_Cmd[i]->pEntry))(pMod, pServData, cmd, pData);
	if(m_hMutex)
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_releaseMutex(m_hMutex);
#else
		ReleaseMutex(m_hMutex);
#endif
	return status;
}

//***************************************************************************************
ULONG CService::IsCmd(ULONG cmd)
{
	int cmd_num = (int)m_Cmd.size();
	int i = 0;
	for(i = 0; i < cmd_num; i++)
		if(m_Cmd[i]->code == cmd)
			break;
	if(i == cmd_num)
		return BRDerr_CMD_UNSUPPORTED;
	return BRDerr_OK;
}

// Записать параметр целого типа в ini-файл:
//    FileName - имя ini-файла (с путем, если нужно)
//    SectionName - название секции
//    ParamName - название параметра (ключа)
//    value - значение параметра
//    base - представление параметра (шестнадцатиричное (16) или десятичное (10))
//    Comment - комментарий (если NULL, то сохраняется прежний комментарий, если он был)
void CService::WriteInifileParam(const BRDCHAR* FileName, const BRDCHAR* SectionName, const BRDCHAR* ParamName, ULONG value, int base, const BRDCHAR* Comment)
{
	BRDCHAR Buffer[255];
	BRDCHAR BufComment[128] = _BRDC("");
	if(Comment)
		//lstrcpy(BufComment, Comment); // если задан комментарий, то просто копируем его
		BRDC_sprintf(BufComment, _BRDC("\t\t\t;%s"), Comment); // если комментарий задан
	else
	{  // ищем параметр в файле и комментарий к нему
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("NONAND"), Buffer, sizeof(Buffer), FileName);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("NONAND"), Buffer, sizeof(Buffer), FileName);
#endif
		if(BRDC_strcmp(Buffer, _BRDC("NONAND")))
		{ // параметр есть в файле
			BRDCHAR* pChar = BRDC_strchr(Buffer, _BRDC(';'));
			if(pChar)
			{ // комментарий есть в строке
				//lstrcpy(BufComment, pChar+sizeof(BRDCHAR));
				BRDC_sprintf(BufComment, _BRDC("\t\t\t;%s"), pChar+sizeof(BRDCHAR));
			}
		}
	}
	// между значением параметра и комментарием вставляются символы табуляции
	if(base == 16)
		BRDC_sprintf(Buffer, _BRDC("0x%X%s"), value, BufComment);
	else
		BRDC_sprintf(Buffer, _BRDC("%u%s"), value, BufComment);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_writePrivateProfileString(SectionName, ParamName, Buffer, FileName);
#else
	WritePrivateProfileString(SectionName, ParamName, Buffer, FileName);
#endif
}

// Записать параметр типа double в ini-файл:
//    FileName - имя ini-файла (с путем, если нужно)
//    SectionName - название секции
//    ParamName - название параметра (ключа)
//    value - значение параметра
//    prec - точность представления параметра (число знаков после запятой: 4 или 2)
//    Comment - комментарий (если NULL, то сохраняется прежний комментарий, если он был)
//void CService::WriteInifileParam(LPCTSTR FileName, LPCTSTR SectionName, LPCTSTR ParamName, double value, int prec, LPCTSTR Comment)
void CService::WriteInifileParam(const BRDCHAR* FileName, const BRDCHAR* SectionName, const BRDCHAR* ParamName, double value, int prec, const BRDCHAR* Comment)
{
	BRDCHAR Buffer[255];
	BRDCHAR BufComment[128] = _BRDC("");
	if(Comment)
		BRDC_sprintf(BufComment, _BRDC("\t\t\t;%s"), Comment); // если комментарий задан
	else
	{  // ищем параметр в файле и комментарий к нему
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("NONAND"), Buffer, sizeof(Buffer), FileName);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("NONAND"), Buffer, sizeof(Buffer), FileName);
#endif
		if(BRDC_strcmp(Buffer, _BRDC("NONAND")))
		{ // параметр есть в файле
			BRDCHAR* pChar = BRDC_strchr(Buffer, ';');
			if(pChar)
			{ // комментарий есть в строке
				BRDC_sprintf(BufComment, _BRDC("\t\t\t;%s"), pChar+sizeof(BRDCHAR));
			}
		}
	}
	// между значением параметра и комментарием вставляются символы табуляции
	if(prec == 4)
		BRDC_sprintf(Buffer, _BRDC("%.4f%s"), value, BufComment);
	else
		BRDC_sprintf(Buffer, _BRDC("%.2f%s"), value, BufComment);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_writePrivateProfileString(SectionName, ParamName, Buffer, FileName);
#else
	WritePrivateProfileString(SectionName, ParamName, Buffer, FileName);
#endif
}

//***************************************************************************************
S32		CService::IndRegRead( void* pModule, S32 tetrNo, S32 regNo, U32 *pVal )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	((CModule*)pModule)->RegCtrl(DEVScmd_REGREADIND, &param);
	*pVal = param.val;

	return BRDerr_OK;
}

//***************************************************************************************
S32		CService::IndRegWrite( void* pModule, S32 tetrNo, S32 regNo, U32 val )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	param.val = val;
	((CModule*)pModule)->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
S32		CService::DirRegRead( void* pModule, S32 tetrNo, S32 regNo, U32 *pVal )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	((CModule*)pModule)->RegCtrl(DEVScmd_REGREADDIR, &param);
	*pVal = param.val;

	return BRDerr_OK;
}

//***************************************************************************************
S32		CService::DirRegWrite( void* pModule, S32 tetrNo, S32 regNo, U32 val )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	param.val = val;
	((CModule*)pModule)->RegCtrl(DEVScmd_REGWRITEDIR, &param);

	return BRDerr_OK;
}

// ***************** End of file service.cpp ***********************
