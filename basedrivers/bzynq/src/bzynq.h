//****************** File bambpex.h *******************************
// Definitions of structures, constants and class CBzynq
// for Data Acquisition Carrier Boards.
//
//	Copyright (c) 2007, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  20-04-07 - builded
//
//*******************************************************************

#ifndef _BZYNQ_H_
#define _BZYNQ_H_

#include	"gipcy.h"
#include	"brderr.h"
#include	"brdpu.h"
#include	"brdapi.h"

#include	"extn.h"
#include	"extn_andor.h"
#include	"basemodule.h"
#include	"adm2if.h"
#include	"mainregs.h"
#include	"ambinfo.h"
#include	"icr.h"
#include	"IcrAmbpex.h"
#include	"IcrAdp201x1.h"
#include	"Icrfmc105p.h"
#include	"icrxm416x250m.h"
#include	"Icrfmc112cp.h"
#include	"Icrfmc127p.h"

#include	"pld.h"
#include	"ddzynq.h"

#include	"sysmonsrv.h"
#include	"testsrv.h"
#include	"streamsrv.h"
#include	"sdramambpcdsrv.h"
#include	"ddr3sdramsrv.h"
#include	"ddr4sdramsrv.h"

#include	"resource.h"		// main symbols

#include	<vector>

using namespace std;

// Constants
#define	VER_MAJOR		0x00010000L
#define	VER_MINOR		0x00000000L

#define MAX_DESCRIPLEN 128

#define MAX_STREAM_SRV 2
#define MAX_TEST_SRV 1
#define MAX_SYSMON_SRV 1
#define MAX_MEMORY_SRV 2

#define MAX_MEM_INI 2

#define ICAP_SIG 0x2482	// сигнатура узла ICAP, используемого для частичной перезагрузки ПЛИС

//const U16 MAX_BASE_CFGMEM_SIZE = 384; // размер под ICR, выделенный в ППЗУ, устанавливаемом на базовый модуль
const U16 MAX_BASE_CFGMEM_SIZE = 256; // размер под ICR, выделенный в ППЗУ, устанавливаемом на базовый модуль
const U16 OFFSET_BASE_CFGMEM = 128; // смещение для ICR в ППЗУ, устанавливаемом на базовый модуль

#pragma pack(push,1)

typedef struct _MEM_INI {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	U32		SlotCnt;		// количество установленных слотов
	U32		ModuleCnt;		// количество установленных DIMM-модулей
	U32		ModuleBanks;	// количество банков в DIMM-модуле
	U32		RowAddrBits;	// количество разрядов адреса строк
	U32		ColAddrBits;	// количество разрядов адреса столбцов
	U32		ChipBanks;		// количество банков в микросхемах
	U32		PrimWidth;		// Primary DDR SDRAM Width
	U32		CasLatency;		// задержка сигнала выборки по столбцам
	U32		Attributes;		// атрибуты DIMM-модуля
	U32		TetrNum;		// номер тетрады (перебивает ID тетрады)
	U32		AdrMask;		// маска разрядов адреса памяти (биты 0..9 соответствуют адресным битам 20..29)
	U32		AdrConst;		// значение разрядов адреса памяти  (биты 0..9 соответствуют адресным битам 20..29)
} MEM_INI, *PMEM_INI;

typedef struct _DSP_INI {
	U32		PldType;		// type of PLD (0-EP1K,1-EP1KA,2-EP10KE,...) (серия(тип) ПЛИС)
	U32		PldVolume;		// volume of PLD (объем ПЛИС)
	U32		PldPins;		// pins counter of PLD (число выводов)
	U32		PldSpeedGrade; // быстродействие 1,2,3,...
	U32		PioType;		// type of PIO (0-non, 1-TTL, 2-LVDS)
} DSP_INI, *PDSP_INI;

