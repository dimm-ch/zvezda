/*
 ***************** File module.cpp ***********************
 *
 * (C) InSys by Dorokhin A. Jan 2004
 *
 ******************************************************
*/

#include "module.h"


#define	CURRFILE "MODULE"

//***************************************************************************************
CModule::CModule()
{
	m_errcode = 0;
	m_errcnt = 0;
	
#ifdef _WIN32
	m_langID = LANG_NEUTRAL;
#else
        m_langID = 0;
#endif
	BRDC_sprintf(m_name, _BRDC("CMODULE_0000"));

	m_puCnt = 0;
}

//***************************************************************************************
CModule::~CModule()
{
}

//***************************************************************************************
//extern "C" void * _ReturnAddress();
//#pragma intrinsic(_ReturnAddress)

//int CModule::GetStringText(UINT uID, PTSTR text)
//{
//	int const nBufSize = 128;
//	BRDCHAR StringText[nBufSize];
//	HINSTANCE hInstExe = NULL;
//	MEMORY_BASIC_INFORMATION mem;
//    if (VirtualQuery(_ReturnAddress(), &mem, sizeof(mem)))
//    {
////        _ASSERTE(mem.Type == MEM_IMAGE);
////        _ASSERTE(mem.AllocationBase != NULL);
//        hInstExe = (HINSTANCE)mem.AllocationBase;
//    }
//	int ret = LoadString((hInstExe, uID, StringText, nBufSize);
//	lstrcpy(text, StringText);
//	return ret;
//}

//***************************************************************************************
//int CModule::GetStringText(DWORD dwMessageId, LPTSTR strMsg) 
//{
//	HINSTANCE hInstExe = NULL;
//	MEMORY_BASIC_INFORMATION mem;
//    if (VirtualQuery(_ReturnAddress(), &mem, sizeof(mem)))
//        hInstExe = (HINSTANCE)mem.AllocationBase;
//	LPVOID lpMsgBuf;
//	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
////					AfxGetInstanceHandle(), dwMessageId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
////					hInstExe, dwMessageId, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
//					hInstExe, dwMessageId, MAKELANGID(m_langID, SUBLANG_DEFAULT),
//					(LPTSTR) &lpMsgBuf, 0, NULL );
//	lstrcpy(strMsg, (LPTSTR)lpMsgBuf);
//	LocalFree( lpMsgBuf );
//	strMsg[lstrlen(strMsg) - 2] = '\0';
//	//strMsg[lstrlen(strMsg) + 1] = '\0';
//	return 1;
//}

//***************************************************************************************
//void CModule::WriteEventLog(LONG errorCode, LPCTSTR strMsg)
//{
//    HANDLE h; 
// 
//    h = RegisterEventSource(NULL,  // uses local computer 
////							m_name);    // source name 
//							_BRDC("Bambp"));    // source name 
//    //if (h == NULL) 
//    //    ErrorExit("Could not register the event source."); 
// 
//    ReportEvent(h,				  // event log handle 
//            EVENTLOG_ERROR_TYPE,  // event type 
//            0,                    // category zero 
//            errorCode,			  // event identifier 
//            NULL,                 // no user security identifier 
//            1,                    // one substitution string 
//            0,                    // no data 
//            &strMsg,				  // pointer to string array 
//            NULL);                // pointer to data 
// 
//    DeregisterEventSource(h); 
//}

//***************************************************************************************
void CModule::SetError(LONG	errorCode)
{
	m_errcnt++;
	m_errcode = errorCode;
	BRD_Error err;
	err.errCode = errorCode;
//	err.errTime = 
//	err.srcModuleName = 
//	err.srcFileName = 
//	err.srcLine = 
//	err.errText = 
	BRDI_setError(&err);
}

//********************** ErrorPrint *********************
// Display error message
//*******************************************************
LONG CModule::ErrorPrint(LONG errorCode, const BRDCHAR *title, const BRDCHAR *msg)
{
	if(errorCode < 0)
		SetError(errorCode);

	BRDI_print(GETERRLVL(errorCode), title, msg );
	return errorCode;
}

//********************** ErrorPrintf ********************
// Display formatting error message
//*******************************************************
LONG CModule::ErrorPrintf(LONG errorCode, const BRDCHAR *title, const BRDCHAR *format, ... )
{
	if(errorCode < 0)
		SetError(errorCode);

	va_list		arglist;

	va_start( arglist, format );		 
	BRDI_printf(GETERRLVL(errorCode), title, format, arglist );
	va_end( arglist );
	return errorCode;
}

//***************************************************************************************
void CModule::GetSrvList(PSERV_ListItem srv_list)
{
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		BRDC_strcpy(srv_list[i].name, m_SrvList[i]->GetName());
		srv_list[i].attr = m_SrvList[i]->GetAttribute();
		srv_list[i].idxMain = m_SrvList[i]->GetIndex();
	}
}

//***************************************************************************************
ULONG CModule::SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, void* pContext)
{
	int idx	 = (handle >> 16) & 0x3F;
	int mode = (handle >> 23) & 0x3;

	ULONG status = BRDerr_CMD_UNSUPPORTED;
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		if(m_SrvList[i]->GetIndex() == idx)
		{
			status = m_SrvList[i]->DoCmd(this, cmd, mode, arg, pContext);
			break;
		}
	}
	return status;
}

//********************** CmdQuery ************************************************
//	long idx - индекс подслужбы
//	ULONG cmd - код команды
//*************************************************************************************
ULONG CModule::CmdQuery(long idx, ULONG cmd)
{
	ULONG status = BRDerr_CMD_UNSUPPORTED;
	int srv_num = (int)m_SrvList.size();
	for(int i = 0; i < srv_num; i++)
	{
		if(m_SrvList[i]->GetIndex() == idx)
		{
			status = m_SrvList[i]->IsCmd(cmd);
			break;
		}
	}
	return status;
}

//***************************************************************************************
LONG CModule::RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam)
{
	return BRDerr_OK;
}

//***************************************************************************************
ULONG CModule::PropertyFromDialog(void* pServ, void* pParam)
{
	CService* pSrv = (CService*)pServ;
	pSrv->SetPropertyFromDialog(this, pParam);
	return BRDerr_OK;
}

//***************** End of file module.cpp *****************
