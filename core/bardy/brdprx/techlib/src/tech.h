
#ifndef __TECH_H__
#define __TECH_H__

#include "brdapi.h"
#include "dict.h"
#include "array.h"
#include "str.h"
#include "gipcy.h"

enum
{
	TECHcmd_INIT,
	TECHcmd_CTRL
};

class TModule;
class TModuleProxy;
class TSrv;

typedef struct
{
	BRDCHAR		name[SERVNAME_SIZE];// Service name with Number
	U32			attr;			// Attributes of Service (Look BRDserv_ATTR_XXX constants)
	S32			idxSuperServ;	// Index of SuperService which forced this Service to be captured
} TSrvInfo;

typedef struct{
	long openCnt;
}TBoardSharedInfo;

class TSrvProxy;
class TSideProxy;
class TModule;



/*

 Abstract parrent

*/


template <class OutputClass, class InputClass>
inline OutputClass horrible_cast(InputClass input)
{
    union
    {
        OutputClass out;
        InputClass in;
    };

    in = input;
    return out;
}

typedef struct
{
	BRDCHAR *name;
	int cmd;
	void *method;
	char *params;
} TModuleEntry;

#define DEFINE_ENTRY( classname, cmd, name, params ) { (BRDCHAR *)_BRDC(#name), cmd, horrible_cast<void*>(&classname::name), (char *)params }

typedef S32 (TModule::*pt2Member)( void *cmd, void *pParam, void *isCheckOpen );


class TModule
{
public:

	BRDCHAR	boardName[BOARDNAME_SIZE];// Exclusive Name of Module

	LONG	m_errcode;			// Driver Last Error Code 
	LONG	m_errcnt;			// Driver Error Counter 
	ULONG	m_langID;			// primary language identifier

	ULONG	pid;				// Physical Identifier (Serial Number)

	ULONG	m_puCnt;			// Programmable Unit Count



	void *m_pEntry;
	void *m_pSideEntry;
	
	HANDLE			hMutex;		// Mutex for Multythread Operation



    virtual S32 GetInfo( BRD_Info *pInfo ) = 0;
    virtual S32 readIcr( void *pData, U32 tag, U32 size, U32* sizeOut=NULL ) = 0;

	virtual S32 Ctrl( S32 cmd,void *pParam ) { return BRDerr_FUNC_UNIMPLEMENTED; };

	S32 DoCtrl( TModuleEntry *pCmdArray, int nCmd,S32 cmd,void *pParam );

	void Release(){ delete this; };

};


class TModuleProxy : public TModule
{
public:
	IPC_handle m_hLib;
	
	void *m_pDev;
	
	

	
	
protected:
	TSrvInfo	*m_pServList;		// Array of Service Controls
	S32				m_nServListSize;	// Number of Items in the "pServList[]"	
	
public:
    virtual S32	CallDriver( S32 cmd, void *pParam, S32 isCheckOpen ) = 0;
	virtual S32	CallSide( S32 cmd, void *pParam )
	{
		return BRDerr_FUNC_UNIMPLEMENTED;
	}

	virtual int GetServiceCount() { return m_nServListSize; };
	virtual TSrvInfo *GetService( int index ) { return &m_pServList[index]; };

	virtual S32 GetInfo( BRD_Info *pInfo );

	virtual void CreateServList();
	virtual void ReleaseServList();

	TSrvProxy  *InitSrv( U32 *pMode, const BRDCHAR *resName, int num, U32 timeout );
	TSrvProxy  *ProxySrv( U32 *pMode, const BRDCHAR *resName, int num, U32 timeout );

	virtual S32 readIcr( void *pData, U32 tag, U32 size,U32* sizeOut=NULL )
	{
		return BRDerr_FUNC_UNIMPLEMENTED;
	}	



};

#define ENTRY_SIZE(x) sizeof(x)/sizeof(x[0])

#define DECLARE_ENTRY( classname ) \
	static TModuleEntry pCmdArray[]; \
	S32 Ctrl( S32 cmd,void *pParam );

#define IMPLEMENT_ENTRY( baseclassname, classname ) \
	S32 classname::Ctrl( S32 cmd,void *pParam ) \
	{ \
		S32 ret = baseclassname::Ctrl(cmd,pParam); \
		if( ret != BRDerr_FUNC_UNIMPLEMENTED ) \
			return ret; \
		return DoCtrl( classname::pCmdArray, ENTRY_SIZE(classname::pCmdArray) , cmd,pParam ); \
	}; 

#define DEFINE_ENTRY_MAP( classname ) \
	TModuleEntry classname::pCmdArray[] =


class TBaseProxy : public TModuleProxy
{ 
	friend class TSrvProxy;
	friend class TSideProxy;
public:
	
	TBoardSharedInfo *pBSI;		// Pointer      to Keep Open Protection Shareable Information
	HANDLE			hFileMap;		// File Mapping to Keep Open Protection Shareable Information

public:
	TBaseProxy();
	~TBaseProxy();

	virtual S32	CallDriver( S32 cmd, void *pParam, S32 isCheckOpen );
	virtual S32	CallSide( S32 cmd, void *pParam );

	virtual void Init( );
	void CleanUp();

	S32 Open ( U32 flag, void *ptr );
	S32 Close( );

	S32	puList( BRD_PuList *pList, U32 item, U32 *pItemReal );
	S32 puLoad( U32 puId, const BRDCHAR *fileName, U32 *state );

	S32	ServiceList( BRD_ServList *pList, U32 item, U32 *pItemReal );

	S32 isFirstOpen();

	DECLARE_ENTRY(TBaseProxy);

};

class TSideProxy : public TModuleProxy
{
public:

	TModule *pTech;
	void *pBaseDev;//FIXME: dev of base device

	DEVS_API_entry		*m_pDevsEntry;	

	void Init( );

	void SetServList();

	S32	CallDriver( S32 cmd, void *pParam, S32 isCheckOpen );



};

class TSrv
{
private:
	TString _name;
public:

	TSrv()
	{
		attr = 0;
		name = NULL;
	}

	BRDCHAR defName[32];

	BRDCHAR *name;
	U32 attr;

	TArray<TSrv*> sub;
	TSrv *stream;

	TModule *owner;

    virtual S32 Ctrl( S32 cmd,void *pParam ) = 0;
	
	virtual S32 Capture();
};

class TSrvProxy : public TSrv
{
public:
	TModuleProxy *m_pDevice;
	S32 m_index;

	HANDLE			m_hEvent;		// Protection Data Event
	HANDLE			m_hMutex;		// Protection Data Mutex

	
	
	S32 Open();
	S32 Close();

	S32 Lock();
	S32 Unlock();
	S32 isLocked();

	void Release(){ delete this; };
	S32 Ctrl( S32 cmd,void *pParam );

	S32 isFirstOpen();
	
	S32 Capture();
};

template<class T> T* staticConstructor()
{
	return new T();
}

typedef TBaseProxy* (*StaticConstructor)();

TBaseProxy *CreateBaseDrv( BRDCHAR *dllName, short CurDevNum, TDict *pDict, StaticConstructor staticConstructor=NULL );

template<class T> T *CreateBaseDrv( BRDCHAR *dllName, TDict *pDict )
{
    return (T*) CreateBaseDrv( dllName, 0, pDict, (StaticConstructor)&staticConstructor<T> );
}

TSideProxy *CreateSideDrv( BRDCHAR *dllName, void *pBaseDev, TModule *pBase, TDict *pDict );
TSrvProxy *CreateSrv( BRDCHAR *srvName, TModule *pDevice );

#endif
