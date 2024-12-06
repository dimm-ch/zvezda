//****************** File ambInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств базового модуля
//
//  Constants, structures & functions definitions
//	for API Property Dialog of base module
//
//	Copyright (c) 2002-2003, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  14-04-04 - builded bambpinfo.h
//  27-12-06 - fixed ambinfo.h
//
//*******************************************************************************

#ifndef _AMBINFO_H
 #define _AMBINFO_H

#ifndef __linux__
	#ifdef AMBINFO_API_EXPORTS
	 #define AMBINFO_API __declspec(dllexport)
	#else
	 #define AMBINFO_API __declspec(dllimport)
	#endif
#else
    #define AMBINFO_API
#endif

#include "ctrladmpro.h"

#pragma pack(1)
/*
// Struct info about device
typedef struct _BAMBP_INFO {
	USHORT		Size;					// sizeof(BAMBP_INFO) + ...
	ULONG		PagesCnt;				// число страниц свойств
	HINSTANCE	hLib;					// дескриптор DLL с этими страницами
	PBRD_PropertyPage pPropPage;		// описание страниц свойств
} BAMBP_INFO, *PBAMBP_INFO;
*/
#pragma pack()

// Functions Declaration

#ifdef	__cplusplus
extern "C" {
#endif

typedef int STDCALL AMBINFO_DlgProp_Type(void* pDev, ULONG mode, ULONG numSrvDll, PVOID pSrvDll, BOOL* pChangeFlag, UCHAR* pCurPage); 
typedef int STDCALL AMBINFO_DelDlg_Type(void* pDev, ULONG mode, ULONG numSrvDll, PVOID pSrvDll, BOOL* pChangeFlag, UCHAR* pCurPage); 

AMBINFO_API int STDCALL AMBINFO_DialogProperty(void* pDev, ULONG mode, ULONG numSrvDll, PVOID pSrvDll, BOOL* pChangeFlag, UCHAR* pCurPage);
AMBINFO_API int STDCALL AMBINFO_DeleteDialog(void* pDev, ULONG mode, ULONG numSrvDll, PVOID pSrvDll, BOOL* pChangeFlag, UCHAR* pCurPage);
#ifndef __linux__
typedef int STDCALL INFO_AddPages_Type(ULONG numDlg, HWND hParentWnd, ULONG viewMode, ULONG numPage, PVOID pPages); 
#endif
typedef int STDCALL INFO_SetProperty_Type(ULONG numDlg); 
typedef int STDCALL INFO_UpdateData_Type(ULONG numDlg, ULONG mode); 
typedef int STDCALL INFO_DeletePages_Type(ULONG numDlg); 

#ifdef	__cplusplus
};
#endif

#endif // _AMBINFO_H

// ****************** End of file ambInfo.h ***************
