/*
 BARDY hack
*/


#define DEVS_API_EXPORTS
#include "tech.h"
#include "dict.h"

#include "brdapi.h"
#include "gipcy.h"

TBRD_InitData initData[32];

TBRD_InitData *makeIniData( TDict *pDict, int *size )
{
	int len = pDict->getPropCount();
	int cnt = 0;


	for( int i=0; i<len; i++)
	{
		TBRD_InitData *prop = pDict->getProp( i );

		BRDC_strcpy( initData[i].key, prop->key );
		BRDC_strcpy( initData[i].val, prop->val );

		cnt++;
	}

	int dictlen = pDict->getDictCount();

	for( int j=0; j<dictlen; j++)
	{
		TDict *pSubDict = pDict->getDict( j );

		BRDC_strcpy( initData[cnt].key, _BRDC("#begin")  );
		BRDC_strcpy( initData[cnt].val, pSubDict->name  );
		cnt++;

		len = pSubDict->getPropCount();
		for( int i=0; i<len; i++)
		{
			TBRD_InitData *prop = pSubDict->getProp( i );

			BRDC_strcpy( initData[cnt].key, prop->key );
			BRDC_strcpy( initData[cnt].val, prop->val );

			cnt++;
		}

		BRDC_strcpy( initData[cnt].key, _BRDC("#end") );
		cnt++;

	}

	*size = cnt;

	return (TBRD_InitData*)&initData;
}



TBaseProxy *CreateBaseDrv( BRDCHAR *dllName, short CurDevNum, TDict *pDict, StaticConstructor staticConstructor )
{


	DEV_API_entry		*pDEV_Entry;
	DEV_API_initData	*pDEV_initData;
	DEVS_API_entry		*pDEVS_Entry;

	TCHAR			boardName[BOARDNAME_SIZE];	// Device Name
	void *pDev;

	IPC_handle hLib;

	// Open Library

	//hLib = LoadLibrary( dllName );

#ifdef _WIN64

	BRDCHAR win64name[64];

	BRDC_strcpy( win64name, dllName );
	BRDC_strcat( win64name, _BRDC("64"));

	dllName = win64name;

#endif

	hLib = IPC_openLibrary( dllName, 0 );

    if(hLib == 0)
		return NULL;

	// Get Entry Point
#ifdef _WIN64
	pDEV_initData	= (DEV_API_initData*)IPC_getEntry( hLib, "DEV_initData" );
	pDEV_Entry		= (DEV_API_entry*)IPC_getEntry( hLib, "DEV_entry" );
	pDEVS_Entry		= (DEVS_API_entry*)IPC_getEntry( hLib, "DEVS_entry" );
#else
	pDEV_initData	= (DEV_API_initData*)IPC_getEntry( hLib, "_DEV_initData@20" );
	pDEV_Entry		= (DEV_API_entry*)IPC_getEntry( hLib, "_DEV_entry@12" );
	pDEVS_Entry		= (DEVS_API_entry*)IPC_getEntry( hLib, "_DEVS_entry@16" );
#endif

	if(!pDEV_initData || !pDEV_Entry)
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

	int size = 0;
	TBRD_InitData *pInitData = makeIniData( pDict, &size );

	S32 type = (pDEV_initData)(CurDevNum, &pDev, boardName, pInitData, size );

	if(type < 0)
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

	BRDCHAR	szName[64];
    IPC_handle		hMutex;		// Mutex for Multythread Operation
	TBoardSharedInfo *pBSI;		// Pointer      to Keep Open Protection Shareable Information
    IPC_handle		hFileMap;	// File Mapping to Keep Open Protection Shareable Information

	BRDC_strcpy( szName, boardName );
	BRDC_strcat( szName, _BRDC("_Mutex"));

    hMutex = IPC_createMutex( szName, 0 );

	BRDC_strcpy( szName, boardName );
	BRDC_strcat( szName, _BRDC("_FileMap"));
    hFileMap = IPC_createSharedMemory( szName, sizeof(*pBSI) );

	if( hFileMap == 0 )
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

    pBSI = (TBoardSharedInfo *)IPC_mapSharedMemory( hFileMap );

	if( pBSI == NULL )
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

    IPC_captureMutex( hMutex, -1 );

	TBaseProxy *pDevice = NULL;

	if( staticConstructor )
		pDevice = (TBaseProxy*)staticConstructor();
	else
		pDevice = new TBaseProxy();

	BRDC_strcpy( pDevice->boardName, boardName );

	pDevice->m_hLib = hLib;
	pDevice->m_pEntry = (void*)pDEV_Entry;
	pDevice->m_pSideEntry = (void*)pDEVS_Entry;
	pDevice->m_pDev = pDev;

	pDevice->hMutex = hMutex;
	pDevice->pBSI = pBSI;
	pDevice->hFileMap = hFileMap;

	pDevice->Init();
	pDevice->CreateServList();

	if( pBSI->openCnt == 0 )
	{
		DEV_CMD_DevHardwareInit	sDHI = {(U32)NODEALL};
		pDevice->CallDriver( DEVcmd_DEVHARDWAREINIT, &sDHI, 0 );
	}

	pBSI->openCnt++;

    IPC_releaseMutex( hMutex );

	return pDevice;
}

