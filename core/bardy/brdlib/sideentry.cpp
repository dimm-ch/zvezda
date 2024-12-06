//***************** File Entry.cpp *********************
//
// Definitions of functions, processed CMD
//
// (C) InSys by Dorokhin A. Nov 2003
//
//******************************************************

#define	SIDE_API_EXPORTS
//#define	SIDE_API		DllExport

#include	"sidedll.h"

//*************** Prepare SIDE_entry *******************
//*****************************************************
typedef S32 CMD_EntryPoint( SIDE_CLASS *pDev, void *pParam );

S32 CMD_init          ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_cleanup       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_open          ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_close         ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_getinfo       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_puLoad        ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_puState       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_puList        ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_puRead        ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_puWrite       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_version       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_capture       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_release       ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_ctrl          ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_hwInit        ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_hwCleanup     ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_getServList   ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_setServList   ( SIDE_CLASS *pDev, void *pParam );
S32 CMD_servCmdQuery  ( SIDE_CLASS *pDev, void *pParam );

CMD_EntryPoint	*g_entryPointArr[DEVcmd_MAX] =
{
	CMD_init          ,
	CMD_cleanup       ,
	CMD_open          ,
	CMD_close         ,
	CMD_getinfo       ,
	CMD_puLoad        ,
	CMD_puState       ,
    CMD_puList        ,
	CMD_version       ,
	CMD_capture       ,
	CMD_release       ,
	CMD_ctrl          ,
	CMD_hwInit		  ,
	CMD_hwCleanup     ,
	CMD_setServList	  ,
	CMD_getServList	  ,
	CMD_puRead        ,
	CMD_puWrite       ,
	CMD_servCmdQuery
};

//************************ SIDE_entry *******************
//*******************************************************
//SIDE_API S32  STDCALL SIDE_entry( void *ptrSub, S32 resIdx, void *pDev, void *entry, S32 cmd, void *pParam )
SIDE_API S32  STDCALL SIDE_entry( void *pSub, void *pContext, S32 cmd, void *args )
{
	if( cmd<0 )
		return BRDerr_FUNC_UNIMPLEMENTED;
	if( cmd>(S32)(sizeof(g_entryPointArr)/sizeof(g_entryPointArr[0])) )
		return BRDerr_FUNC_UNIMPLEMENTED;
	if( g_entryPointArr[cmd] == NULL )
		return BRDerr_FUNC_UNIMPLEMENTED;
	return g_entryPointArr[cmd]( (SIDE_CLASS*)pSub, args );
}
/*
DEV_API S32		STDCALL DEV_entry( void *pDev, S32 cmd, void *pParam )
{
	if( cmd<0 )
		return SIDEERR(BRDerr_FUNC_UNIMPLEMENTED);
	if( cmd>sizeof(g_entryPointArr)/sizeof(g_entryPointArr[0]) )
		return SIDEERR(BRDerr_FUNC_UNIMPLEMENTED);
	if( g_entryPointArr[cmd] == NULL )
		return SIDEERR(BRDerr_FUNC_UNIMPLEMENTED);
	return g_entryPointArr[cmd]( (SIDE_CLASS*)pDev, pParam );
}
*/
//********************** CMD_init *********************
//*****************************************************
S32 CMD_init( SIDE_CLASS *pDev, void *pParam )
{
	PSIDE_CMD_Init ptr = (PSIDE_CMD_Init)pParam;

	pDev->BaseInfoInit(ptr->pDev, ptr->pEntry, ptr->boardName);

	return BRDerr_OK;
}

//=********************** CMD_cleanup ******************
//=*****************************************************
S32 CMD_cleanup( SIDE_CLASS *pDev, void *pParam )
{
	//
	// TODO: Remove Device Date and Free Memory
	//

//	pDev->CleanUp();
//	HeapFree( GetProcessHeap(), 0, pDev );
	delete pDev;

	return BRDerr_OK;
}

//=********************** CMD_open ******************
//=*****************************************************
S32 CMD_open( SIDE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Open ptr = (PDEV_CMD_Open)pParam;

	//
	// TODO: Check BRDopen_SHARE flag
	//
	if( ptr->flag & BRDopen_SHARED )
	{
		// If Device is already opened without BRDopen_SHARE flag
		//	return SIDEERR(BRDerr_DEVICE_UNSHAREABLE);
	}
	else
	{
		// If Device is already opened
		//	return SIDEERR(BRDerr_DEVICE_UNSHAREABLE);
	}
	//
	// TODO: Make open operation
	//

	return BRDerr_OK;
}

//=********************** CMD_close ********************
//=*****************************************************
S32 CMD_close( SIDE_CLASS *pDev, void *pParam )
{
	//
	// TODO: Make close operation
	//

	return BRDerr_OK;
}

