// clang-format off
#ifndef _BAMBUSB_H_
 #define _BAMBUSB_H_

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

#include	"Icrfmc119e.h"
#include	"pld.h"

//#include	"datainsrv.h"
//#include	"dataoutsrv.h"
#include	"streamsrv.h"
#include	"testsrv.h"
#include	"sysmonsrv.h"
#include	"ddr3sdramsrv.h"
//#include	"fmc128srv.h"
#include	"gipcy.h"
#ifdef __CYPRESS_API
#include    "CyAPI.h"
#else
#include    "cyapi.h"
#endif

#include	"stdlib.h"

#include	<vector>
using namespace std;

#define	VER_MAJOR		0x00000001L
#define	VER_MINOR		0x00000000L
#define VER_BCD			0x0200

const U32 MAX_STREAM_SRV = 4;
const U32 MAX_SYSMON_SRV = 1;
const U32 MAX_MEMORY_SRV = 4;
const U32 MAX_TEST_SRV = 1;

const U16 MAX_BASE_CFGMEM_SIZE = 384; // размер под ICR, выделенный в ППЗУ, устанавливаемом на базовый модуль
const U16 OFFSET_BASE_CFGMEM = 128; // смещение для ICR в ППЗУ, устанавливаемом на базовый модуль

#pragma pack(push,1)

typedef struct _MEM_INI {
	U32		TetrNum;		// номер тетрады (перебивает ID тетрады)
	U32		AdrMask;		// маска разрядов адреса памяти (биты 0..9 соответствуют адресным битам 20..29)
	U32		AdrConst;		// значение разрядов адреса памяти  (биты 0..9 соответствуют адресным битам 20..29)
} MEM_INI, *PMEM_INI;

typedef struct _BASE_INI {
	U32			PRID;			// серийный (физический) номер изделия
	U32			VID;			// номер вендора
	U32			RESET;		//режим подачи сброса ARM
	//U32			MEMTETR;	//номер тетрады памяти
	U32			ISDEBUG;	// TaskID = 0xFF для отладки
	U32			FMCPOWER;	// FMC Power enable on init
	BRDCHAR		PID[8];		// серийный номер платы
	U32			LOGGING;	//включение логирования команд
	U32			REGLOG;		//включение RegLog
	U32			ENUM_WAIT;	//время ожидания появления платы в системе
	U32			LOAD_WAIT;	//время ожидания загрузки ПЛИС/ARM
	MEM_INI		MemIni[MAX_MEMORY_SRV];
	U32			ARMPROG;	//признак программирования ARM
} BASE_INI, *PBASE_INI;

typedef struct _PU_Info {
	U32		Id;							// Programmable Unit ID
	U32		Type;						// Programmable Unit Type
	U32		Attr;						// Programmable Unit Attribute
	TCHAR	Description[MAX_DESCRIPLEN];// Programmable Unit Description Text
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
	0,			// маска разрядов адреса памяти (биты 0..9 соответствуют адресным битам 20..29)
	0			// значение разрядов адреса памяти  (биты 0..9 соответствуют адресным битам 20..29)
};

typedef struct _USB_STREAM_DESC {
	ULONG			Cycling;
	ULONG			CycleNum;
	BRDstrm_Stub	CurState;
	ULONG	BlockCount;
	ULONG	BlockSize;
	HANDLE  hBufEndEvent;
	HANDLE	hBlockEndEvent;
	IPC_handle	hThread;
	U32		threadID;
	UCHAR	isBreak; // 0 => stream work; 1 => stop stream; 2 => need dummy start
	UCHAR	bEndPoint;
	U32		nTetrNum;
	PVOID*	pBlock;
	U32	nDone;
	U32 isAdjust;
	
} USB_STREAM_DESC, *PUSB_STREAM_DESC;

union rHostCommand {
	struct {USHORT code;} error;
	struct {USHORT parametr:8,condition:3,type:5;} system;
	struct {USHORT number:8,port:2,write:1,type:5;} hardware;
	struct {USHORT number:8,condition:3,type:5;} tetrad;
	struct {USHORT reference:5,name:2, spar:1,condition:3,type:5;} stream;
	struct {USHORT request:8, requestH:5, type:3;} setup;
	struct {USHORT address:8, addrH:5, type:3;} buffer;
	struct {USHORT address:8, addrH:5, type:3;} read;
	struct {USHORT address:8, addrH:5, type:3;} program;
	struct {USHORT ready:1, reserved:2, swap:1, endean:2, doubling:2, res:1, timeexp:4, rest:3;} status;

};