DEFINE_ENTRY_MAP( TBaseProxy )
{
	DEFINE_ENTRY( TBaseProxy, DEVcmd_INIT, Init, "iii" )
};

IMPLEMENT_ENTRY(TModuleProxy,TBaseProxy);

S32 TModule::DoCtrl( TModuleEntry *pCmdArray, int nCmd,S32 cmd,void *pParam )
{
	TModuleEntry *pEntry = NULL;
	char *pParamBytes = (char*)pParam;

	for( int i=0; i<nCmd; i++ )
	{
		if( pCmdArray[i].cmd == cmd )
		{
			pEntry = &pCmdArray[i];
			break;
		}
	}

	if( pEntry == NULL )
		return BRDerr_FUNC_UNIMPLEMENTED;

    char *nt = pEntry->params;

	void *aParams[32];
	int nParam = 0;

    while( *nt )
	{
        switch(*nt)
		{
		case 'i':
			{
				aParams[nParam] = (void*)(*(int*)pParamBytes);

				pParamBytes+=sizeof(int);

				break;
			}
		}

		nParam++;
        nt++;
	}

	switch(nParam)
	{
		case 3:
		{
			pt2Member pm = horrible_cast<pt2Member>(pEntry->method);
			((TModule*)this->*pm)(aParams[0],aParams[1],aParams[2]);

			break;
		}
	}

	return 0;
}

#include "icr.h"

TSideProxy *CreateSideDrv( BRDCHAR *dllName, void *pBaseDev, TModule *pBase, TDict *pDict )
{
	short CurDevNum = 0;

	SIDE_API_entry		*pDEV_Entry;
	SIDE_API_initData	*pDEV_initData;

	//TCHAR			boardName[BOARDNAME_SIZE];	// Device Name
	void *pDev;

	IPC_handle hLib;

#ifndef __linux__
	// Open Library
    hLib = IPC_openLibraryEx(dllName, 0);
    if(hLib == 0)
		return NULL;
#else
#ifdef	__IPC_LINUX__

    char *libAbsNames[3] = {0,0,0};
    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        libAbsNames[i] = (char*)malloc(PATH_MAX);
    }

    // prepare first case if library placed in current directory
    if(libAbsNames[0]) {
        const char *currdir = getcwd(libAbsNames[0], PATH_MAX);
        if(currdir) {
            unsigned dirlen = strlen(libAbsNames[0]);
            snprintf(libAbsNames[0]+dirlen, PATH_MAX-dirlen, "%s%s%s", "/lib", dllName, ".so");
        }
    }

    // prepare second case if library placed in pointed BARDYLIB environment variable
    if(libAbsNames[1]) {
        const char *envpath = getenv("BARDYLIB");
        if(envpath) {
            snprintf(libAbsNames[1], PATH_MAX, "%s%s%s%s", envpath, "/lib", dllName, ".so");
        } else {
            free(libAbsNames[1]);
            libAbsNames[1] = 0;
        }
    }

    // prepare third absname in predefined directory "/usr/local/lib/bardy/lib"
    if(libAbsNames[2]) {
        snprintf(libAbsNames[2], PATH_MAX, "%s%s%s", "/usr/local/lib/bardy/lib", dllName, ".so");
    }

    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        //fprintf(logfile, "TRY LIBRARY: ---- %s\n", libAbsNames[i]);
        if(libAbsNames[i]) {
            hLib = IPC_openLibrary(libAbsNames[i], 0);
            if(!hLib) {
                continue;
            } else {
                break;
            }
        }
    }

    for(unsigned i=0; i<sizeof(libAbsNames)/sizeof(libAbsNames[0]); i++) {
        if(libAbsNames[i]) {
            free(libAbsNames[i]);
        }
    }
