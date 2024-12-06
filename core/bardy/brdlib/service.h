/*
 ****************** File service.h ************************
 *
 * (C) InSys by Dorokhin A. Nov. 2003
 *
 ******************************************************
*/

#ifndef _SERVICE_H_
 #define _SERVICE_H_

#include	"brderr.h"
#include	"brdapi.h"
#include	"useful.h"

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
#include	"gipcy.h"
#endif

typedef DEV_CMD_ServListItem SERV_ListItem, *PSERV_ListItem;

#include	<vector>

using namespace std;

#define MAX_UNDERSRV 8

#define MAIN_TETR_ID 1

class CService;

typedef int (CService::*CmdEntry)(
								  void		*pDev,		// pointer to CModule or BaseModule or SubModule
								  void		*pServData,	// Specific Service Data
								  ULONG		cmd,		// pointer to Command Code (from BRD_ctrl())
								  void		*args 		// Command Arguments (from BRD_ctrl())
								  );

typedef struct _CMD_Info {
	ULONG	code;				// Command Code
	ULONG	isSpy		: 1,	// 0-Control Cmd, 1-Monitoring Cmd
			isCapture	: 1,	// 0-don't capture the Service, 1-capture the Service if need
			isRelease	: 1;	// 0-don't release the Service, 1-release the Service if need
	CmdEntry pEntry;			// Command Support Entry Point
} CMD_Info, *PCMD_Info;

typedef struct _OVUN_SRV {
	long	idxOverSrv;					// Index of Over Service
	long	idxUnderSrv[MAX_UNDERSRV];	// Index of UnderService
} OVUN_SRV, *POVUN_SRV;

typedef struct _UNDERSERV_Cmd {
	ULONG	code;					// Command Code
	PVOID	pParams;				// pointer to Command parameters
} UNDERSERV_Cmd, *PUNDERSERV_Cmd;

class CService
{

protected:

	BRDCHAR	m_name[SERVNAME_SIZE];			// Service name with Number
	ULONG	m_attribute;					// Attributes of Service (Look BRDserv_ATTR_XXX constants)
	long	m_index;						// Index of this Service into TECH/DUPLEX Main Service List
	long	m_idxCandidSrv[MAX_UNDERSRV];	// Index of Candidate Service
	ULONG	m_isAvailable;					// flag availabled (tetrad present)
	vector <CMD_Info*>	m_Cmd;				// Service Command List

	POVUN_SRV m_pOvUnSrv;
	PVOID	m_pModule;
	PVOID	m_pConfig;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle	m_hFileMap;
	IPC_handle	m_hMutex;				// Mutex for Multythread Operation
#else
	HANDLE	m_hFileMap;
	HANDLE	m_hMutex;				// Mutex for Multythread Operation
#endif
	DEVS_API_PropDlg* m_pPropDlg;

  	const BRDCHAR* getClassName(void) { return _BRDC("CService"); }

    template<typename T>
    static T& ModuleCast(void* pDev) {
        return *static_cast<T*>(pDev);
    };

    template<typename T>
    static T& ArgumentCast(void* args) {
        return *static_cast<T*>(args);
    };

 	template<typename T>
    static T& ConfigCast(void* pConfig) {
        return *static_cast<T*>(pConfig);
    };

public:

	CService(int idx, const BRDCHAR *name, int num, PVOID pModule, PVOID pCfg, ULONG cfgSize);
	virtual ~CService();

	// Запрещаем копирование/перемещение
    CService(const CService&) = delete;
    CService(CService&&) = delete;
    CService& operator=(const CService&) = delete;
    CService& operator=(CService&&) = delete;

	virtual void SetCfgMem(PVOID pCfg, ULONG cfgSize);
	virtual void SetPropDlg(PVOID pPropDlg);

	void Init(PCMD_Info pCmdInfo, CmdEntry pEntry);
	inline ULONG GetAvailable() {return m_isAvailable;};

	inline BRDCHAR *GetName() {return m_name;};
	inline void SetName(BRDCHAR *name) {	BRDC_strcpy(m_name, name); };
	inline ULONG GetAttribute() {return m_attribute;};
	inline long GetIndex() {return m_index;};
	inline void SetIndex(long index) { m_index = index;};
	inline long GetCandidIndex(int num) {return m_idxCandidSrv[num];};
	inline void SetCandidIndex(int num, long index) { m_idxCandidSrv[num] = index;};
	inline long GetOverIndex() {return m_pOvUnSrv->idxOverSrv;};
	inline void SetOverIndex(long index) { m_pOvUnSrv->idxOverSrv = index;};
	inline long GetUnderIndex(int num) {return m_pOvUnSrv->idxUnderSrv[num];};
	inline void SetUnderIndex(int num, long index) { m_pOvUnSrv->idxUnderSrv[num] = index;};
	ULONG DoCmd(PVOID pModule, ULONG cmd, int mode, PVOID pData, PVOID pServData);
	virtual ULONG IsCmd(ULONG cmd);

	virtual int SetPropertyFromDialog(
									void *pDev,
									void *args
									);

	virtual int CtrlIsAvailable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   );
	virtual int CtrlCapture(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	virtual int CtrlRelease(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );
	virtual int CtrlGetAddrData(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   );

	void WriteInifileParam(const BRDCHAR* FileName,
						const BRDCHAR* SectionName,
						const BRDCHAR* ParamName,
						ULONG value,
						int base,
						const BRDCHAR* Comment);

	void WriteInifileParam(const BRDCHAR* FileName,
						const BRDCHAR* SectionName,
						const BRDCHAR* ParamName,
						double value,
						int prec,
						const BRDCHAR* Comment);

protected:
	//
	// Read/Write Tetrada's Registers
	//
	S32	IndRegRead( void* pModule, S32 tetrNo, S32 regNo, U32 *pVal );
	S32	IndRegWrite( void* pModule, S32 tetrNo, S32 regNo, U32 val );

	S32	DirRegRead( void* pModule, S32 tetrNo, S32 regNo, U32 *pVal );
	S32	DirRegWrite( void* pModule, S32 tetrNo, S32 regNo, U32 val );

};

#endif	// _SERVICE_H_

// ****************** End of file service.h **********************