//=********************** CMD_getinfo ******************
//=*****************************************************
S32 CMD_getinfo( SIDE_CLASS *pDev, void *pParam )
{
//	PSIDE_CMD_Getinfo ptr = (PSIDE_CMD_Getinfo)pParam;
	PDEV_CMD_Getinfo ptr = (PDEV_CMD_Getinfo)pParam;

	// Check and Clear BRD_Info structure
//	if( ptr->pInfo->size < sizeof(ADM_Info) )
	if( ptr->pInfo->size < sizeof(BRD_Info) )
		return BRDerr_BUFFER_TOO_SMALL;
//	memset( &(ptr->pInfo->type), 0, ptr->pInfo->size - sizeof(ptr->pInfo->size) );
	memset( &(ptr->pInfo->boardType), 0, ptr->pInfo->size - sizeof(ptr->pInfo->size) );
	

	//
	// TODO: Fill BRD_Info structure
	//
	pDev->GetDeviceInfo(ptr->pInfo);

	return BRDerr_OK;
}

//=********************** CMD_puLoad *******************
//=*****************************************************
S32 CMD_puLoad( SIDE_CLASS *pDev, void *pParam )
{
	DEV_CMD_PuLoad *ptr = (DEV_CMD_PuLoad*)pParam;

	return pDev->PuFileLoad(
							ptr->puId, 
							(BRDCHAR*)ptr->fileName, 
							ptr->state
							);
}

//=********************** CMD_puState ******************
//=*****************************************************
S32 CMD_puState(SIDE_CLASS *pDev, void *pParam )
{
	DEV_CMD_PuState	*ptr = (DEV_CMD_PuState*)pParam;

	return pDev->GetPuState(
							ptr->puId, 
							ptr->state
							);
}

//=********************** CMD_puList *******************
//=*****************************************************
S32 CMD_puList(SIDE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_PuList	ptr = (PDEV_CMD_PuList)pParam;

	*(ptr->pItemReal) = pDev->GetPuCnt();

	// Check List Size
	if(ptr->item < *(ptr->pItemReal))
		return BRDerr_BUFFER_TOO_SMALL;

	// Fill List
	pDev->GetPuList(ptr->pList);

	return BRDerr_OK;
}

//=********************** CMD_puRead *******************
//=*****************************************************
S32 CMD_puRead(SIDE_CLASS *pDev, void *pParam )
{
/*	PDEV_CMD_PuRW ptr = (PDEV_CMD_PuRW)pParam;

	ULONG puNum = ptr->id;
	switch(puNum)
	{
	case 1:
		return SIDEERR(BRDerr_BAD_PARAMETER);
		break;
	case 2:
		pDev->ReadNvRAM(ptr->pBuf,ptr->size, ptr->offset + 0x80);
		break;
	case 3:
		pDev->ReadAdmIdROM(ptr->pBuf,ptr->size, ptr->offset);
		break;
	}*/
	return BRDerr_OK;
}

//=********************** CMD_puRead *******************
//=*****************************************************
S32 CMD_puWrite(SIDE_CLASS *pDev, void *pParam )
{
/*	PDEV_CMD_PuRW ptr = (PDEV_CMD_PuRW)pParam;

	ULONG puNum = ptr->id;
	switch(puNum)
	{
	case 1:
		return SIDEERR(BRDerr_BAD_PARAMETER);
		break;
	case 2:
		pDev->WriteNvRAM(ptr->pBuf,ptr->size, ptr->offset + 0x80);
		break;
	case 3:
		pDev->WriteAdmIdROM(ptr->pBuf,ptr->size, ptr->offset);
		break;
	}*/
	return BRDerr_OK;
}

//=********************** CMD_servCmdQuery **********
//=*****************************************************
S32 CMD_servCmdQuery( SIDE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Ctrl ptr = (PDEV_CMD_Ctrl)pParam;

	//
	// TODO: Make operation
	//
	S32 status = BRDerr_OK;
	status = pDev->CmdQuery(ptr->handle, ptr->cmd);

	return status;
}

//=********************** CMD_capture ******************
//=*****************************************************
S32 CMD_capture( SIDE_CLASS *pDev, void *pParam )
{
	//PDEV_CMD_Ctrl ptr = (PDEV_CMD_Ctrl)pParam;

	//
	// TODO: Make Resource operation
	//
	//S32 status = BRDerr_OK;
//	status = pDev->UnderSrvCapture(ptr->handle, ptr->cmd);
	//
	// It is a dummy function (may be)
	//

	return BRDerr_FUNC_UNIMPLEMENTED;

//	return SIDEERR(status);
}

//=********************** CMD_release ******************
//=*****************************************************
S32 CMD_release( SIDE_CLASS *pDev, void *pParam )
{
	//
	// It is a dummy function (may be)
	//

	return BRDerr_FUNC_UNIMPLEMENTED;
}