#endif
#endif

	// Get Entry Point
	pDEV_initData	= (SIDE_API_initData*)IPC_getEntry( hLib, "SIDE_initData" );
	pDEV_Entry		= (SIDE_API_entry*)IPC_getEntry( hLib, "SIDE_entry" );

	if(!pDEV_initData || !pDEV_Entry)
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

	int size = 0;
	TBRD_InitData *pInitData = makeIniData( pDict, &size );

	U32 baseIcrSize = 0;
	char base_icr[512];

	memset( base_icr, 0, sizeof(base_icr) );

	pBase->readIcr( base_icr, BASE_ID_TAG, SUBMOD_CFGMEM_SIZE, &baseIcrSize );

	U32 subIcrSize = 0;
	char sub_icr[SUBMOD_CFGMEM_SIZE];

	memset( sub_icr, 0, sizeof(sub_icr) );

	pBase->readIcr( sub_icr, ADM_ID_TAG, SUBMOD_CFGMEM_SIZE, &subIcrSize );

	S32 type = 0;

	{
		int icrSize = baseIcrSize+subIcrSize;

		if( icrSize <= 0 )
			icrSize = 32;

		char *icr = (char*)malloc( sizeof(char)*(icrSize) );

		memcpy( icr,base_icr,baseIcrSize);
		memcpy( icr+baseIcrSize,sub_icr,subIcrSize);

		//HACK HACK HACK
		*((PUSHORT)icr + 2) = icrSize;

		type = (pDEV_initData)( &pDev, icr, pInitData, size );
		free( icr );
	}

	if(type < 0)
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

	BRDCHAR	szName[64];
    IPC_handle		hMutex;		// Mutex for Multythread Operation
	TBoardSharedInfo *pBSI;		// Pointer      to Keep Open Protection Shareable Information
    IPC_handle		hFileMap;	// File Mapping to Keep Open Protection Shareable Information

	BRDC_strcpy( szName, pBase->boardName );
	BRDC_strcat( szName, _BRDC("_Mutex"));

    hMutex = IPC_createMutex( szName, 0 );

	if (pBase->hMutex == 0)
		pBase->hMutex = hMutex;



	BRDC_strcpy( szName, pBase->boardName );
	BRDC_strcat( szName, _BRDC("_FileMap"));
    hFileMap = IPC_createSharedMemory( szName, sizeof(*pBSI) );

	if( hFileMap == 0 )
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

    pBSI = (TBoardSharedInfo *)IPC_mapSharedMemory( hFileMap );

	if( pBSI == NULL )
	{
		IPC_closeLibrary(hLib);
		return NULL;
	}

	TSideProxy *pDevice = new TSideProxy();

	pDevice->hMutex = hMutex;

	BRDC_strcpy( pDevice->boardName, pBase->boardName );

	pDevice->m_hLib = hLib;
    pDevice->m_pEntry = (void*)pDEV_Entry;
	pDevice->m_pDev = pDev;

	pDevice->pTech = (TModule*)pBase;
	pDevice->pBaseDev = pBaseDev;

	pDevice->Init();
	pDevice->CreateServList();
	pDevice->SetServList();

	return (TSideProxy*)pDevice;
}

