//****************** File bambpci.h *******************************
// Definitions of structures, constants and class CBambpci
// for Data Acquisition Carrier Boards.
//
//	Copyright (c) 2004, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  05-10-04 - builded
//  30-08-05 - add CTestSrv
//
//*******************************************************************

#ifndef _BAMBPCI_H_
 #define _BAMBPCI_H_

#include 	<type_traits>
#include	"brderr.h"
#include	"brdpu.h"
#include	"brdapi.h"

#include	"extn.h"
#include	"extn_andor.h"
//#include	"useful.h"
#include	"basemodule.h"
//#include	"module.h"
#include	"adm2if.h"
#include	"mainregs.h"
#include	"bambpini.h"
//#include	"bambpinfo.h"
#include	"ambinfo.h"
//#include	"ambini.h"
#include	"icr.h"
#include	"IcrAmbp.h"
#include	"IcrAmbpex.h"
#include	"IcrC520F1.h"
#include	"pld.h"
#include	"ddwambp.h"

#include	"sdramambpcdsrv.h"
#include	"sdramambpcxsrv.h"
//#include	"SdramSrv.h"
#include	"sysmonsrv.h"
#include	"testsrv.h"
#include	"streamsrv.h"

#include "bambpMsg.h"

#include "resource.h"		// main symbols

//#include	<string>
//#include	<set>
//#include	<list>
#include	<vector>

using namespace std;

// Constants
#define	VER_MAJOR		0x00010000L
#define	VER_MINOR		0x00000000L

#define MAX_DESCRIPLEN 128

#define MAX_STREAM_SRV 2
#define MAX_MEMORY_SRV 2
#define MAX_DSPNODE_SRV 2
#define MAX_SYSMON_SRV 1
#define MAX_TEST_SRV 1

const U16 MAX_BASE_CFGMEM_SIZE = 384; // размер под ICR, выделенный в ППЗУ, устанавливаемом на базовый модуль
const U16 OFFSET_BASE_CFGMEM = 128; // смещение для ICR в ППЗУ, устанавливаемом на базовый модуль

#pragma pack(push,1)

typedef struct _PU_Info {
	U32		Id;							// Programmable Unit ID
	U32		Type;						// Programmable Unit Type
	U32		Attr;						// Programmable Unit Attribute
	TCHAR	Description[MAX_DESCRIPLEN];// Programmable Unit Description Text
} PU_Info, *PPU_Info;

typedef struct _MEM_CFG {
	TCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	U08		SlotCnt;		// количество установленных слотов
	U08		ModuleCnt;		// количество установленных DIMM-модулей
	U08		ModuleBanks;	// количество банков в DIMM-модуле
	U08		RowAddrBits;	// количество разрядов адреса строк
	U08		ColAddrBits;	// количество разрядов адреса столбцов
	U08		ChipBanks;		// количество банков в микросхемах
	U08		PrimWidth;		// Primary DDR SDRAM Width
	U08		CasLat;			// задержка сигнала выборки по столбцам
	U08		Attributes;		// атрибуты DIMM-модуля
} MEM_CFG, *PMEM_CFG;

const MEM_CFG MemCfg_dflt = {
	_BRDC("sdramsrvdlg"), // dialog dll name
	0,			// slot number
	0,			// DIMM number
	2,			// DIMM banks
	13,			// row address bits
	9,			// column address bits
	4,			// chip banks
	4,			// width = x4
	2,			// CAS latency
	0x20		// Attributes
};

typedef struct _DSPNODE_CFG {
	U08		PldType;			// type of PLD (0-EP1K,1-EP1KA,2-EP10KE,...) (серия(тип) ПЛИС)
	U16		PldVolume;			// volume of PLD (объем ПЛИС)
	U16		PldPins;			// pins counter of PLD (число выводов)
	U08		PldSpeedGrade;		// быстродействие 1,2,3,...
	U08		LoadRom;			// Is loading ROM (наличие загрузочного ПЗУ: 0 - нет, 1 - есть, 0xFF - снимаемое)
	U08		PioType;			// type of PIO (0-non, 1-TTL, 2-LVDS)
	U08		SramChipCnt;		// количество установленных микросхем
	U08		SramChipDepth;		// Depth of Chip (глубина (размер) микросхемы (в словах))
	U08		SramChipBitsWidth;	// Width of Chip (ширина микросхемы (число бит в слове))
} DSPNODE_CFG, *PDSPNODE_CFG;

const DSPNODE_CFG DspNodeCfg_dflt = {
	4,
	1000,
	896,
	6,
	0,	// loading ROM - NON
	0,	// PIO - NON
	0,	// SRAM chips - 0
	0,
	0
};

#pragma pack(pop)

// Base Module Class definition
class CBambpci : public CBaseModule
{

protected:

	PBASE_INI	m_pIniData;		// Data from Registry or INI File

	BRDCHAR	m_DeviceName[32];		// имя драйвера устройства