typedef struct _BASE_INI {
	U32		PID;			// серийный (физический) номер
	U32		OrderNumber;	// порядковый номер ПЛИС(для модулей CompactPCI, таких как FMC115cP)
	U32		BusNumber;		// номер шины (Bus Number)
	U32		DevNumber;		// номер устройства (Device Number)
	U32		SlotNumber;		// номер слота (Slot Number)
	U32		SysGen;			// значение частоты системного генератора в Гц
	MEM_INI MemIni[MAX_MEM_INI];// параметры инициализации памяти
	DSP_INI DspIni;			// параметры инициализации узла ЦОС
	U32		FmcPower;		// флаг включения питания FMC
} BASE_INI, *PBASE_INI;	

typedef struct _PU_Info {
	U32		Id;							// Programmable Unit ID
	U32		Type;						// Programmable Unit Type
	U32		Attr;						// Programmable Unit Attribute
	BRDCHAR	Description[MAX_DESCRIPLEN];// Programmable Unit Description Text
} PU_Info, *PPU_Info;

typedef struct _DDR3_CFG {
	U08		CfgSrc;			// откуда брать конфигурацию: 0 и 1 - из ICR, 2 - из SPD 
	U08		ModuleCnt;		// количество установленных SODIMM-модулей
	U08		ModuleBanks;	// количество банков в SODIMM-модуле
	U08		RowAddrBits;	// количество разрядов адреса строк
	U08		ColAddrBits;	// количество разрядов адреса столбцов
	U08		ChipBanks;		// количество банков в микросхемах
	U08		PrimWidth;		// ширина шины SODIMM-модуля (количество разрядов в SODIMM-модуле) : 8,16,32,64
	U08		ChipWidth;		// ширина шины микросхемы (количество разрядов в микросхемах) : 4,8,16,32
	U64		CapacityMbits;  // объём модуля в мегабитах : 256,512,1024,2048,4096,8192,16384
} DDR3_CFG, *PDDR3_CFG;

typedef struct _MEM_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	U08		SlotCnt;		// количество установленных слотов
	U08		ModuleCnt;		// количество установленных DIMM-модулей
	U08		ModuleBanks;	// количество банков в DIMM-модуле
	U08		RowAddrBits;	// количество разрядов адреса строк
	U08		ColAddrBits;	// количество разрядов адреса столбцов
	U08		ChipBanks;		// количество банков в микросхемах
	U08		PrimWidth;		// Primary DDR SDRAM Width
	U08		CasLat;			// задержка сигнала выборки по столбцам
	U08		Attributes;		// атрибуты DIMM-модуля
	S08		TetrNum;		// номер тетрады (перебивает ID тетрады)
	U16		AdrMask;		// маска разрядов адреса памяти (биты 0..9 соответствуют адресным битам 20..29)
	U16		AdrConst;		// значение разрядов адреса памяти  (биты 0..9 соответствуют адресным битам 20..29)
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
	0x20,		// Attributes
	-1,			// tetrada number
	0,
	0
};

typedef struct _DSPNODE_CFG {
	U08		PldType;			// type of PLD (0-EP1K,1-EP1KA,2-EP10KE,...) (серия(тип) ПЛИС)
	U16		PldVolume;			// volume of PLD (объем ПЛИС)
	U16		PldPins;			// pins counter of PLD (число выводов)
	U08		PldSpeedGrade;		// быстродействие 1,2,3,...
	U08		PioType;			// type of PIO (0-non, 1-TTL, 2-LVDS)
} DSPNODE_CFG, *PDSPNODE_CFG;

const DSPNODE_CFG DspNodeCfg_dflt = {
	22,
	240,
	1156,
	0,
	0,	// PIO - NON
};

// Configuration register (00h)
typedef union _INA219_CONFREG {
	U32 EnBlock; // Configuration register en block
	struct { // Configuration register by Fields
		U32 MODE : 3; // 2..0
		U32 SADC : 4; // 6..3
		U32 BADC : 4; // 10..7
		U32 PG : 2; // 12..11
		U32 BRNG : 1; // 13
		U32 nullValBit : 1; // 14
		U32 RST : 1; // 15	
		U32 space : 16; // остаток
	} ByFields;
} INA219_CONFREG, *PINA219_CONFREG;