TSrvProxy *CreateSrv( BRDCHAR *srvName, TModule *pDevice )
{
    IPC_handle			hEvent,hMutex;		// File Mapping to Keep Open Protection Shareable Information

	BRDCHAR	szName[64];

	BRDC_sprintf( szName, _BRDC("%s%s_Ev"), srvName, pDevice->boardName );
    hEvent = IPC_createEvent( szName, TRUE, FALSE );

	if( hEvent == 0 )
	{
		return NULL;
	}

	BRDC_sprintf( szName, _BRDC("%s%s_MX"), srvName, pDevice->boardName );
    hMutex = IPC_createMutex( szName, 0 );

	if( hMutex == 0 )
	{
		return NULL;
	}

	TSrvProxy *pSrv = new TSrvProxy();
	pSrv->m_hEvent = hEvent;
	pSrv->m_hMutex = hMutex;

	return pSrv;
}

S32 TModuleProxy::GetInfo( BRD_Info *pInfo )
{
	DEV_CMD_Getinfo param;

	param.pInfo = pInfo;

	return CallDriver( DEVcmd_GETINFO, &param, 1 );
}


void TModuleProxy::CreateServList()
{
	DEV_CMD_GetServList	sGRL = {0,0,0};
	DEV_CMD_ServListItem	*pSLI;

	//
	// Get Number of Services (into "sGRL.itemReal")
	//
	CallDriver( DEVcmd_GETSERVLIST, &sGRL, 0 );

	//
	// If There are no Services on The Board
	//
	if( sGRL.itemReal==0 )
		return;

	//
	// Get Service Information
	//
	pSLI = (DEV_CMD_ServListItem*)calloc( sizeof(DEV_CMD_ServListItem), sGRL.itemReal );

	if( pSLI==NULL )
		return;

	sGRL.pSLI = pSLI;
	sGRL.item = sGRL.itemReal;

	CallDriver( DEVcmd_GETSERVLIST, &sGRL, 0 );

	m_pServList = (TSrvInfo*)calloc( sizeof(TSrvInfo), sGRL.itemReal );
	if( m_pServList==NULL )
	{
		free( pSLI );
		return;
	}
	m_nServListSize = sGRL.itemReal;

	//
	// Fill Service Support Table
	//
	for( int ii=0; ii<m_nServListSize; ii++ )
	{
		BRDC_strcpy( m_pServList[ii].name, pSLI[ii].name );
		m_pServList[ii].attr = pSLI[ii].attr;
		m_pServList[ii].idxSuperServ = 0;

		//m_pServList[ii].owner = this;
	}

	free( pSLI );
}

void TModuleProxy::ReleaseServList()
{
	if( m_pServList!=NULL )
		free( m_pServList );

	m_nServListSize = 0;
}


TSrvProxy  *TModuleProxy::InitSrv( U32 *pMode, const BRDCHAR *resName, int num,  U32 timeout )
{
	//DEV_CMD_Capture	paramCapt;

	TSrvProxy *pSrv = NULL;

	int itemID = -1;
	for( int ii=0; ii<m_nServListSize && ii<0x7F; ii++ )
	{
		if( BRDC_strcmp( m_pServList[ii].name, resName ) == 0 )
		{
			itemID = ii;
			break;
		}

	}

	if( itemID == -1 )
		return 0;

	S32 ret = 0;






	pSrv = CreateSrv( (BRDCHAR*)resName, this );

	if( pSrv == NULL )
		return 0;

	pSrv->m_index = itemID;
	pSrv->m_pDevice = (TModuleProxy*)this;

	BRDC_strcpy( pSrv->defName, resName );

	pSrv->name = (BRDCHAR*)pSrv->defName;
	pSrv->attr = m_pServList[itemID].attr;

	return pSrv;
}

