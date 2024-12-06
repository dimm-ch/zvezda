//****************** File baxizync.h *******************************
// Definitions of structures, constants and class CBaxizynq
// for Data Acquisition Carrier Boards.
//
//	Copyright (c) 2020, Instrumental Systems,Corp.
//
//  History:
//  20-04-20 - builded
//
//*******************************************************************

#ifndef _BAXIZYNQ_H_
#define _BAXIZYNQ_H_

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
#include	"Icrfmc105p.h"
//#include	"Icrfmc130e.h"

#include	"pld.h"

#include	"sysmonsrv.h"
#include	"streamsrv.h"
#include	"ddr3sdramsrv.h"

#include "fmcx_dev.h"
#include "fid_blk.h"
#include "main_blk.h"
#include "fmcx_trd.h"
#include "main_trd.h"
#include "fmcx_dma.h"
#include "axi_switch.h"

#include	<vector>

using namespace std;

// Constants
#define	VER_MAJOR		0x00010000L
#define	VER_MINOR		0x00000000L

#define MAX_DESCRIPLEN 128

#define MAX_STREAM_SRV 2
#define MAX_SYSMON_SRV 1
#define MAX_MEMORY_SRV 2

#define MAX_MEM_INI 2

const U16 MAX_BASE_CFGMEM_SIZE = 256; // размер под ICR, выделенный в ППЗУ, устанавливаемом на базовый модуль
const U16 OFFSET_BASE_CFGMEM = 128; // смещение для ICR в ППЗУ, устанавливаемом на базовый модуль

#pragma pack(push,1)

// board location data structure
typedef struct _AMB_LOCATION {
	ULONG	BusNumber;		// OUT
	ULONG	DeviceNumber;	// OUT 
	ULONG	SlotNumber;		// OUT
//} __packed AMB_LOCATION, *PAMB_LOCATION;
} AMB_LOCATION, *PAMB_LOCATION;

typedef struct _MEM_INI {
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

typedef struct _BASE_INI {
	U32		PID;			// серийный (физический) номер
	U32		OrderNumber;	// порядковый номер ПЛИС(для модулей CompactPCI, таких как FMC115cP)
	U32		BusNumber;		// номер шины (Bus Number)
	U32		DevNumber;		// номер устройства (Device Number)
	U32		SlotNumber;		// номер слота (Slot Number)
	U32		SysGen;			// значение частоты системного генератора в Гц
	MEM_INI MemIni[MAX_MEM_INI];// параметры инициализации памяти
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

#pragma pack(pop)

// Base Module Class definition
class CAxizynq : public CBaseModule
{

protected:

	fmcx_dev_t m_dev;
    main_blk_t m_blk_main;
    fid_blk_t m_blk_fid;
	fmcx_dma_t m_dma[MAX_STREAM_SRV];
	//fmcx_dma_t m_mm2s; // host->card
	//fmcx_dma_t m_s2mm; // card->host
	int m_dma_tetr_num[MAX_STREAM_SRV];

	//IPC_handle	m_hMutex;			// Mutex for multithread operation

	PBASE_INI	m_pIniData;		// Data from Registry or INI File

	U32		m_BusNum;			// Bus Number
	U32		m_DevNum;			// Device Number
	U32		m_SlotNum;			// Slot Number
	U32		m_SysGen;			// System Generator value (Hz)
	MEM_CFG	m_MemCfg[MAX_MEMORY_SRV]; // SDRAM configuration data
	DDR3_CFG m_Ddr3Cfg;			 // DDR3 configuration data
	
	PU_Info m_AdmPldInfo;		// ADM PLD info
	PU_Info m_BaseIcrInfo;		// base EEPROM info
	PU_Info m_AdmIcrInfo;		// subunit ICR EEPROM info
	//PULONG	m_pMemBuf[2];		// виртуальные адреса аппаратных областей
	//U32		m_PhysMem[2];		// физические адреса аппаратных областей
	PU_Info m_AdmFruInfo;		// subunit FRUID EEPROM info
	UCHAR	m_pldType;			// ADM PLD type (save for SYSMON)
	UCHAR	m_pldSpeedGrade;	// ADM PLD speed grade (save for SYSMON)
	ULONG GetBaseIcrDataSize();

	vector <PU_Info> m_PuInfos;
	void GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld);
	ULONG GetPldDescription(BRDCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);

	BRDextn_PLDFILEINFO m_pldFileInfo;	// информация о файле-прошивке, загруженной в ПЛИС
	void GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const;

	ULONG GetLocation(PAMB_LOCATION pAmbLocation);
	ULONG GetDeviceID(USHORT& DeviceID);
	ULONG GetRevisionID(UCHAR& RevisionID);
	ULONG GetPid();

	//ULONG ReadPldFile(const BRDCHAR* PldFileName, UCHAR*& fileBuffer, ULONG& fileSize);
	//ULONG CheckPldFile(const BRDCHAR* PldFileName);
	//ULONG LoadPldFile(const BRDCHAR* PldFileName, ULONG PldNum);
	//void* SearchInfoStart(UCHAR* fileBuffer, ULONG& fileSize);
	ULONG SetAdmPldFileInfo();
	
	ULONG GenReset();
	ULONG AllReset();
	ULONG Unreset();

	//ULONG LoadPld(const BRDCHAR* PldFileName, UCHAR& PldStatus);
	//ULONG SetPldStatus(ULONG PldStatus, ULONG PldNum);
	ULONG GetPldStatus(ULONG& PldStatus, ULONG PldNum);
	ULONG GetBaseIcrStatus(ULONG& Status);
	ULONG GetAdmIcrStatus(ULONG& Status, int idxSub); // определяет не просто наличие, а то что прописана информацией

	ULONG SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize); // override function of basemodule 
//	void GetAmbpCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbp pCfgBase);
	void GetAmbpexCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAmbpex pCfgBase);
	void GetFmcCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc105p pCfgBase);
	void GetDdr3CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdr3 pCfgDdr3);
	ULONG GetAdmIfCntICR(int& AdmIfCnt);