	BRDCHAR	m_pDlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	
//	U32		m_PID;				// Physical Identifier (Serial Number) // CModule
	U32		m_BusNum;			// Bus Number
	U32		m_DevNum;			// Device Number
	U32		m_SlotNum;			// Slot Number
	U32		m_SysGen;			// System Generator value (Hz)
	MEM_CFG	m_MemCfg[MAX_MEMORY_SRV]; // SDRAM configuration data
	DSPNODE_CFG	m_DspCfg[MAX_DSPNODE_SRV]; // DSP node configuration data
//	BASE_CFG m_AmbCfg[MAX_ADMIF];// AMB configuration
//	int		m_AdmIfCfgNum;
//	ADMIF_CFG m_AdmIfCfg[MAX_ADMIF];// 

	U32 m_pSDRAM;
	//	U32		m_puCnt;			// Programmable Unit Count // CModule
	PU_Info m_AdmPldInfo;		// ADM PLD info
	PU_Info m_DspPldInfo;		// DSP PLD info
	PU_Info m_BaseIcrInfo;		// base EEPROM info
	PU_Info m_AdmIcrInfo;		// subunit EEPROM info
	UCHAR	m_pldType;			// ADM PLD type (save for SYSMON)

	ULONG GetBaseIcrDataSize();

	vector <PU_Info> m_PuInfos;
	void GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld);
//	ULONG GetPldDescription(TCHAR* Description);
//	ULONG GetDspPld(UCHAR& DspPldCnt);
	ULONG GetPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetDspPld(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize, UCHAR& DspPldCnt);
	ULONG GetDspPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);

	BRDextn_PLDFILEINFO m_pldFileInfo;	// информация о файле-прошивке, загруженной в ПЛИС
	void GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const;

	IPC_handle	m_hWDM;				// WDM Driver Handle

	int  DevIoCtl(IPC_handle hDev,
				 DWORD code, 
				 LPVOID InBuf = 0, 
				 DWORD InBufSize = 0, 
				 LPVOID OutBuf = 0, 
				 DWORD OutBufSize = 0, 
				 DWORD* pRet = 0, 
				 LPOVERLAPPED lpOverlapped = 0);

//	int		m_SubunitCnt;		// количество субмодулей
//	SIDEDLL_Info	m_Subunit[MAX_ADMIF];	// Total Subunit Information

	//ULONG OpenDevice(int DevNum);
	void CloseDevice();
	//ULONG GetVersion(PTSTR pVerInfo);
	ULONG GetVersion(char* pVerInfo);
	ULONG GetLocation(PAMB_LOCATION pAmbLocation);
	ULONG GetConfiguration(PAMB_CONFIGURATION pAmbConfiguration);
	ULONG GetDeviceID(USHORT& DeviceID);
	ULONG GetRevisionID(UCHAR& RevisionID);
	ULONG GetPid();

    ULONG ReadPldFile(const BRDCHAR* PldFileName, UCHAR*& fileBuffer, DWORD& fileSize);
    ULONG LoadPldFile(const BRDCHAR* PldFileName, ULONG PldNum);
	ULONG AdmPldStartTest();
	ULONG AdmPldWorkAndCheck();
	ULONG SetAdmPldFileInfo();
	ULONG DspPldWorkAndCheck();
	
	//ULONG GetMemoryAddress();
	//ULONG CheckPldFile(PCTSTR PldFileName);
	//ULONG LoadJamPld(PCTSTR PldFileName);
	ULONG LoadAcePld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum);
	ULONG LoadPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum);
	ULONG LoadPldRom(UCHAR& PldStatus);
	ULONG GetPldStatus(ULONG& PldStatus, ULONG PldNum);
	ULONG GetBaseIcrStatus(ULONG& Status);
	ULONG GetAdmIcrStatus(ULONG& Status); // определяет не просто наличие, а то что прописана информацией

	ULONG SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize); // override function of basemodule 
	template<typename PICR_CfgAmbpType>
	void GetAmbpCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpType pCfgBase);
	void GetAmbpexCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpex pCfgBase);
	ULONG GetAdmIfCntICR(int& AdmIfCnt);
	void GetSdramCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdrSdram pCfgSdram);
	ULONG SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize); // override function of basemodule
	ULONG SetSubEeprom(int idxSub, ULONG& SubCfgSize);

	void GetDspNodeCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDspNode pCfgDspNode, PICR_CfgSram pCfgSram);
	ULONG SetDspNodeConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);

	CTestSrv*	m_pTestSrv[MAX_TEST_SRV];
	CSdramSrv*	m_pMemorySrv[MAX_MEMORY_SRV];
//	CSdramAmbpcdSrv*	m_pMemorySrv[MAX_MEMORY_SRV];
	CSysMonSrv*	m_pSysMonSrv[MAX_SYSMON_SRV];
	CStreamSrv*	 m_pStreamSrv[MAX_STREAM_SRV];
	ULONG SetServices();
	void DeleteServices();