TSrvProxy  *TModuleProxy::ProxySrv( U32 *pMode, const BRDCHAR *resName, int num, U32 timeout )
{
	TSrvProxy *pSrv = InitSrv(pMode,resName,0, timeout);

	if( pSrv )
	{
		SERV_CMD_IsAvailable av;

		pSrv->Ctrl( SERVcmd_SYS_ISAVAILABLE, &av );

		//pSrv->Ctrl( SERVcmd_SYS_CAPTURE, 0 );
	}

	return pSrv;
}

TBaseProxy::TBaseProxy()
{

}

TBaseProxy::~TBaseProxy()
{
	ReleaseServList();



	CleanUp();
}

S32		TBaseProxy::CallDriver( S32 cmd, void *pParam, S32 isCheckOpen )
{
    IPC_captureMutex( hMutex, -1 );

	S32 err = ( (DEV_API_entry*) m_pEntry)( m_pDev, cmd, pParam );

    IPC_releaseMutex( hMutex );

	return err;
}

S32		TBaseProxy::CallSide( S32 cmd, void *pParam )
{
    IPC_captureMutex( hMutex, -1 );

	S32 err = ( (DEVS_API_entry*) m_pSideEntry)( m_pDev, NULL, cmd, pParam );

    IPC_releaseMutex( hMutex );

	return err;
}

void		TBaseProxy::Init( )
{
	CallDriver( DEVcmd_INIT, NULL, 0 );
	//CallDriver( DEVcmd_DEVHARDWAREINIT, NULL, 0 );
}

void		TBaseProxy::CleanUp( )
{


	//CallDriver( DEVcmd_DEVHARDWARECLEANUP, NULL, 1 );

	IPC_interlockedDecrement(&pBSI->openCnt);

	CallDriver( DEVcmd_CLEANUP, NULL, 1 );
}

/*
 BRD funx
*/



S32 TBaseProxy::Open ( U32 flag, void *ptr )
{
	struct { U32 flag; void *ptr; S32 isFirstOpen; } param;

	param.flag = flag;
	param.ptr  = ptr;

	if( pBSI->openCnt > 0 )
		param.isFirstOpen = 0;
	else
		param.isFirstOpen = 1;

	S32 ret = CallDriver( DEVcmd_OPEN, &param, 0 );
	pBSI->openCnt++;

	return ret;
}

S32 TBaseProxy::isFirstOpen (  )
{
	return (pBSI->openCnt <= 1);
}

S32 TBaseProxy::Close( )
{
	S32 ret = CallDriver( DEVcmd_CLOSE, 0, 1 );
	pBSI->openCnt--;

	return BRDerr_OK;
}

S32	TBaseProxy::puList( BRD_PuList *pList, U32 item, U32 *pItemReal )
{
	DEV_CMD_PuList param;

	param.pList = pList;
	param.item  = item;
	param.pItemReal = pItemReal;

	return CallDriver( DEVcmd_PULIST, &param, 1 );
}

S32 TBaseProxy::puLoad( U32 puId, const BRDCHAR *fileName, U32 *state )
{
	DEV_CMD_PuLoad	param;
	U32				aState;

	param.puId = puId;
	param.fileName = fileName;
	param.state = (state) ? state : &aState;

	return CallDriver( DEVcmd_PULOAD, &param, 1 );
}

S32	TBaseProxy::ServiceList( BRD_ServList *pList, U32 item, U32 *pItemReal )
{
	U32		itemReal;

	itemReal = 0;
	for( int ii=0; ii<m_nServListSize && ii<0x7F; ii++ )
	{
		DEV_CMD_Ctrl			param;
		SERV_CMD_IsAvailable	sSA;

		//
		// Check Service Visibility
		//
		if( m_pServList[ii].attr & BRDserv_ATTR_UNVISIBLE )
			continue;

		//
		// Check Service Avialability
		//
		param.handle = (ii<<16);
		param.nodeId = 0;
		param.cmd = SERVcmd_SYS_ISAVAILABLE;
		param.arg = &sSA;
		CallDriver( DEVcmd_CTRL, &param, 1 );

		if( sSA.isAvailable==0 )
			continue;
		//
		// Put Service to List
		//
		if( (itemReal<item) && (pList!=NULL) )
		{
			BRDC_strcpy( pList[itemReal].name, m_pServList[ii].name );
			pList[itemReal].attr = (sSA.attr) ? sSA.attr : m_pServList[ii].attr;
		}
		itemReal++;
	}

	if( pItemReal != NULL )
		*pItemReal = itemReal;

	return BRDerr_OK;
}

