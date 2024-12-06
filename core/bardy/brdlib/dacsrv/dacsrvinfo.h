//****************** File DacSrvInfo.h **********************************
//
//  РћРїСЂРµРґРµР»РµРЅРёСЏ РєРѕРЅСЃС‚Р°РЅС‚, СЃС‚СЂСѓРєС‚СѓСЂ Рё С„СѓРЅРєС†РёР№
//	РґР»СЏ API РґРёР°Р»РѕРіР° СЃРІРѕР№СЃС‚РІ СЃР»СѓР¶Р±С‹ Р¦РђРџ
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
	TCHAR		Name[NAME_SIZE];		// РЅР°Р·РІР°РЅРёРµ СЃР»СѓР¶Р±С‹ Рё РјРѕРґСѓР»СЏ
	UCHAR		Bits;					// СЂР°Р·СЂСЏРґРЅРѕСЃС‚СЊ
	UCHAR		Encoding;				// С‚РёРї РєРѕРґРёСЂРѕРІРєРё (0 - РїСЂСЏРјРѕР№, 1 - РґРІРѕРёС‡РЅРѕ-РґРѕРїРѕР»РЅРёС‚РµР»СЊРЅС‹Р№, 2 - РєРѕРґ Р“СЂРµСЏ)
//	UCHAR		Code;					// 0 - two's complement, 1 - floating point, 2 - straight binary, 7 - Gray code
	ULONG		FifoSize;				// СЂР°Р·РјРµСЂ FIFO Р¦РђРџ (РІ Р±Р°Р№С‚Р°С…)
	UCHAR		ChanCnt;				// РєРѕР»РёС‡РµСЃС‚РІРѕ РєР°РЅР°Р»РѕРІ Р¦РђРџ (РєРѕРЅС„РёРіСѓСЂР°С†РёРѕРЅРЅС‹Р№ РїР°СЂР°РјРµС‚СЂ)
	ULONG		ChanMask;				// РјР°СЃРєР° РІС‹Р±СЂР°РЅРЅС‹С… РєР°РЅР°Р»РѕРІ
	ULONG		SyncMode;				// СЂРµР¶РёРј СЃРёРЅС…СЂРѕРЅРёР·Р°С†РёРё (Master/Single/Slave)
	double		SamplingRate;			// С‡Р°СЃС‚РѕС‚Р° РґРёСЃРєСЂРµС‚РёР·Р°С†РёРё
	ULONG		MinRate;				// РјРёРЅ. С‡Р°СЃС‚РѕС‚Р° РґРёСЃРєСЂРµС‚РёР·Р°С†РёРё
	ULONG		MaxRate;				// РјР°РєСЃ. С‡Р°СЃС‚РѕС‚Р° РґРёСЃРєСЂРµС‚РёР·Р°С†РёРё
	double		ClockValue;				// С‚Р°РєС‚РѕРІР°СЏ С‡Р°СЃС‚РѕС‚Р°
	ULONG		ClockSrc;				// РёСЃС‚РѕС‡РЅРёРє С‚Р°РєС‚РѕРІРѕР№ С‡Р°СЃС‚РѕС‚С‹
	ULONG		SysRefGen;				// system generator (Р·РЅР°С‡РµРЅРёРµ СЃРёСЃС‚РµРјРЅРѕРіРѕ РѕРїРѕСЂРЅРѕРіРѕ РіРµРЅРµСЂР°С‚РѕСЂР° (Р“С†))
	ULONG		BaseRefGen[2];			// frequency of generators (Р·РЅР°С‡РµРЅРёСЏ РѕРїРѕСЂРЅС‹С… РіРµРЅРµСЂР°С‚РѕСЂРѕРІ (Р“С†))
	double		BaseExtClk;				// any external frequency of clock (Р»СЋР±Р°СЏ РёР· РІРЅРµС€РЅРёС… С‡Р°СЃС‚РѕС‚ С‚Р°РєС‚РёСЂРѕРІР°РЅРёСЏ (Р“С†))
	double		Range[MAX_CHAN];		// РІС‹С…РѕРґРЅРѕР№ РґРёР°РїР°Р·РѕРЅ РґР»СЏ РєР°Р¶РґРѕРіРѕ РєР°РЅР°Р»Р°
	double		Bias[MAX_CHAN];			// СЃРјРµС‰РµРЅРёРµ РЅСѓР»СЏ РґР»СЏ РєР°Р¶РґРѕРіРѕ РєР°РЅР°Р»Р°
	UCHAR		Format;					// 0 - 16 Р±РёС‚, 1 - 8 Р±РёС‚
	USHORT		StartDelayCnt;			// СЃС‡РµС‚С‡РёРє РЅР°С‡Р°Р»СЊРЅРѕР№ Р·Р°РґРµСЂР¶РєРё
	USHORT		OutCnt;					// СЃС‡РµС‚С‡РёРє РІС‹РґР°РІР°РµРјС‹С… РґР°РЅРЅС‹С…
	USHORT		SkipCnt;				// СЃС‡РµС‚С‡РёРє РїСЂРѕРїСѓС‰РµРЅРЅС‹С… РґР°РЅРЅС‹С…
	UCHAR		Cnt0Enable;				// СЂР°Р·СЂРµС€РµРЅРёРµ СЃС‡РµС‚С‡РёРєР° РЅР°С‡Р°Р»СЊРЅРѕР№ Р·Р°РґРµСЂР¶РєРё
	UCHAR		Cnt1Enable;				// СЂР°Р·СЂРµС€РµРЅРёРµ СЃС‡РµС‚С‡РёРєР° РІС‹РґР°РІР°РµРјС‹С… РґР°РЅРЅС‹С…
	UCHAR		Cnt2Enable;				// СЂР°Р·СЂРµС€РµРЅРёРµ СЃС‡РµС‚С‡РёРєР° РїСЂРѕРїСѓС‰РµРЅРЅС‹С… РґР°РЅРЅС‹С…
	ULONG		StartSrc;				// РёСЃС‚РѕС‡РЅРёРє СЃС‚Р°СЂС‚Р°
	UCHAR		StartInv;				// РёРЅРІРµСЂСЃРёСЏ СЃРёРіРЅР°Р»Р° СЃС‚Р°СЂС‚Р°
	ULONG		StopSrc;				// РёСЃС‚РѕС‡РЅРёРє РѕСЃС‚Р°РЅРѕРІР°
	UCHAR		StopInv;				// РёРЅРІРµСЂСЃРёСЏ СЃРёРіРЅР°Р»Р° РѕСЃС‚Р°РЅРѕРІР°
	UCHAR		StartStopMode;			// 1 - СЂРµР¶РёРј С‚СЂРёРіРіРµСЂРЅРѕРіРѕ СЃС‚Р°СЂС‚Р°-РѕСЃС‚Р°РЅРѕРІР°, 0 - СЂРµР¶РёРј РїРѕС‚РµРЅС†РёР°Р»СЊРЅРѕРіРѕ СЃС‚Р°СЂС‚Р°-РѕСЃС‚Р°РЅРѕРІР°
} DACSRV_INFO, *PDACSRV_INFO;

#pragma pack()

// Functions Declaration

#ifdef	__cplusplus
extern "C" {
#endif

//typedef int __stdcall INFO_DlgProp_Type(void* pDev, void* pServ, ULONG dlgMode, PVOID pSrvInfo); 
typedef int STDCALL INFO_InitPages_Type(void* pDev, DEVS_API_PropDlg* pPropDlg, void* pServ, ULONG* pNumDlg, PVOID pSrvInfo);
typedef int STDCALL INFO_DeletePages_Type(ULONG numDlg);
#ifdef _WIN32
typedef int STDCALL INFO_AddPages_Type(ULONG numDlg, HWND hParentWnd, ULONG viewMode, ULONG numPage, PVOID pPages);
#endif
typedef int STDCALL INFO_SetProperty_Type(ULONG numDlg);
typedef int STDCALL INFO_UpdateData_Type(ULONG numDlg, ULONG mode);

//SRVINFO_API int __stdcall INFO_DialogProperty(void* pDev, void* pServ, ULONG dlgMode, PVOID pSrvInfo);
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
