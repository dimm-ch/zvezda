/*
 ****************** File module.h ************************
 *
 * Definitions of structures and constants
 * of base class CModule for Data Acquisition Boards.
 *
 * (C) InSys by Dorokhin A. Feb. 2004
 *
 *
 *********************************************************
*/

#ifndef _MODULE_H_
 #define _MODULE_H_

#include	"service.h"

#include	<vector>

using namespace std;

// Module base class definition
class CModule
{

protected:

	BRDCHAR	m_name[BOARDNAME_SIZE];// Exclusive Name of Module
	LONG	m_errcode;			// Driver Last Error Code
	LONG	m_errcnt;			// Driver Error Counter
	ULONG	m_langID;			// primary language identifier

	ULONG	m_PID;				// Physical Identifier (Serial Number)

	ULONG	m_puCnt;			// Programmable Unit Count

	vector <CService*> m_SrvList; // Services List

  	const BRDCHAR* getClassName(void) { return _BRDC("CModule"); }

public:

	CModule();
	virtual ~CModule();

    // Запрещаем копирование/перемещение
    CModule(const CModule&) = delete;
    CModule(CModule&&) = delete;
    CModule& operator=(const CModule&) = delete;
    CModule& operator=(CModule&&) = delete;

	inline int GetPID() {return m_PID;};
	inline BRDCHAR *GetName() {return m_name;};

	inline int GetPuCnt() {return m_puCnt;};

	inline ULONG GetSrvCnt() {return (ULONG)m_SrvList.size();};
	virtual void GetSrvList(PSERV_ListItem list);

	virtual ULONG SrvCtrl(BRD_Handle handle, ULONG cmd, PVOID arg, PVOID pContext); // управление службой
	// virtual ULONG CmdQuery(long idx, ULONG cmd); // поиск подслужбы по команде
	ULONG CmdQuery(long idx, ULONG cmd); // поиск подслужбы по команде

	virtual LONG RegCtrl(ULONG cmd, PDEVS_CMD_Reg pRegParam);

	ULONG PropertyFromDialog(void* pServ, void* pParam);

//	int GetStringText(UINT uID, PTSTR title);
//	int GetStringText(DWORD dwMessageId, LPTSTR strMsg);

//	void WriteEventLog(LONG errorCode, LPCTSTR strMsg);

	void SetError(LONG errorCode);
	LONG ErrorPrint(LONG errorCode, const BRDCHAR *title, const BRDCHAR *msg);
	LONG ErrorPrintf(LONG errorCode, const BRDCHAR *title, const BRDCHAR *format, ... );
};

#endif	// _MODULE_H_

// ****************** End of file module.h **********************