struct rUsbHostCommand {
	union {	UINT64 value;
	struct {UINT32 valueL; UINT32 valueH;};
	};
	union rHostCommand command;
	union {USHORT transaction;
	struct {UCHAR shortTransaction;UCHAR taskID;};
	};
	USHORT debugValue;
	UCHAR versionValue;
	UCHAR signatureValue;
};

struct rICRmapping {
	PVOID pBaseICR;
	HANDLE hBaseICR;
	PVOID pSubICR;
	HANDLE hSubICR;
	PVOID pFRUID;
	HANDLE hFRUID;
};

#pragma pack(pop)

// Base Module Class definition
class CBambusb : public CBaseModule
{

protected:

	//U16		m_DeviceID;			// PCI Device Identifier
	//U08		m_RevisionID;		// PCI Revision Identifier
	
	U08		m_TaskID;			// application identifier for ARM	
	U08		m_EPin;				// IN command endpoint
	U08		m_Trans;			// command trasaction
	U08		m_isLog;			// command log enable
	DWORD   m_LastComTime;		// last command timecode

	MEM_INI	m_memIni[MAX_MEMORY_SRV];
	//U08		m_memTetrNum[MAX_MEMORY_SRV];		//номер тетрады памяти

	U08		m_baseICR[512];		// base ICR
	U08		m_subICR[512];		// submodule ICR
	U08		m_FRUID[1024];		// FRU ID
	U08		m_RegLog;			// enables logging of register operations
	U08		m_isArmProg;		// enables ARM_PU
	U08		m_isDLMode;			// special mode without FPGA firmware

	rICRmapping m_rICRmap;		// file mappings for ICR and FRU

	IPC_handle	m_hCommand;			// mutex for commands
	IPC_handle	m_hStartStop;		//mutex for start and stop operations
	IPC_handle	m_hServThread;		// handle for service thread
	
	TCHAR	m_pDlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll

	//MEM_CFG	m_MemCfg[MAX_MEMORY_SRV]; // SDRAM configuration data
	DDR3_CFG m_Ddr3Cfg[MAX_MEMORY_SRV];			 // DDR3 configuration data

	UCHAR	m_pldType;			// ADM PLD type (save for SYSMON)
	
	//U32		m_IPaddress;		// IP address
	//U32		m_IPport;			// IP port
	U32		m_SysGen;			// System Generator value (Hz)

	PU_Info m_AdmPldInfo;		// ADM PLD info
	PU_Info m_AdmRomInfo;		// ADM ROM info
	PU_Info m_BaseIcrInfo;		// base EEPROM info
	PU_Info m_AdmIcrInfo;		// subunit EEPROM info
	PU_Info m_ArmPldInfo;		// ARM PLD INFO

	ULONG GetBaseIcrDataSize();

	void (*vPULoadProgressCB)(ULONG, ULONG, ULONG, ULONG);
	
	void CallBackLoadProgress(ULONG nStage, ULONG nStageMax, ULONG nCurrent, ULONG nMax);