// Bus Voltage register (02h)
typedef union _INA219_VOLTREG {
	U32 EnBlock; // Configuration register en block
	struct { // Configuration register by Fields
		U32 OVR : 1; // 0
		U32 CNVR : 1;	// 1
		U32 nullValBit : 1;	// 2
		U32 BVR : 13;	// 15..3
		U32 space : 16; // остаток
	} ByFields;
} INA219_VOLTREG, *PINA219_VOLTREG;

#pragma pack(pop)

// Base Module Class definition
class CBzynq : public CBaseModule
{

protected:

	IPC_handle	m_hMutex;			// Mutex for multithread operation
	int  DevIoCtl(IPC_handle hDev,
				 DWORD code, 
				 LPVOID InBuf = 0, 
				 DWORD InBufSize = 0, 
				 LPVOID OutBuf = 0, 
				 DWORD OutBufSize = 0, 
				 DWORD* pRet = 0, 
				 LPOVERLAPPED lpOverlapped = 0);

	PBASE_INI	m_pIniData;		// Data from Registry or INI File

//	U16		m_DeviceID;			// PCI Device Identifier
//	U08		m_RevisionID;		// PCI Revision Identifier

	BRDCHAR	m_pDlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	
//	U32		m_PID;				// Physical Identifier (Serial Number) // CModule
	U32		m_BusNum;			// Bus Number
	U32		m_DevNum;			// Device Number
	U32		m_SlotNum;			// Slot Number
	U32		m_SysGen;			// System Generator value (Hz)
	MEM_CFG	m_MemCfg[MAX_MEMORY_SRV]; // SDRAM configuration data
	DDR3_CFG m_Ddr3Cfg;			 // DDR3 configuration data
	DSPNODE_CFG	m_DspCfg;		// DSP node configuration data
	
	//	U32		m_puCnt;			// Programmable Unit Count // CModule
	PU_Info m_AdmPldInfo;		// ADM PLD info
	PU_Info m_DspPldInfo;		// DSP PLD info
	PU_Info m_BaseIcrInfo;		// base EEPROM info
	PU_Info m_AdmIcrInfo;		// subunit ICR EEPROM info
	PULONG	m_pMemBuf[2];		// виртуальные адреса аппаратных областей
	U32		m_PhysMem[2];		// физические адреса аппаратных областей
	ULONG m_BlockFidAddr;		// смещение относительно m_pMemBuf[0], по которому находится блок FID (если = 0, то нет блока FID)
	ULONG m_BlockFifoAddr[4];	// смещения относительно m_pMemBuf[0], по которым находятся блоки EXT_FIFO
	PU_Info m_AdmFruInfo;		// subunit FRUID EEPROM info
	UCHAR	m_pldType;			// ADM PLD type (save for SYSMON)
	UCHAR	m_pldSpeedGrade;	// ADM PLD speed grade (save for SYSMON)
	USHORT	m_IcapSig;			// to save the signature of ICAP = 0x2482
	PU_Info m_PartPldInfo;		// Partition PLD info
	ULONG GetBaseIcrDataSize();