//	ULONG AllocMemForMainRegs();
//	ULONG FreeMemForMainRegs();

	ULONG WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
	ULONG WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
	ULONG ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
	ULONG ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);

	ULONG SetMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG FreeMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StopMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StateMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG SetDirection(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG SetSource(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG SetDmaRequest(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);

	ULONG SetStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent);
	ULONG ClearStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent);

#ifdef __linux__
    ULONG WaitDmaBlock(void* pParam, ULONG sizeParam);
    ULONG WaitDmaBuffer(void* pParam, ULONG sizeParam);
#endif


	const BRDCHAR* getClassName(void) { return _BRDC("CBambpci"); }

public:

//	CBambpci() { memset(this, 0, sizeof(CBplx)); } 
	CBambpci(); 
	CBambpci(PBRD_InitData pInitData, long sizeInitData);
	~CBambpci();

//	void* operator new (size_t size) {return HeapAlloc( GetProcessHeap(), 0, size ); }
//	void operator delete (void* p) {if(p) HeapFree( GetProcessHeap(), 0, p ); }

//	S32 Init(PBASE_INI pIniData, U16 DeviceNumber, PTSTR pBoardName, PTSTR pIniString = NULL);
	S32 Init(short DeviceNumber, BRDCHAR* pBoardName, BRDCHAR* pIniString = NULL);
	void CleanUp();
//	S32 SubunitInit(PTSTR pLibName, int idxSubDriver, PBRD_InitData pSubInitData, S32 subInitDataSize);
//	S32 BaseDllInit(PTSTR pLibName, int idxSubunit, void* pSubInitData, S32 subInitDataSize, int Auto);
//	S32 SubunitInit(PTSTR pLibName, int idxSubunit, void* pSubInitData, S32 subInitDataSize, int Auto);
//	int GetAdmIfCntAuto() { return m_AdmIfCnt;};
//	int GetSubunitCntAuto() { return m_SubunitCnt;};
//	S32 GetSubunitTypeAuto(int idxSubunit, ULONG& type);

	ULONG HardwareInit();

	ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	void GetDeviceInfo(BRD_Info* info) const;

	ULONG GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo);
	ULONG ShowDialog(PBRDextn_PropertyDlg pDlg);
	ULONG DeleteDialog(PBRDextn_PropertyDlg pDlg);

	ULONG GetPldInfo(PBRDextn_PLDINFO pInfo);

//	inline ULONG GetPuCnt() {return m_puCnt;}; // CModule
	void GetPuList(BRD_PuList* list) const;
	ULONG PuFileLoad(ULONG id, const BRDCHAR* fileName, PUINT pState);
	ULONG GetPuState(ULONG id, PUINT pState);

//	inline ULONG GetSrvCnt() {return m_SrvList.size();}; // CModule
//	ULONG GetSrvCnt() const;
//	void GetSrvList(PSERV_LIST_ITEM list); // CModule also
	void SetCandidateSrv();
//	ULONG SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext); // CModule also

	LONG RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam); // CModule also

	ULONG StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);

};

template<typename PICR_CfgAmbpType>
void CBambpci::GetAmbpCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpType pCfgBase)
{
	PVOID pEndCfgMem = (PVOID)((PUCHAR)pCfgMem + RealBaseCfgSize);
	int end_flag = 0;
	do
	{
		USHORT sign = *((USHORT*)pCfgMem);
		USHORT size = 0;
		if(sign == BASE_ID_TAG)
		{
			PICR_IdBase pBaseId = (PICR_IdBase)pCfgMem;
			ICR_IdBase	idBase;
			memcpy(&idBase, pBaseId, sizeof(ICR_IdBase));
			//RealBaseCfgSize = idBase.wSizeAll;
			PUCHAR pBaseCfg = (PUCHAR)pCfgMem + sizeof(ICR_IdBase);
			size = sizeof(ICR_IdBase);
			PICR_CfgAmbp pBambpCfg = (PICR_CfgAmbp)pBaseCfg;
			pBambpCfg->bAdmIfCnt = 1;
			if (std::is_same<PICR_CfgAmbpType, ICR_CfgC520F1>::value) {
				memcpy(pCfgBase, pBambpCfg, pBambpCfg->wSize + 4);
				size += (pBambpCfg->wSize + 4);
			} else
			{
				memcpy(pCfgBase, pBambpCfg, sizeof(ICR_CfgAmbp));
				size += sizeof(ICR_CfgAmbp);
			}
			end_flag = 1;
		}
		else
		{
			size = *((USHORT*)pCfgMem + 1);
			size += 4;
//			break;
		}
		pCfgMem = (PUCHAR)pCfgMem + size;
	} while(!end_flag && pCfgMem < pEndCfgMem);
}

#endif	// _BAMBPCI_H_

// ****************** End of file bambpci.h **********************