	vector <PU_Info> m_PuInfos;
	void GetPldCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgAdmPld pCfgPld);
	ULONG GetPldDescription(TCHAR* Description, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);

	BRDextn_PLDFILEINFO m_pldFileInfo;	// информация о файле-прошивке, загруженной в ПЛИС
	void GetAdmPldFileInfo(PBRDextn_PLDFILEINFO pInfo) const;
	
	CCyFX3Device*	m_pDevice; //device object

	ULONG GetIdInfo(U16& devId, U08& revId);
	//ULONG GetPid();
	ULONG GetICR(PVOID pBuffer, U08 bICRType);
	ULONG GetNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG GetAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	ULONG PldReadToFileTetrad(const BRDCHAR *PldFileName, int nTetr);
	ULONG PldReadToFileAlt(const BRDCHAR *PldFileName);
	ULONG ArmReadToFile(const BRDCHAR *PldFileName);
	ULONG ReadPldFileTetrad(int nTetr, const BRDCHAR *PldFileName);
	ULONG ReadPldFileAlt(const BRDCHAR *PldFileName);
	ULONG LoadBitFile(const BRDCHAR *PldFileName);
	ULONG LoadPldFile(const BRDCHAR *PldFileName, ULONG PldNum);
	ULONG LoadPldFileTetrad(int nTetr, const BRDCHAR *PldFileName);
	ULONG LoadPldFileAlt(const BRDCHAR *PldFileName);
	ULONG LoadPldFileCom(bool isFPGAROM, const BRDCHAR *PldFileName, int nMaxBuf, unsigned int ep_in, unsigned char bSignature, unsigned char bDoubling, unsigned char bEndean, unsigned char bSwap, USHORT *wTrans);
	ULONG LoadPldFileDMA(const BRDCHAR *PldFileName, int nStreamEp, int nInStreamEp, unsigned int ep_in, int nMaxBuf, USHORT *wTrans, unsigned char bDoubling, unsigned char bEndean, unsigned char bSwap);
	ULONG LoadArmFile(const BRDCHAR *fileName);
	ULONG ErasePld(int nTimeExp, unsigned int ep_in, unsigned char bSignature, unsigned char bEdge, unsigned int nHighAddr, unsigned int nLastAddr, USHORT *wTrans);
	ULONG AdmPldStartTest();
	ULONG AdmPldWorkAndCheck();
	ULONG SetAdmPldFileInfo();

	ULONG WaitSemaReady(int SemaOrReady);
	U32 EepromWordRead(int idRom, U32 addr);
	ULONG EepromWordWrite(int idRom, U32 addr, U32 data);
	//ULONG EepromBufRead(int idRom, PUSHORT buf, U32 Offset, long Length);
	//ULONG EepromBufWrite(int idRom, PUSHORT buf, U32 Offset, long Length);
	//ULONG EepromBufRead(int idRom, PUCHAR buf, U32 Offset, long Length);
	//ULONG EepromBufWrite(int idRom, PUCHAR buf, U32 Offset, long Length);
	ULONG EepromBufReadFromBase(PUCHAR buf, U32 Offset, LONG Length);
	ULONG EepromBufWriteIntoBase(PUCHAR buf, U32 Offset, LONG Length);
	ULONG EepromBufReadFromAdm(PUSHORT buf, U32 Offset, LONG Length);
	ULONG EepromBufWriteIntoAdm(PUSHORT buf, U32 Offset, LONG Length);

	UCHAR FlashGetVendorID();
	UCHAR FlashGetDeviceID();
	//void FlashReset();
	U32 FlashGetStatus(U32 addr);
	//void FlashBlockErase(U32 blknum);
	ULONG FlashWordProg(U32 addr, U32 data);
	//void FlashBlockProg(U32 addr, PUSHORT buf, long Length);
	//void FlashBlockProg(U32 addr, PUCHAR buf, long Length);

	ULONG WaitPldReady(ULONG PldNum);
	ULONG WritePldData(ULONG Val);
	ULONG WritePldBuf(PVOID Buf, ULONG BufSize);
	//ULONG LoadPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize, ULONG PldNum);
	ULONG LoadPld(UCHAR& PldStatus, PVOID pBuffer, ULONG BufferSize);
	//ULONG LoadPld(PCTSTR PldFileName);
	ULONG GetPldStatus(ULONG& PldStatus, ULONG PldNum);
	ULONG GetBaseIcrStatus(ULONG& Status);
	ULONG GetAdmIcrStatus(ULONG& Status); // определяет не просто наличие, а то что прописана информацией

	ULONG SetPuInfo(PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize); // override function of basemodule
	void GetBaseCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc119e pCfgBase);
	ULONG GetAdmIfCntICR(int& AdmIfCnt);
	void GetFmcCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgFmc119e pCfgBase);
	void GetDdr3CfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdr3 pCfgDdr3);
//	void GetSdramCfgICR(PVOID pCfgMem, ULONG RealBaseCfgSize, PICR_CfgDdrSdram pCfgSdram);
	ULONG SetSdramConfig(PBASE_INI pIniData, PUCHAR pBaseEEPROMMem, ULONG BaseEEPROMSize);
	ULONG GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize); // override function of basemodule
	ULONG SetSubEeprom(int idxSub, ULONG& SubCfgSize);

	//CDataInSrv*	m_pDataInSrv;
	//CDataOutSrv*	m_pDataOutSrv;
	CStreamSrv*	 m_pStreamSrv[MAX_STREAM_SRV];
	CSysMonSrv*	m_pSysMonSrv[MAX_SYSMON_SRV];
	CSdramSrv*	m_pMemorySrv[MAX_MEMORY_SRV];
	CTestSrv*	m_pTestSrv[MAX_TEST_SRV];
