/*
 ****************** File basemodule.h ************************
 *
 * Definitions of structures and constants
 * of derived class CBaseModule for Data Acquisition Boards.
 *
 * (C) InSys by Dorokhin A. Mar. 2004
 *
 *
 *********************************************************
*/

#ifndef _BASEMODULE_H_
 #define _BASEMODULE_H_

#include	"extn.h"
#include	"service.h"
#include	"module.h"
//#include	"HwCfg.h"
#include	"icr.h"

#include	<vector>
#include	<string>
#include	<set>

using namespace std;

#define SUBNAME_SIZE 16

const U16 MAX_ADMIF	= 4; // максимальное число ADM-интерфейсов на базовом модуле

#define AMBPCD_DEVID 0x4D44 // DeviceID for AMBPCD
#define AMBPCX_DEVID 0x4D58 // DeviceID for AMBPCX

#define AMBPEX1_DEVID 0x5502 // DeviceID for AMBPEX1
#define VK3_DEVID	  0x5506 // DeviceID for VK3
#define AMBPEX2_DEVID 0x5508 // DeviceID for AMBPEX2

#define SYNCDAC_DEVID 0x53A0 // DeviceID for SYNCDAC
#define SYNCBCO_DEVID 0x53A1 // DeviceID for SYNCBCO
#define SYNCCP6_DEVID 0x53A2 // DeviceID for SYNC-cP6
#define DR16_DEVID	  0x53B0 // DeviceID for DR-16
#define GRADC_DEVID	  0x5701 // DeviceID for GR-ADC
#define C520F1_DEVID  0x53A3 // DeviceID for C520F1
#define RFDR4_DEVID	  0x53B4 // DeviceID for RFDR4
#define SYNCCP6R_DEVID 0x53B7 // DeviceID for SYNC-cP6R

#define AMBPEX8_DEVID		0x5503 // DeviceID for AMBPEX8
#define ADP201X1AMB_DEVID	0x5504 // DeviceID for ADP201X1AMB
#define ADP201X1DSP_DEVID	0x5505 // DeviceID for ADP201X1DSP
#define AMBPEX5_DEVID		0x5507 // DeviceID for AMBPEX5
#define UADCPEX_DEVID		0x5510 // DeviceID for УАЦП-PCI Express(Кулик)

#define FMC105P_DEVID		0x5509 // DeviceID for FMC105P
#define FMC106P_DEVID		0x550A // DeviceID for FMC106P
#define FMC103E_DEVID		0x550B // DeviceID for FMC103E2
#define FMC114V_DEVID		0x550C // DeviceID for FMC114V
#define FMC110P_DEVID		0x550D // DeviceID for FMC110P
#define FMC113E_DEVID		0x550E // DeviceID for FMC113E
#define FMC108V_DEVID		0x550F // DeviceID for FMC108V
#define FMC115CP_DEVID		0x53B1 // DeviceID for FMC115CP
#define FMC112CP_DEVID		0x53B2 // DeviceID for FMC112CP
#define FMC117CP_DEVID		0x53B3 // DeviceID for FMC117CP
#define FMC121CP_DEVID		0x53B5 // DeviceID for FMC121CP
#define FMC122P_DEVID		0x551C // DeviceID for FMC122P
#define FMC123E_DEVID		0x551D // DeviceID for FMC123E
#define FMC124P_DEVID		0x551E // DeviceID for FMC124P
#define FMC125CP_DEVID		0x53B6 // DeviceID for FMC125CP
#define FMC127P_DEVID		0x551F // DeviceID for FMC127P
#define FMC107P_DEVID		0x5511 // DeviceID for FMC107P
#define AC_ADC_DEVID		0x5512 // DeviceID for AсDSP_ADC
#define AC_DSP_DEVID		0x5513 // DeviceID for AсDSP_DSP
#define AC_SYNC_DEVID       0x5514 // DeviceID for Ac_SYNC
#define XM416X250M_DEVID    0x5516 // DeviceID for XM416X250M
#define FMC111P_DEVID		0x5521 // DeviceID for FMC111P
#define FMC126P_DEVID		0x5522 // DeviceID for FMC126P
#define FMC130E_DEVID		0x546A // DeviceID for FMC130E
#define FMC138M_DEVID		0x5438 // DeviceID for FMC138E
#define FMC132P_DEVID		0x5523 // DeviceID for FMC132P
#define FMC121P_DEVID		0x5524 // DeviceID for FMC121cP(PCIe)
#define FMC133V_DEVID		0x5525 // DeviceID for FMC133V
#define DSP134V_DEVID		0x5526 // DeviceID for DSP134V
#define RFG2_DEVID			0x5527 // DeviceID for RFG2
#define FMC131VZQ_DEVID		0x5528 // DeviceID for FMC131V Zynq
#define FMC131VKU_DEVID		0x5529 // DeviceID for FMC131V KU (FPGA)
#define FMC139P_DEVID		0x552A // DeviceID for FMC139P
#define FMC141VZQ_DEVID		0x5530 // DeviceID for FMC141V Zynq
#define FMC141VKU1_DEVID	0x5531 // DeviceID for FMC141V KU (FPGA 1)
#define FMC141VKU2_DEVID	0x5532 // DeviceID for FMC141V KU (FPGA 2)
#define FMC143VZQ_DEVID		0x552D // DeviceID for FMC143V Zynq
#define FMC143VKU_DEVID		0x552E // DeviceID for FMC143V KU (FPGA)
#define FMC143VKUS_DEVID	0x552F // DeviceID for FMC143V KU-Slave (FPGA)
#define FMC146VZQ_DEVID		0x5533 // DeviceID for FMC146V Zynq
#define FMC146VKU_DEVID		0x5534 // DeviceID for FMC146V KU (FPGA)