void TSideProxy::Init( )
{
	SIDE_CMD_Init init;

	BRDC_sprintf( init.boardName , pTech->boardName );

	init.hMutex = pTech->hMutex;
	init.pDev = pBaseDev;
	init.pEntry = (DEVS_API_entry*)pTech->m_pSideEntry;//DEVS_entry;

	CallDriver( SIDEcmd_INIT, &init, 1 );
}

void TSideProxy::SetServList()
{
	//
	// Get Service Information
	//
	DEV_CMD_ServListItem *pSLI = (DEV_CMD_ServListItem*)calloc( sizeof(DEV_CMD_ServListItem), m_nServListSize );

	if( pSLI==NULL )
		return;

	//
	// Fill Service Support Table
	//

	U32 devId = 0;



	for( int ii=0; ii<m_nServListSize; ii++ )
	{
		devId = 0;

		int len = BRDC_strlen( m_pServList[ii].name );

		for( int jj=0; jj<ii; jj++ )
		{
			if( BRDC_strncmp( m_pServList[ii].name, m_pServList[jj].name,  len) == 0 )
				devId++;
		}

		BRDCHAR		name[SERVNAME_SIZE];// Service name with Number

		BRDC_sprintf( name, _BRDC("%sBUNI%d"), m_pServList[ii].name, devId );

		BRDC_strcpy( pSLI[ii].name, name );

		BRDC_sprintf(name, _BRDC("%s%d"), m_pServList[ii].name, devId);

		BRDC_sprintf(m_pServList[ii].name, name );

		pSLI[ii].attr = m_pServList[ii].attr;
		pSLI[ii].idxMain = ii;
	}

	DEV_CMD_GetServList	sGRL = {0,0,0};

	sGRL.pSLI = pSLI;
	sGRL.item = m_nServListSize;

	CallDriver( SIDEcmd_SETSERVLIST, &sGRL, 0 );

	free( pSLI );
}






int side_cmd_remap[] =
{
	0,
	0,
	0,
	0,
	DEVcmd_GETINFO,
	0,
	DEVcmd_PUSTATE,
	DEVcmd_PULIST,
	DEVcmd_VERSION,
	DEVcmd_CAPTURE,
	DEVcmd_RELEASE,
	DEVcmd_CTRL,

	DEVcmd_DEVHARDWAREINIT,
	DEVcmd_DEVHARDWARECLEANUP,
	0,
	DEVcmd_GETSERVLIST,
	DEVcmd_PUREAD,
	DEVcmd_PUWRITE,

	0
};

S32	TSideProxy::CallDriver( S32 cmd, void *pParam, S32 isCheckOpen )
{

	int nn = sizeof( side_cmd_remap )/sizeof(side_cmd_remap[0]);

	for( int i=0; i<nn; i++ )
	{
		if( side_cmd_remap[i] == cmd )
		{
			cmd = i;
			break;
		}
	}


    IPC_captureMutex( hMutex, -1 );

	S32 err = ((SIDE_API_entry*)m_pEntry)( m_pDev, 0, cmd, pParam );

    IPC_releaseMutex( hMutex );

	return err;
}

S32 TSrv::Capture()
{

	for( int i=0; i<sub.get_length(); i++ )
	{
		TSrv *psub = sub[i];

		if( psub )
		{
			S32 ret = psub->Capture();
		}
	}

	return 0;
}