	vector <PU_Info> m_PuInfos;
	void GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld);
	ULONG GetPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);

	BRDextn_PLDFILEINFO m_pldFileInfo;	// информация о файле-прошивке, загруженной в ПЛИС
	void GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const;
	BRDextn_PLDFILEINFO m_FdspFileInfo;	// информация о файле-прошивке, загруженной в ПЛИС ЦОС

	IPC_handle	m_hWDM;				// WDM Driver Handle

	//ULONG GetVersion(PTSTR pVerInfo);
	ULONG GetVersion(char* pVerInfo);
	ULONG GetLocation(PAMB_LOCATION pAmbLocation);
	ULONG GetConfiguration(PAMB_CONFIGURATION pAmbConfiguration);
	ULONG GetDeviceID(USHORT& DeviceID);
	ULONG GetRevisionID(UCHAR& RevisionID);
	ULONG GetPid();

	AMB_CONFIGURATION m_AmbConfiguration; //Configuration info
	ULONG GetMemoryAddress();
	ULONG MapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar);
	ULONG UnmapDeviceMemory(AMB_CONFIGURATION& AmbConfiguration, int bar);

	ULONG ReadPldFile(const BRDCHAR* PldFileName, UCHAR*& fileBuffer, ULONG& fileSize);
	ULONG CheckPldFile(const BRDCHAR* PldFileName);
	ULONG LoadPldFile(const BRDCHAR* PldFileName, ULONG PldNum);
	void* SearchInfoStart(UCHAR* fileBuffer, ULONG& fileSize);
	ULONG isFdsp();
	long m_FdspTetrNum;
	//ULONG GetFdspPldStatus();
	ULONG DspPldConnect();
	ULONG AdmPldStartTest();
	ULONG AdmPldWorkAndCheck();
	ULONG SetAdmPldFileInfo();
	ULONG SetDspPldFileInfo();
	
	ULONG GenReset();
	ULONG AllReset();
	ULONG Unreset();

	ULONG LoadBitPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize);
	ULONG LoadPartPld(UCHAR PartPldNum, UCHAR& PldStatus, UCHAR* pBuffer, ULONG BufferSize);
	ULONG LoadPld(const BRDCHAR* PldFileName, UCHAR& PldStatus);
	ULONG SetPldStatus(ULONG PldStatus, ULONG PldNum);
	ULONG GetPldStatus(ULONG& PldStatus, ULONG PldNum);
	ULONG GetBaseIcrStatus(ULONG& Status);
	ULONG GetAdmIcrStatus(ULONG& Status, int idxSub); // определяет не просто наличие, а то что прописана информацией

	ULONG SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize); // override function of basemodule 
//	void GetAmbpCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbp pCfgBase);
	void GetAmbpexCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpex pCfgBase);
	void GetAdp201x1CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdp201x1 pCfgBase);
	void GetFmcCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc105p pCfgBase);
	void GetXm5516CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_Cfg5516 pCfgBase);
	void GetFmc551FCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_Cfg551F pCfgBase);
	void GetDdr3CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdr3 pCfgDdr3);
	ULONG GetAdmIfCntICR(int& AdmIfCnt);
//	void GetSdramCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdrSdram pCfgSdram);
	ULONG SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	void SetSdramfromIniFile(PBASE_INI pIniData);
	ULONG GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize); // override function of basemodule
	ULONG SetSubEeprom(int idxSub, ULONG& SubCfgSize);

	ULONG GetDspPld(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize, UCHAR& DspPldCnt);
	ULONG GetDspPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	void GetDspNodeCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDspNode pCfgDspNode);
	ULONG SetDspNodeConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);

	CSdramSrv*	m_pMemorySrv[MAX_MEMORY_SRV];
	CTestSrv*	m_pTestSrv[MAX_TEST_SRV];
	CSysMonSrv*	m_pSysMonSrv[MAX_SYSMON_SRV];
	CStreamSrv*	 m_pStreamSrv[MAX_STREAM_SRV];
	ULONG SetServices();
	void DeleteServices();

	ULONG WriteRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG WriteRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG ReadRegData(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG ReadRegDataDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, U32& RegVal);
	ULONG WriteRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
	ULONG WriteRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
	ULONG ReadRegBuf(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);
	ULONG ReadRegBufDir(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);

	ULONG ReadMemDataDir(ULONG* pDataReg, ULONG* pStatusReg, ULONG& Value);
	ULONG WriteMemDataDir(ULONG* pDataReg, ULONG* pStatusReg, ULONG Value);
	ULONG ReadMemData(ULONG* pCmdReg, ULONG* pDataReg, ULONG* pStatusReg, ULONG RegNumber, ULONG& Value);
	ULONG WriteMemData(ULONG* pCmdReg, ULONG* pDataReg, ULONG* pStatusReg, ULONG RegNumber, ULONG Value);

	ULONG SetMemory(PVOID pParam, ULONG sizeParam);
	ULONG FreeMemory(PVOID pParam, ULONG sizeParam);
	ULONG StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StopMemory(PVOID pParam, ULONG sizeParam);
	ULONG StateMemory(PVOID pParam, ULONG sizeParam);
	ULONG SetDirection(PVOID pParam, ULONG sizeParam);
	ULONG SetSource(PVOID pParam, ULONG sizeParam);
	ULONG SetDmaRequest(PVOID pParam, ULONG sizeParam);
	ULONG ResetFifo(PVOID pParam, ULONG sizeParam);
	ULONG Adjust(PVOID pParam, ULONG sizeParam);
	ULONG Done(PVOID pParam, ULONG sizeParam);
	ULONG GetDmaChanInfo(PVOID pParam, ULONG sizeParam);
	ULONG SetIrqMode(PVOID pParam, ULONG sizeParam);