//=********************** CMD_ctrl *********************
//=*****************************************************
S32 CMD_ctrl( SIDE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Ctrl ptr = (PDEV_CMD_Ctrl)pParam;

	//
	// TODO: Check Resource Index and Capture Mode
	//

	//
	// TODO: Make Resource operation
	//
	S32 status = BRDerr_OK;
	UNDERSERV_Cmd new_cmd = {0, NULL};
	status = pDev->SrvCtrl(ptr->handle, ptr->cmd, ptr->arg, &new_cmd);
	if(new_cmd.code)
	{
		ptr->cmd = new_cmd.code;
		ptr->arg = new_cmd.pParams;
	}

/*
	switch(ptr->cmd)
	{
		case SYScmd_SERV_AVAILABLE:
		{
			PSYS_CMD_ServAvailable pSrvAvailable = (PSYS_CMD_ServAvailable)ptr->arg;
			status = pDev->IsSrvAvailable(ptr->handle, &pSrvAvailable->isAvailable, &pSrvAvailable->attr);
			break;
		}
		default:
			status = pDev->SrvCtrl(ptr->handle, ptr->cmd, ptr->arg);
//			return BRDerr_CMD_UNSUPPORTED;
	}
*/

//	return SIDEERR(status);
	return status;
}

//=********************** CMD_version ******************
//=*****************************************************
S32 CMD_version( SIDE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Version ptr = (PDEV_CMD_Version)pParam;

	//
	// TODO: Get Product Version
	//

	ptr->pVersion->drvMajor = VER_MAJOR;
	ptr->pVersion->drvMinor = VER_MINOR;

	return BRDerr_OK;
}

//=********************** CMD_hwInit **********
//=*****************************************************
S32 CMD_hwInit( SIDE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_SubHardwareInit	*ptr = (DEV_CMD_SubHardwareInit*)pParam;

	//
	// TODO: Make operation
	//

	return BRDerr_OK;
}

//=********************** CMD_hwCleanup *******
//=*****************************************************
S32 CMD_hwCleanup( SIDE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_SubHardwareCleanup	*ptr = (DEV_CMD_SubHardwareCleanup*)pParam;

	//
	// TODO: Make operation
	//

	return BRDerr_OK;
}
/*
//=********************** CMD_getResList ***************
//=*****************************************************
S32 CMD_getServList( SIDE_CLASS *pDev, void *pParam )
{
	DEV_CMD_GetServList	*ptr = (DEV_CMD_GetServList*)pParam;

	ptr->itemReal = 0; // None Resources

	return SIDEERR(BRDerr_OK);
}
*/
/*
//=********************** CMD_getServList ***************
//=*****************************************************
S32 CMD_getServList( SIDE_CLASS *pDev, void *pParam )
{
	DEV_CMD_GetServList	*ptr = (DEV_CMD_GetServList*)pParam;

    if(ptr == NULL )
        return SIDEERR(BRDerr_BAD_PARAMETER);

//	ptr->itemReal = 0; // None Services

	ptr->itemReal = pDev->GetSrvCnt();

	// Check List Size
	if(ptr->item < ptr->itemReal)
		return SIDEERR(BRDerr_BUFFER_TOO_SMALL);

	// Fill List
	pDev->GetSrvList(ptr->pList);

//    return Serv_GetServList( pDev, ptr );
	return SIDEERR(BRDerr_OK);
}
*/
S32 CMD_getServList( SIDE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_SrvList	*ptr = (DEV_CMD_SrvList*)pParam;
	PSIDE_CMD_GetServList ptr = (PSIDE_CMD_GetServList)pParam;

    if(ptr == NULL )
        return BRDerr_BAD_PARAMETER;

//	ptr->itemReal = 0; // None Services

    ptr->itemReal = pDev->GetSrvCnt();

	// Check List Size
	if(ptr->item < ptr->itemReal)
		return BRDerr_BUFFER_TOO_SMALL;

	// Fill List
//	pDev->GetSrvList(ptr->pList);
	pDev->GetServ(ptr->pSLI);

//    return Serv_GetServList( pDev, ptr );
	return BRDerr_OK;
}
//=********************** CMD_setServList ***************
//=*****************************************************
S32 CMD_setServList( SIDE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_GetServList	*ptr = (DEV_CMD_GetServList*)pParam;
	PSIDE_CMD_GetServList ptr = (PSIDE_CMD_GetServList)pParam;

    if(ptr == NULL )
        return BRDerr_BAD_PARAMETER;

//	ptr->itemReal = 0; // None Services

    ptr->itemReal = pDev->GetSrvCnt();

	// Check List Size
	if(ptr->item < ptr->itemReal)
		return BRDerr_BUFFER_TOO_SMALL;

	// Fill Main List Index
	pDev->SetSrvList(ptr->pSLI);

//    return Serv_GetServList( pDev, ptr );
	return BRDerr_OK;
}

//***************** End of file Entry.cpp *****************
