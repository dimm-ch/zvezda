//****************** File SdramSrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы SDRAM базового модуля
//
//  Constants, structures & functions definitions
//	for API Property Dialog of SDRAM service
//
//	Copyright (c) 2002-2005, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  17-02-05 - builded
//
//*******************************************************************************

#ifndef _SDRAMSRVINFO_H
 #define _SDRAMSRVINFO_H

#ifdef _WIN32
	#ifdef SRVINFO_API_EXPORTS
	 #define SRVINFO_API __declspec(dllexport)
	#else
	 #define SRVINFO_API __declspec(dllimport)
	#endif
#else
    #define SRVINFO_API
#endif

#define NAME_SIZE 64

#pragma pack(1)

// Struct info about device
typedef struct _SDRAMSRV_INFO {
	USHORT		Size;				// sizeof(SDRAMSRV_INFO)
	BRDCHAR		Name[NAME_SIZE];	// название службы и модуля
	UCHAR		SlotCnt;			// количество установленных слотов
	UCHAR		ModuleCnt;			// количество установленных DIMM-модулей (занятых слотов)
	UCHAR		RowAddrBits;		// количество разрядов адреса строк
	UCHAR		ColAddrBits;		// количество разрядов адреса столбцов
	UCHAR		ModuleBanks;		// количество банков в DIMM-модуле
	UCHAR		ChipBanks;			// количество банков в микросхемах DIMM-модуля
	ULONG		ReadMode;			// режим чтения памяти (авто/произвольный)
	ULONG		StartAddr;			// начальный адрес сбора данных (активной зоны) (в 32-битных словах)
	ULONG		ReadAddr;			// начальный адрес чтения (только в произвольном режиме)(в 32-битных словах)
	ULONG		PostTrigSize;		// размер пост-триггера (только в режиме претриггера) (в 32-битных словах)
} SDRAMSRV_INFO, *PSDRAMSRV_INFO;

#pragma pack()

#ifdef _WIN32
// 0 - eng; 1 - rus
static ULONG GetRegistryLocalParams()
{
	ULONG retcode = 0;
	HKEY hkey;
	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _BRDC("System\\CurrentControlSet\\Control\\Nls\\Locale"), 0, KEY_QUERY_VALUE, &hkey);
	DWORD Type = REG_SZ;
	DWORD retSize = MAX_PATH;
	BRDCHAR szBuffer[MAX_PATH];
	if(RegQueryValueEx(hkey, _BRDC(""), NULL, &Type, (LPBYTE)szBuffer, &retSize) == ERROR_SUCCESS)
		if(!lstrcmpi(szBuffer, _BRDC("00000419")))
			retcode = 1;
	RegCloseKey(hkey);
	return retcode;
}
#endif

// Functions Declaration

#ifdef	__cplusplus
extern "C" {
#endif

//typedef int STDCALL INFO_DlgProp_Type(void* pDev, void* pServ, ULONG dlgMode, PVOID pSrvInfo); 
typedef int STDCALL INFO_InitPages_Type(void* pDev, DEVS_API_PropDlg* pPropDlg, void* pServ, ULONG* pNumDlg, PVOID pSrvInfo); 
typedef int STDCALL INFO_DeletePages_Type(ULONG numDlg); 
#ifdef _WIN32
typedef int STDCALL INFO_AddPages_Type(ULONG numDlg, HWND hParentWnd, ULONG viewMode, ULONG numPage, PVOID pPages); 
#endif
typedef int STDCALL INFO_SetProperty_Type(ULONG numDlg); 
typedef int STDCALL INFO_UpdateData_Type(ULONG numDlg, ULONG mode); 

//SRVINFO_API int STDCALL INFO_DialogProperty(void* pDev, void* pServ, ULONG dlgMode, PVOID pSrvInfo);
SRVINFO_API int STDCALL INFO_InitPages(void* pDev, DEVS_API_PropDlg* pPropDlg, void* pServ, ULONG* pNumDlg, PVOID pSrvInfo);
SRVINFO_API int STDCALL INFO_DeletePages(ULONG numDlg);
#ifdef _WIN32
SRVINFO_API int STDCALL INFO_AddPages(ULONG numDlg, HWND hParentWnd, ULONG viewMode, ULONG numPage, PVOID pPages);
#endif
SRVINFO_API int STDCALL INFO_SetProperty(ULONG numDlg);
SRVINFO_API int STDCALL INFO_UpdateData(ULONG numDlg, ULONG mode);

#ifdef	__cplusplus
};
#endif

#endif // _SDRAMSRVINFO_H

// ****************** End of file SdramSrvInfo.h ***************

