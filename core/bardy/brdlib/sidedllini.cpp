/*
 ***************** File sidedllini.cpp ********************
 *
 * Initialization Exported functions
 * for BRD Driver for SIDEDLL
 *
 * (C) InSys by Dorokhin A. Jul 2002
 *
 ******************************************************
*/

#define	SIDE_API_EXPORTS

#include "sidedll.h"

#define	CURRFILE _BRDC("SIDEDLLINI")

//=********************** DllMain ***********************
//=******************************************************
#ifdef _WIN32
BOOL __stdcall DllMain( HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved )
{
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}

//=********************** DllEntryPoint *****************
//=******************************************************
BOOL __stdcall DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
        // Included for compatibility with Borland 
        return DllMain (hinstDLL, fdwReason, lpvReserved);
}
#endif
//=********************** Sub_ErrorPrintf ****************
//=*******************************************************
S32	Sub_ErrorPrintf(void* ptr, S32 errorCode, const BRDCHAR *title, const BRDCHAR *msg)
{
	if( ptr && errorCode<0 )
	{
		SIDE_CLASS *pSub = (SIDE_CLASS*)ptr;
		pSub->SetError(errorCode);
	}

	BRDI_print(GETERRLVL(errorCode), title, msg );
	return errorCode;
}

//=********************** Sub_Printf *********************
//=*******************************************************
S32 Sub_Printf(void* ptr, S32 errorCode, const BRDCHAR *title, const BRDCHAR *format, ... )
{
	if( ptr && errorCode<0 )
	{
		SIDE_CLASS *pSub = (SIDE_CLASS*)ptr;
		pSub->SetError(errorCode);
	}

	va_list		arglist;

	va_start( arglist, format );		 
	BRDI_printf(GETERRLVL(errorCode), title, format, arglist );
	va_end( arglist );

	return errorCode;
}

//=******************************************************
//
// DRIVER EXPORTED FUNCTIONS
//
//

//=************************ SIDE_initData ****************
// Return:  if OK returns boardType, Error returns >0.
//=******************************************************
SIDE_API S32  STDCALL SIDE_initData(void **ppSub, PVOID pBufICR, PBRD_InitData pInitData, S32 size)
{
	SIDE_CLASS *pSub;			// DEV Extension
	pSub = new SIDE_CLASS(pInitData, size);
	if(pSub)
	{
		S32 RetVal = pSub->Init((PUCHAR)pBufICR);
		if(RetVal <= 0)	{
			delete pSub;
			return RetVal;
		}
	}
	else
	{
		return Sub_ErrorPrintf( NULL, SIDEERR(BRDerr_INSUFFICIENT_RESOURCES),
								CURRFILE, _BRDC("<SIDE_initData> No enough memory"));
	}

	*ppSub = (void*)pSub;
	
	BRD_Info info;
	info.size = sizeof(info);
	pSub->GetDeviceInfo(&info);

	return info.boardType;
}

//=************************ SIDE_initAuto ****************
// Return:  if OK returns boardType, Error returns >0.
//=******************************************************
SIDE_API S32  STDCALL SIDE_initAuto(void **ppSub, PVOID pBufICR, BRDCHAR *pLin, S32 linSize)
{
	SIDE_CLASS *pSub = new SIDE_CLASS;
	if(pSub)
	{
		S32 ret = pSub->Init((PUCHAR)pBufICR, pLin);
		if(ret <= 0)
		{
			delete pSub;
			return ret;
		}
	}
	else
	{
		return Sub_ErrorPrintf( NULL, SIDEERR(BRDerr_INSUFFICIENT_RESOURCES), 
								CURRFILE, _BRDC("<SIDE_initAuto> No enough memory"));
	}

	*ppSub = (void*)pSub;	

	BRD_Info info;
	info.size = sizeof(info);
	pSub->GetDeviceInfo(&info);

	return info.boardType;
}

//******************************************************
SIDE_API S32  STDCALL SIDE_propDlg(void* pDev, void* pServ, void* pParam)
{
	SIDE_CLASS *pSub = (SIDE_CLASS *)pDev;
	S32	ret = pSub->PropertyFromDialog(pServ, pParam);
	return ret;
}

//***************** End of file sidedllini.cpp *****************