#define PS_DSP_DEVID		0x5518 // DeviceID for PS_DSP
#define PS_ADC_DEVID		0x5519 // DeviceID for PS_ADC
#define PS_SYNC_DEVID       0x551A // DeviceID for PS_SYNC
#define D2XT006_DEVID       0x5702 // DeviceID for D2XT006
#define PANORAMA_DEVID		0x53B8 // DeviceID for PANORAMA

#pragma pack(push,1)

typedef struct _SRV_OVUN {
	long	idxOverSrv;					// Index of Over Service
	long	idxUnderSrv[MAX_UNDERSRV];	// Index of UnderService
} SRV_OVUN, *PSRV_OVUN;

// Struct info about services, candidate services & under services 
typedef struct _SERV_Info {
	BRDCHAR	name[SERVNAME_SIZE];		// Service name with Number
	ULONG	attribute;					// Attributes of Service (Look BRDserv_ATTR_XXX constants)
	long	index;						// Index of this Service into Main Service List
//	long	idxOverSrv;					// Index of Over Service
//	long	idxUnderSrv[MAX_UNDERSRV];	// Index of Under Service
	PSRV_OVUN pSrvOvUn;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle	hFileMap;
#else
	HANDLE	hFileMap;
#endif
	long	idxCandidSrv[MAX_UNDERSRV];	// Index of Candidate Service
	UCHAR	baseORsideDLL;				// service locate into base DLL or side DLL
	void*	pSideDll;					// Side DLL object
	void*	pSideEntry;					// Side DLL API Entry Point
	ULONG	isCaptured;					// the Under Service was captured by the process
} SERV_Info, *PSERV_Info;

// Struct info about side DLL 
typedef struct _SIDEDLL_Info {
	S32				type;		// Subunit Type ID
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle		hLib;		// Dll Handle 
#else
	HINSTANCE		hLib;		// Dll Handle 
#endif
	SIDE_API_entry*	pEntry;		// SIDE API Entry Point
	void*			pSideDrv;	// SIDE Driver Extension
} SIDEDLL_Info, *PSIDEDLL_Info;

// Struct info about Subunit
typedef struct _SUBUNIT_Info {
	S32		type;			// Subunit Type ID
	PVOID	pSubEEPROM;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle	hFileMap;
#else
	HANDLE	hFileMap;
#endif
} SUBUNIT_Info, *PSUBUNIT_Info;

#pragma pack(pop)

// Module base class definition
class CBaseModule : public CModule
{

protected:

	U16		m_DeviceID;			// PCI Device Identifier
	U08		m_RevisionID;		// PCI Revision Identifier

	USHORT m_sizeICR; // размер ППЗУ на базовом модуле

	vector <PSERV_Info> m_ServInfoList; // Services info list
	void AddServInfoList(PSERV_ListItem pServItem, UCHAR flagDll, void* pSideDll, void* pFunc);

	int				m_sidedllCnt;		// количество боковых библиотек
	SIDEDLL_Info	m_sidedll[2*MAX_ADMIF];	// Total Side dll Information

  	const BRDCHAR* getClassName(void) { return _BRDC("CBaseModule"); }

