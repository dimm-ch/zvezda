/*
 ***************** File BasedllIni.cpp ********************
 *
 * Initialization Exported functions
 * for BRD Driver for AMBxxx Data Acquisition Carrier Boards, 
 * based on PLX9x56 PCI Controller.
 *
 * (C) InSys by Dorokhin A. Apr 2004
 *
 ******************************************************
*/

#define	DEV_API_EXPORTS
#define	DEVS_API_EXPORTS

#include "basedll.h"

#define	CURRFILE _BRDC("BASEDLLINI")

//********************** DllMain ***********************
//******************************************************
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
//******************************************************
//
// DRIVER EXPORTED FUNCTIONS
//
//******************************************************

//*******************************************************
// DEV_initData - полуавтоматическая инициализация (в соответствии с секциями LID:##)
//  Input:  idxThisBrd - последовательный номер обнаружения БМ одного типа в ИИ
//          pInitData - массив структур с содержимым ИИ
//			size - количество элементов в pInitData
//  Output: ppDev - указатель на объект класса БМ
//			pBoardName - строка, содержащая уникальное имя БМ (< 20 байт)
//			return - if OK returns boardType, else > 0
//  Notes:  Функция вызывается столько раз, 
//			сколько логических записей (LID) в источнике инициализации
//*******************************************************
DEV_API S32	STDCALL DEV_initData(S32 idxThisBrd, void **ppDev, BRDCHAR *pBoardName, PBRD_InitData pBrdInitData, S32 size)
{
	BASE_CLASS	*pDev;			// DEV Extension
//	BASE_CLASS *pBoard = new BASE_CLASS(pBrdInitData, size);
	pDev = new BASE_CLASS(pBrdInitData, size);
	if(pDev)
	{
		S32		ret;			// Return value
		//ret = pBoard->Init(idxThisBrd, pBoardName);
		ret = pDev->Init(idxThisBrd, pBoardName);
		if(ret <= 0)
		{
			delete pDev;
			return ret;
		}
	}
	else
	{
		S32 err_code = DRVERR(BRDerr_INSUFFICIENT_RESOURCES);
		BRDI_print(GETERRLVL(err_code), CURRFILE, _BRDC("<DEV_initData> No enough memory"));
		return err_code;
	}

//	*ppDev = pBoard;
	*ppDev = pDev;

	// Connect SIDE Drivers
//	pDev = (BASE_CLASS*)*ppDev;
	pDev->InitDataSubDll(pBrdInitData, size );

	BRD_Info info;
	info.size = sizeof(info);
	pDev->GetDeviceInfo(&info);
	return info.boardType;
}

//************************************************
// DEV_initAuto - автоматическая инициализация
//  Input:  idxThisBrd - счетчик вызовов
//			pSubEnum - массив со списком драйверов СМ
//			sizeSubEnum - размер массива pSubEnum
//  Output: ppDev - указатель на объект класса БМ
//			pBoardName - строка, содержащая уникальное имя БМ (< 20 байт)
//			pLin - строка, содержащая информацию о БМ для формирования отчета
//			linSize - размер строки pLin (в байтах)
//			return - if OK returns boardType, else > 0
//  Notes:  Функция вызывается BRD_COUNT раз (64)
//
//************************************************
//DEV_API S32	STDCALL DEV_initAuto(S32 idxThisBrd, void **ppDev, PTSTR pBoardName, PTSTR pLin, S32 linSize, PBRD_InitEnum pSubEnum, U32 sizeSubEnum)
DEV_API S32	STDCALL DEV_initAuto(S32 idxThisBrd, void **ppDev, BRDCHAR *pBoardName, BRDCHAR *pLin, S32 linSize, PBRD_SubEnum pSubEnum, U32 sizeSubEnum)
{
	BASE_CLASS	*pDev;			// DEV Extension
	//S32	errorCode = DRVERR(BRDerr_ERROR);

	pDev = new BASE_CLASS;
//	HANDLE handle = GetProcessHeap();
	if(pDev)
	{
		S32 ret = pDev->Init(idxThisBrd, pBoardName, pLin);
		if(ret <= 0) {
			delete pDev;
			return ret;
		}
	}
	else
	{
		S32 err_code = DRVERR(BRDerr_INSUFFICIENT_RESOURCES);
		BRDI_print(GETERRLVL(err_code), CURRFILE, _BRDC("<DEV_initAuto> No enough memory"));
		return err_code;
//		return Dev_ErrorPrintf( NULL, DRVERR(BRDerr_INSUFFICIENT_RESOURCES), 
//								CURRFILE, "<DEV_initAuto> No enough memory");
	}

	*ppDev = pDev;

	// Link SUB Drivers
	//pDev = (BASE_CLASS*)*ppDev;
	pDev->InitAutoSubDll(pLin, linSize, pSubEnum, sizeSubEnum);

	BRD_Info info;
	info.size = sizeof(info);
	pDev->GetDeviceInfo(&info);
	return info.boardType;
}

//********************** DEVS_entry *******************
//*****************************************************
DEVS_API S32  STDCALL DEVS_entry( void *pDev, void *pContext, S32 cmd, void *args )
{
	ULONG status = BRDerr_OK;
	BASE_CLASS* pBaseDev = (BASE_CLASS*)pDev;
	{ // команда работы с аппаратурой
		PDEVS_CMD_Reg pParam = (PDEVS_CMD_Reg)args;
		status = pBaseDev->RegCtrl(cmd, pParam);
	}
//	return DRVERR(BRDerr_OK);
	return status;
}

//******************************************************
DEVS_API S32  STDCALL DEVS_propDlg(void* pDev, void* pServ, void* pParam)
{
	BASE_CLASS* pBaseDev = (BASE_CLASS*)pDev;
	S32	ret = pBaseDev->PropertyFromDialog(pServ, pParam);
	return ret;
}

//***************** End of file BasedllIni.cpp *****************
