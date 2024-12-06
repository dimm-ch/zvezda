//****************** File DdcSrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы DDC
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
	TCHAR		Name[NAME_SIZE];		// название службы и модуля
	UCHAR		Bits;					// разрядность
	UCHAR		Encoding;				// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
//	UCHAR		Code;					// 0 - two's complement, 1 - floating point, 2 - straight binary, 7 - Gray code
	ULONG		FifoSize;				// размер FIFO ЦАП (в байтах)
	UCHAR		ChanCnt;				// количество каналов ЦАП (конфигурационный параметр)
	ULONG		ChanMask;				// маска выбранных каналов
	ULONG		SyncMode;				// режим синхронизации (Master/Single/Slave)
	double		SamplingRate;			// частота дискретизации
	ULONG		MinRate;				// мин. частота дискретизации
	ULONG		MaxRate;				// макс. частота дискретизации
	double		ClockValue;				// тактовая частота
	ULONG		ClockSrc;				// источник тактовой частоты
	double		ExtClk;					// any external frequency of clock (любая из внешних частот тактирования (Гц))
	ULONG		StartSrc;				// источник старта
	UCHAR		StartInv;				// инверсия сигнала старта
	ULONG		StopSrc;				// источник останова
	UCHAR		StopInv;				// инверсия сигнала останова
	UCHAR		StartStopMode;			// 1 - режим триггерного старта-останова, 0 - режим потенциального старта-останова
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