//	Cfmc128srv* m_pFmc128Srv;
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
	ULONG FormCommand(ULONG AdmNum, ULONG Param, ULONG HValue, U32& LValue, U08 bType, U08 Port, U08 bOpCode, bool isWrite, PVOID pBuf); //form command
	ULONG SendCommand(ULONG AdmNum, ULONG Param, ULONG HValue, U32& LValue, U08 bType, U08 Port, U08 bOpCode, bool isWrite); //send command by USB
	ULONG PackExecute(ULONG AdmNum, ULONG TetrNum, ULONG RegNum, PVOID RegBuf, ULONG RegBufSize);//form batch of commands to send by usb
	ULONG SendBuffer(PVOID pOutBuf, PVOID pInBuf, LONG& nOutSize/*, LONG& nInSize*/);	//send batch of commands by USB
	ULONG CheckCommand(PVOID pOutBuf, PVOID pInBuf); //check command for errors

	ULONG SetMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG FreeMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StartMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StopMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG StateMemory(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG SetDirection(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG SetSource(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG SetDmaRequest(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG GetDmaChanInfo(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG ResetFIFO(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG WaitDMABlock(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG WaitDMABuffer(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG Adjust(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);
	ULONG BufferDone(PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);

	ULONG GetTetrNum(ULONG& tetrNum);
	//ULONG SetTetrNum(ULONG TetrNumber);
	ULONG SetDmaMode(ULONG NumberOfChannel, ULONG AdmNumber, ULONG TetrNumber);

	const BRDCHAR* getClassName(void) { return _BRDC("CBambusb"); }

public:

	CBambusb();
	CBambusb(PBRD_InitData pInitData, LONG sizeInitData);
	~CBambusb();

	S32 Init(short DeviceNumber, BRDCHAR* pBoardName, BRDCHAR* pIniString = NULL);
	void CleanUp();
	void Open();
	void Close();

	ULONG ChangeConfig(UCHAR bMode);

	ULONG HardwareInit();

	ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	//ULONG ReadNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset, ULONG flag = 0);
	ULONG WriteNvRAM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	ULONG ReadSubICR(ULONG puId, void* buffer, U32 bufSize, U32 bufOffset);
	ULONG WriteSubICR(ULONG puId, void* buffer, U32 bufSize, U32 bufOffset);

	ULONG ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	ULONG WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	void GetDeviceInfo(BRD_Info* info) const;

	ULONG GetPldFileInfo(PBRDextn_PLDFILEINFO pInfo);
	ULONG ShowDialog(PBRDextn_PropertyDlg pDlg);
	ULONG DeleteDialog(PBRDextn_PropertyDlg pDlg);

	ULONG GetPldInfo(PBRDextn_PLDINFO pInfo);
	ULONG GetPower(PBRDextn_FMCPOWER pow);
	ULONG PowerOnOff(PBRDextn_FMCPOWER pow, int force);
	
	ULONG ReadFmcExt(PBRDextn_FMCEXT ext);
	ULONG WriteFmcExt(PBRDextn_FMCEXT ext);

	void GetPuList(BRD_PuList* list) const;
	ULONG PuFileLoad(ULONG id, const BRDCHAR *fileName, PUINT pState);
	ULONG PuVerify(PBRDextn_PuVerify arg);
	ULONG GetPuState(ULONG id, PUINT pState);
	void SetCandidateSrv();
	LONG RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam); // CModule also
	ULONG StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap);

	PUSB_STREAM_DESC m_pStreamDscr[MAX_STREAM_SRV];

	ULONG DmaEnable(ULONG AdmNumber, ULONG TetrNumber);
	ULONG DmaDisable(ULONG AdmNumber, ULONG TetrNumber);

	ULONG I2CCapture(void);
	ULONG I2CRelease(void);

	ULONG ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte);
	ULONG WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte);
	ULONG GetSensors(void* sens);

	ULONG RegisterCallBack(PVOID funcName);
	ULONG PldReadToFile(U32 puId, const BRDCHAR *fileName);

	PBASE_INI	m_pIniData;		// Data from Registry or INI File

};

#endif	// _BAMBUSB_H_