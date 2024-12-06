//****************** File AdcSrvInfo.h **********************************
//
//  Определения констант, структур и функций
//	для API диалога свойств службы ЦАП базового модуля
//
//  Constants, structures & functions definitions
//	for API Property Dialog of ADC service
//
//	Copyright (c) 2002-2003, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//
//  History:
//  14-04-04 - builded
//
//*******************************************************************************

#ifndef _ADCSRVINFO_H
 #define _ADCSRVINFO_H

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
#define BASE_MAXREFS 2
#define NAME_SIZE 64

#pragma pack(1)

// Struct info about device
typedef struct _ADCSRV_INFO {
	USHORT		Size;					// sizeof(ADCSRV_INFO)
	TCHAR		Name[NAME_SIZE];		// название службы и модуля
	UCHAR		Bits;					// разрядность
	UCHAR		Encoding;				// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
//	UCHAR		Code;					// 0 - two's complement, 1 - floating point, 2 - straight binary, 7 - Gray code
	ULONG		FifoSize;				// размер FIFO АЦП (в байтах = глубина * ширина)
//	USHORT		FifoDeep;				// глубина FIFO АЦП (в словах, равных ширине FIFO)
//	USHORT		FifoWidth;				// ширина FIFO АЦП (в байтах)
	UCHAR		ChanCnt;				// количество каналов АЦП (конфигурационный параметр)
	ULONG		ChanMask;				// маска выбранных каналов
	ULONG		SyncMode;				// режим синхронизации (Master/Single/Slave)
	double		SamplingRate;			// частота дискретизации
	ULONG		MinRate;				// мин. частота дискретизации
	ULONG		MaxRate;				// макс. частота дискретизации
	double		ClockValue;				// тактовая частота
	ULONG		ClockSrc;				// источник тактовой частоты
	ULONG		SysRefGen;				// system generator (значение системного опорного генератора (Гц))
	ULONG		BaseRefGen[BASE_MAXREFS];// frequency of generators (значения опорных генераторов (Гц))
	double		BaseExtClk;				// any external frequency of clock (любая из внешних частот тактирования (Гц))
	ULONG		RefPVS;					// Basic Voltage (опорное напряжение источников программируемых напряжений (мВольт))
	double		Range[MAX_CHAN];		// входной диапазон для каждого канала
	double		Bias[MAX_CHAN];			// смещение нуля для каждого канала
	UCHAR		Resist[MAX_CHAN];		// входное сопротивление для каждого канала
	UCHAR		DcCoupling[MAX_CHAN];	// открытость / закрытость входа по постоянной составляющей для каждого канала
	UCHAR		Format;					// 0 - 16 бит, 1 - 8 бит
	UCHAR		PretrigEnable;			// включение режима претриггера
	UCHAR		PretrigExernal;			// включение режима внешнего претриггера
	UCHAR		PretrigAssur;			// включение режима гарантированного претриггера
	USHORT		StartDelayCnt;			// счетчик начальной задержки
	USHORT		DaqCnt;					// счетчик принимаемых данных
	USHORT		SkipCnt;				// счетчик пропущенных данных
	UCHAR		Cnt0Enable;				// разрешение счетчика начальной задержки
	UCHAR		Cnt1Enable;				// разрешение счетчика принимаемых данных
	UCHAR		Cnt2Enable;				// разрешение счетчика пропущенных данных
	ULONG		StartSrc;				// источник старта
	UCHAR		StartInv;				// инверсия сигнала старта
	ULONG		StopSrc;				// источник останова
	UCHAR		StopInv;				// инверсия сигнала останова
	UCHAR		StartStopMode;			// 1 - режим триггерного старта-останова, 0 - режим потенциального старта-останова
	UCHAR		ReStart;				// 1 - режим сбора с автоматическим перезапуском
} ADCSRV_INFO, *PADCSRV_INFO;

#pragma pack()

// 0 - eng; 1 - rus
//static ULONG GetRegistryLocalParams()
//{
//	ULONG retcode = 0;
//	HKEY hkey;
//	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Control\\Nls\\Locale"), 0, KEY_QUERY_VALUE, &hkey);
//	DWORD Type = REG_SZ;
//	DWORD retSize = MAX_PATH;
//	TCHAR szBuffer[MAX_PATH];
//	if(RegQueryValueEx(hkey, _T(""), NULL, &Type, (LPBYTE)szBuffer, &retSize) == ERROR_SUCCESS)
//		if(!lstrcmpi(szBuffer, _T("00000419")))
//			retcode = 1;
//	RegCloseKey(hkey);
//	return retcode;
//}

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

#endif // _ADCSRVINFO_H

// ****************** End of file AdcSrvInfo.h ***************
