//****************** File DacSrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы ЦАП
//
//  Constants, structures & functions definitions
//	for API Property Dialog of DAC service
//
//	Copyright (c) 2002-2003, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  11-10-05 - builded
//
//*******************************************************************************

#ifndef _DACSRVINFO_H
 #define _DACSRVINFO_H

#ifdef _WIN32
	#ifdef SRVINFO_API_EXPORTS
	 #define SRVINFO_API __declspec(dllexport)
	#else
	 #define SRVINFO_API __declspec(dllimport)
	#endif
#else
    #define SRVINFO_API
#endif

#define MAX_CHAN 2
#define NAME_SIZE 64

#pragma pack(1)

// Struct info about device
typedef struct _DACSRV_INFO {
	USHORT		Size;					// sizeof(DACSRV_INFO)
	BRDCHAR		Name[NAME_SIZE];		// название службы и модуля
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
	ULONG		SysRefGen;				// system generator (значение системного опорного генератора (Гц))
	ULONG		BaseRefGen[2];			// frequency of generators (значения опорных генераторов (Гц))
	double		BaseExtClk;				// any external frequency of clock (любая из внешних частот тактирования (Гц))
	double		Range[MAX_CHAN];		// выходной диапазон для каждого канала
	double		Bias[MAX_CHAN];			// смещение нуля для каждого канала
	UCHAR		Format;					// 0 - 16 бит, 1 - 8 бит
	USHORT		StartDelayCnt;			// счетчик начальной задержки
	USHORT		OutCnt;					// счетчик выдаваемых данных
	USHORT		SkipCnt;				// счетчик пропущенных данных
	UCHAR		Cnt0Enable;				// разрешение счетчика начальной задержки
	UCHAR		Cnt1Enable;				// разрешение счетчика выдаваемых данных
	UCHAR		Cnt2Enable;				// разрешение счетчика пропущенных данных
	ULONG		StartSrc;				// источник старта
	UCHAR		StartInv;				// инверсия сигнала старта
	ULONG		StopSrc;				// источник останова
	UCHAR		StopInv;				// инверсия сигнала останова
	UCHAR		StartStopMode;			// 1 - режим триггерного старта-останова, 0 - режим потенциального старта-останова
} DACSRV_INFO, *PDACSRV_INFO;

#pragma pack()

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

#endif // _DACSRVINFO_H

// ****************** End of file DacSrvInfo.h ***************
