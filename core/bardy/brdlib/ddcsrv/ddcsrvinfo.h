//****************** File DdcSrvInfo.h **********************************
//
//  ����������� ��������, �������� � �������
//	��� API ������� ������� ������ DDC
//
//  Constants, structures & functions definitions
//	for API Property Dialog of DDC service
//
//	Copyright (c) 2002-2006, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  26-01-06 - builded
//
//*******************************************************************************

#ifndef _DDCSRVINFO_H
 #define _DDCSRVINFO_H

#ifdef _WIN32
    #ifdef SRVINFO_API_EXPORTS
     #define SRVINFO_API __declspec(dllexport)
    #else
     #define SRVINFO_API __declspec(dllimport)
    #endif
#else
    #define SRVINFO_API
#endif

//#define MAX_CHAN 4
#define NAME_SIZE 64

#pragma pack(1)

// Struct info about device
typedef struct _DDCSRV_INFO {
	USHORT		Size;					// sizeof(DACSRV_INFO)
	TCHAR		Name[NAME_SIZE];		// �������� ������ � ������
	UCHAR		Bits;					// �����������
	UCHAR		Encoding;				// ��� ��������� (0 - ������, 1 - �������-��������������, 2 - ��� ����)
//	UCHAR		Code;					// 0 - two's complement, 1 - floating point, 2 - straight binary, 7 - Gray code
	ULONG		FifoSize;				// ������ FIFO ��� (� ������)
	UCHAR		ChanCnt;				// ���������� ������� ��� (���������������� ��������)
	ULONG		ChanMask;				// ����� ��������� �������
	ULONG		SyncMode;				// ����� ������������� (Master/Single/Slave)
	double		SamplingRate;			// ������� �������������
	ULONG		MinRate;				// ���. ������� �������������
	ULONG		MaxRate;				// ����. ������� �������������
	double		ClockValue;				// �������� �������
	ULONG		ClockSrc;				// �������� �������� �������
	double		ExtClk;					// any external frequency of clock (����� �� ������� ������ ������������ (��))
	ULONG		StartSrc;				// �������� ������
	UCHAR		StartInv;				// �������� ������� ������
	ULONG		StopSrc;				// �������� ��������
	UCHAR		StopInv;				// �������� ������� ��������
	UCHAR		StartStopMode;			// 1 - ����� ����������� ������-��������, 0 - ����� �������������� ������-��������
} DDCSRV_INFO, *PDDCSRV_INFO;

#pragma pack()

// Functions Declaration

#ifdef	__cplusplus
extern "C" {
#endif

//typedef int __stdcall INFO_DlgProp_Type(void* pDev, void* pServ, ULONG dlgMode, PVOID pSrvInfo); 
typedef int STDCALL INFO_InitPages_Type(void* pDev, DEVS_API_PropDlg* pPropDlg, void* pServ, ULONG* pNumDlg, PVOID pSrvInfo);
typedef int STDCALL INFO_DeletePages_Type(ULONG numDlg);
#ifndef __linux__
typedef int STDCALL INFO_AddPages_Type(ULONG numDlg, HWND hParentWnd, ULONG viewMode, ULONG numPage, PVOID pPages);
#endif
typedef int STDCALL INFO_SetProperty_Type(ULONG numDlg);
typedef int STDCALL INFO_UpdateData_Type(ULONG numDlg, ULONG mode);

//SRVINFO_API int __stdcall INFO_DialogProperty(void* pDev, void* pServ, ULONG dlgMode, PVOID pSrvInfo);
SRVINFO_API int STDCALL INFO_InitPages(void* pDev, DEVS_API_PropDlg* pPropDlg, void* pServ, ULONG* pNumDlg, PVOID pSrvInfo);
SRVINFO_API int STDCALL INFO_DeletePages(ULONG numDlg);
#ifndef __linux__
SRVINFO_API int STDCALL INFO_AddPages(ULONG numDlg, HWND hParentWnd, ULONG viewMode, ULONG numPage, PVOID pPages);
#endif
SRVINFO_API int STDCALL INFO_SetProperty(ULONG numDlg);
SRVINFO_API int STDCALL INFO_UpdateData(ULONG numDlg, ULONG mode);

#ifdef	__cplusplus
};
#endif

#endif // _DDCSRVINFO_H

// ****************** End of file DdcSrvInfo.h ***************