//	void GetSdramCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdrSdram pCfgSdram);
	ULONG SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	void SetSdramfromIniFile(PBASE_INI pIniData);
	ULONG GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize); // override function of basemodule
	ULONG SetSubEeprom(int idxSub, ULONG& SubCfgSize);

	CSdramSrv*	m_pMemorySrv[MAX_MEMORY_SRV];
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

	ULONG SetAxiSwitch(unsigned val_id);

	ULONG SetMemory(PVOID pParam, ULONG sizeParam);
	ULONG FreeMemory(PVOID pParam, ULONG sizeParam);
	ULONG StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StopMemory(PVOID pParam, ULONG sizeParam);
	ULONG StateMemory(PVOID pParam, ULONG sizeParam);
	ULONG SetDirection(PVOID pParam, ULONG sizeParam);
	ULONG SetSource(PVOID pParam, ULONG sizeParam);
	ULONG ResetFifo(PVOID pParam, ULONG sizeParam);
	ULONG GetDmaChanInfo(PVOID pParam, ULONG sizeParam);
#ifdef __linux__
	ULONG WaitDmaBlock(void* pParam, ULONG sizeParam);
	ULONG WaitDmaBuffer(void* pParam, ULONG sizeParam);
#endif

    const BRDCHAR* getClassName(void) { return _BRDC("CAxizynq"); }

public:

	CAxizynq();
	CAxizynq(PBRD_InitData pInitData, long sizeInitData);
    ~CAxizynq();

	S32 Init(short DeviceNumber, BRDCHAR* pBoardName, BRDCHAR* pIniString = NULL);
	void CleanUp();

	ULONG HardwareInit();

	ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

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

	ULONG WaitFidReady(ULONG* BlockMem);
	ULONG ReadFidSpd(int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length);
	ULONG WriteFidSpd(int devnum, int devadr, UCHAR* buf, USHORT Offset, int Length);
	virtual ULONG ReadFmcExt(PBRDextn_FMCEXT ext);
	virtual ULONG WriteFmcExt(PBRDextn_FMCEXT ext);
	//virtual ULONG ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte);
	//virtual ULONG WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte);

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

#endif	// _BAXIZYNQ_H_

// ****************** End of file baxizync.h **********************