#ifdef __linux__
	ULONG WaitDmaBlock(void* pParam, ULONG sizeParam);
	ULONG WaitDmaBuffer(void* pParam, ULONG sizeParam);
    ULONG WaitStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent);
#endif
	ULONG SetStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent);
	ULONG ClearStatusIrq(ULONG AdmNum, ULONG TetrNum, ULONG IrqMask, ULONG IrqInv, HANDLE hEvent);

    const BRDCHAR* getClassName(void) { return _BRDC("CBzynq"); }

public:

    CBzynq();
    CBzynq(PBRD_InitData pInitData, long sizeInitData);
    ~CBzynq();

//	void* operator new (size_t size) {return HeapAlloc( GetProcessHeap(), 0, size ); }
//	void operator delete (void* p) {if(p) HeapFree( GetProcessHeap(), 0, p ); }

	//S32 Init(short DeviceNumber, PTSTR pBoardName, PTSTR pIniString = NULL);
	S32 Init(short DeviceNumber, BRDCHAR* pBoardName, BRDCHAR* pIniString = NULL);
	void CleanUp();

	ULONG HardwareInit();

	ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG BaseIcrCorrectError();
	ULONG WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	virtual ULONG ReadSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG WriteSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset);


	virtual ULONG ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	void GetDeviceInfo(BRD_Info* info) const;
	void GetPuList(BRD_PuList* list) const;

	ULONG GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo);
	ULONG ShowDialog(PBRDextn_PropertyDlg pDlg);
	ULONG DeleteDialog(PBRDextn_PropertyDlg pDlg);

	ULONG GetPldInfo(PBRDextn_PLDINFO pInfo);
	virtual ULONG GetPower(PBRDextn_FMCPOWER pow);
	virtual ULONG PowerOnOff(PBRDextn_FMCPOWER pow, int force);
	virtual ULONG InitSensors();
    //virtual ULONG GetSensors(PBRDextn_Sensors sens);
    virtual ULONG GetSensors(void *sens);

	ULONG WaitFidReady(ULONG* BlockMem);
	ULONG ReadFidSpd(int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length);
	ULONG WriteFidSpd(int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length);
	virtual ULONG ReadFmcExt(PBRDextn_FMCEXT ext);
	virtual ULONG WriteFmcExt(PBRDextn_FMCEXT ext);
	virtual ULONG ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte);
	virtual ULONG WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte);

	virtual ULONG SetIrqMinTime(ULONG* pTimeVal);
	virtual ULONG GetIrqMinTime(ULONG* pTimeVal);

//	inline ULONG GetPuCnt() {return m_puCnt;}; // CModule
//	ULONG PuFileLoad(ULONG id, PCTSTR fileName, PUINT pState);
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

#endif	// _BZYNQ_H_

// ****************** End of file bambpex.h **********************