	int		m_AdmIfCnt;			// число ADM-интерфейсов
	int		m_SubunitCnt;		// количество субмодулей
	SUBUNIT_Info m_Subunit[MAX_ADMIF];	// Total Subunit Information
	int		m_isFMC2;

	ULONG SetSubunitAuto();
	virtual ULONG GetBaseEEPROM(PUCHAR pBaseCfgMem, ULONG& BaseCfgSize);
	virtual ULONG GetSubEEPROM(int idxSub, PUCHAR pSubCfgMem, ULONG& SubCfgSize);

	virtual ULONG SetSubEeprom(int idxSub, ULONG& SubCfgSize);

	BRD_Handle m_curHandle;	// дескриптор для выполняемой в данный момент SrvCtrl()

	BRDCHAR subsectionName[SUBNAME_SIZE];

public:

	CBaseModule(const U16 sizeICR);
	virtual ~CBaseModule();

//	int GetAdmIfCntAuto() { return m_AdmIfCnt;};
	S32 GetSubunitTypeAuto(int idxSubunit, ULONG& type);
//	void InitAutoSubDll(PTSTR pInitStr, long InitStrSize, PBRD_InitEnum pSubEnum, ULONG sizeSubEnum);
	void InitAutoSubDll(BRDCHAR *pInitStr, long InitStrSize, PBRD_SubEnum pSubEnum, ULONG sizeSubEnum);
	void InitDataSubDll(PBRD_InitData pInitData, S32 initDataSize);
	long BaseDllInit(BRDCHAR *pLibName, int idxSub, void* pSubInitData, long subInitDataSize, int Auto);
	S32 SubunitInit(BRDCHAR *pLibName, int idxSubunit, void* pSubInitData, S32 subInitDataSize, int Auto);
	virtual void Open();
	virtual void Close();

	virtual ULONG GetBaseSrvCnt() const;
	virtual ULONG GetSrvCnt() const;
	virtual void GetBaseSrvList(PSERV_ListItem srv_list, int& idx_srv_list, set <string> *nameSet);
	virtual void GetBaseSrvList(PSERV_ListItem srv_list, int& idx_srv_list, set <wstring> *nameSet);
	virtual void GetSrvList(PSERV_ListItem list); // CModule also
	virtual ULONG SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext); // CModule also

#ifdef __linux__
        virtual ULONG StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap = 0);
#else
        virtual ULONG StreamCtrl(ULONG cmd, PVOID pParam, ULONG sizeParam, LPOVERLAPPED pOverlap = NULL);
#endif

	virtual ULONG CmdQuery(BRD_Handle handle, long idx, ULONG cmd);
	virtual ULONG UnderSrvCapture(BRD_Handle handle, long idxSrv, long index); // захват подслужбы
	virtual ULONG UnderSrvRelease(BRD_Handle handle, long idxSrv, long idx); // освобождение подслужбы
//	virtual ULONG CmdQuery(long idx, ULONG cmd); // поиск подслужбы по команде
	ULONG UnderSrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext);
	ULONG CandidSrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext);

//	inline void SetCurHandle(BRD_Handle handle) {m_curHandle = handle;};

	virtual ULONG GetPower(PBRDextn_FMCPOWER pow);
	virtual ULONG PowerOnOff(PBRDextn_FMCPOWER pow, int force);
	virtual ULONG ReadFmcExt(PBRDextn_FMCEXT ext);
	virtual ULONG WriteFmcExt(PBRDextn_FMCEXT ext);
	virtual ULONG ReadI2C(U32 devnum, U32 devadr, U32 regadr, U32* pVal, int dblbyte);
	virtual ULONG WriteI2C(U32 devnum, U32 devadr, U32 regadr, U32 val, int dblbyte);
	virtual ULONG GetSensors(void* sens);
	virtual ULONG PuVerify(PBRDextn_PuVerify arg);
	virtual ULONG SetIrqMinTime(ULONG* pTimeVal);
	virtual ULONG GetIrqMinTime(ULONG* pTimeVal);

	virtual ULONG ReadAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG WriteAdmIdROM(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG ReadSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG WriteSubICR(int submod, PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG ReadFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset);
	virtual ULONG WriteFmcFRUID(PVOID pBuffer, ULONG BufferSize, ULONG Offset);

	virtual ULONG RegisterCallBack(PVOID funcName);
	virtual ULONG PldReadToFile(U32 puId, const BRDCHAR *fileName);
};

#endif	// _BASEMODULE_H_

// ****************** End of file basemodule.h **********************
