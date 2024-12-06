/*
 ***************** File Entry.cpp *********************
 *
 * Definitions of functions, processed CMD
 *
 * (C) InSys by Dorokhin A. Jul 2002
 *
 ******************************************************
*/

#define	DEV_API_EXPORTS
//#define	DEV_API		DllExport

//#include	"amb.h"
//#include "bplx.h"
#include "basedll.h"

//=*************** Prepare DEV_entry *******************
//=*****************************************************
typedef S32 CMD_EntryPoint( BASE_CLASS *pDev, void *pParam );

S32 CMD_init          ( BASE_CLASS *pDev, void *pParam );
S32 CMD_cleanup       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_reinit        ( BASE_CLASS *pDev, void *pParam ); // for hot PnP
S32 CMD_checkExistance( BASE_CLASS *pDev, void *pParam ); // for hot PnP
S32 CMD_open          ( BASE_CLASS *pDev, void *pParam );
S32 CMD_close         ( BASE_CLASS *pDev, void *pParam );
S32 CMD_getinfo       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_load          ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_puLoad        ( BASE_CLASS *pDev, void *pParam );
S32 CMD_puState       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_puList        ( BASE_CLASS *pDev, void *pParam );
S32 CMD_puRead        ( BASE_CLASS *pDev, void *pParam );
S32 CMD_puWrite       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_reset         ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_start         ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_stop          ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_symbol        ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_version       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_capture       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_release       ( BASE_CLASS *pDev, void *pParam );
S32 CMD_ctrl          ( BASE_CLASS *pDev, void *pParam );
S32 CMD_extension     ( BASE_CLASS *pDev, void *pParam );
S32 CMD_peek          ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_poke          ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_readRAM       ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_writeRAM      ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_readFIFO      ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_writeFIFO     ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_readDPRAM     ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_writeDPRAM    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_read          ( BASE_CLASS *pDev, void *pParam );
S32 CMD_write         ( BASE_CLASS *pDev, void *pParam );
S32 CMD_getMsg        ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_putMsg        ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalSend    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalGrab    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalWait    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalIack    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalFresh   ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalInfo    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_signalList    ( BASE_CLASS *pDev, void *pParam ); // only for DSP
S32 CMD_devHardwareInit    ( BASE_CLASS *pDev, void *pParam );
S32 CMD_devHardwareCleanup ( BASE_CLASS *pDev, void *pParam );
S32 CMD_subHardwareInit    ( BASE_CLASS *pDev, void *pParam );
S32 CMD_subHardwareCleanup ( BASE_CLASS *pDev, void *pParam );
S32 CMD_getServList    ( BASE_CLASS *pDev, void *pParam );
S32 CMD_servCmdQuery  ( BASE_CLASS *pDev, void *pParam );

CMD_EntryPoint	*g_entryPointArr[DEVcmd_MAX] =
{
	CMD_init          ,
	CMD_cleanup       ,
	CMD_reinit        ,
	CMD_checkExistance,
	CMD_open          ,
	CMD_close         ,
	CMD_getinfo       ,
	NULL,				//	CMD_load          ,
	CMD_puLoad        ,
	CMD_puState       ,
    CMD_puList        ,
	NULL,				//	CMD_reset         ,
	NULL,				//	CMD_start         ,
	NULL,				//	CMD_stop          ,
	NULL,				//	CMD_symbol        ,
	CMD_version       ,
	CMD_capture       ,
	CMD_release       ,
	CMD_ctrl          ,
	CMD_extension     ,
	NULL,				//	CMD_peek          ,
	NULL,				//	CMD_poke          ,
	NULL,				//	CMD_readRAM       ,
	NULL,				//	CMD_writeRAM      ,
	NULL,				//	CMD_readFIFO      ,
	NULL,				//	CMD_writeFIFO     ,
	NULL,				//	CMD_readDPRAM     ,
	NULL,				//	CMD_writeDPRAM    ,
	CMD_read          ,
	CMD_write         ,
	NULL,				//	CMD_getMsg        , 
	NULL,				//	CMD_putMsg        ,
	NULL,				//	CMD_signalSend    ,
	NULL,				//	CMD_signalGrab    ,
	NULL,				//	CMD_signalWait    ,
	NULL,				//	CMD_signalIack    ,
	NULL,				//	CMD_signalFresh   ,
	NULL,				//	CMD_signalInfo    ,
	NULL,				//	CMD_signalList    ,
	CMD_devHardwareInit    ,
	CMD_devHardwareCleanup ,
	CMD_subHardwareInit    ,
	CMD_subHardwareCleanup ,
	CMD_getServList   ,
    CMD_puRead        ,
	CMD_puWrite       ,
	NULL				//	CMD_servCmdQuery
};

//=************************ DEV_entry ******************
//=*****************************************************
DEV_API S32		STDCALL DEV_entry( void *pDev, S32 cmd, void *pParam )
{
	if( cmd<0 )
		return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
	if( cmd>(S32)(sizeof(g_entryPointArr)/sizeof(g_entryPointArr[0])) )
		return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
	if( g_entryPointArr[cmd] == NULL )
		return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
	return g_entryPointArr[cmd]( (BASE_CLASS*)pDev, pParam );
}

//=********************** CMD_init *********************
//=*****************************************************
S32 CMD_init( BASE_CLASS *pDev, void *pParam )
{
	//
	// Dummy fuction
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_cleanup ******************
//=*****************************************************
S32 CMD_cleanup( BASE_CLASS *pDev, void *pParam )
{
	//
	// TODO: Remove Device Date and Free Memory
	//

//	pDev->CleanUp();
//	HeapFree( GetProcessHeap(), 0, pDev );
	delete pDev;

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_reinit ******************
//=*****************************************************
S32 CMD_reinit( BASE_CLASS *pDev, void *pParam )
{
	//
	// Dummy fuction
	//

	return DRVERR(BRDerr_OK);
}


//=********************** CMD_checkExistance ******************
//=*****************************************************
S32 CMD_checkExistance( BASE_CLASS *pDev, void *pParam )
{	
	PDEV_CMD_CheckExistance	ptr = (PDEV_CMD_CheckExistance)pParam;

	//
	// TODO: If Board is in Work, return 1.
	//
	*(ptr->state) = 1;

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_open ******************
//=*****************************************************
S32 CMD_open( BASE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Open ptr = (PDEV_CMD_Open)pParam;

	//
	// TODO: Check BRDopen_SHARE flag
	//
	if( ptr->flag & BRDopen_SHARED )
	{
		// If Device is already opened without BRDopen_SHARE flag
		//	return DRVERR(BRDerr_DEVICE_UNSHAREABLE);
	}
	else
	{
		// If Device is already opened
		//	return DRVERR(BRDerr_DEVICE_UNSHAREABLE);
	}
	pDev->Open();
	//
	// TODO: Make open operation
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_close ********************
//=*****************************************************
S32 CMD_close( BASE_CLASS *pDev, void *pParam )
{
	//
	// TODO: Make close operation
	//
	pDev->Close();
	return DRVERR(BRDerr_OK);
}

//=********************** CMD_getinfo ******************
//=*****************************************************
S32 CMD_getinfo( BASE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Getinfo ptr = (PDEV_CMD_Getinfo)pParam;
/*
	typedef struct {
		BRD_Info *pInfo;
	} Struct;
	Struct	*ptr = (Struct*)pParam;
*/
	//
	// Check and Clear BRD_Info structure
	//
	if( ptr->pInfo->size < sizeof(BRD_Info) )
		return DRVERR(BRDerr_BUFFER_TOO_SMALL);
	memset( &(ptr->pInfo->code), 0, ptr->pInfo->size - sizeof(ptr->pInfo->size) );

	//
	// TODO: Fill BRD_Info structure
	//
	pDev->GetDeviceInfo(ptr->pInfo);

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_puLoad *******************
//=*****************************************************
S32 CMD_puLoad( BASE_CLASS *pDev, void *pParam )
{
	DEV_CMD_PuLoad *ptr = (DEV_CMD_PuLoad*)pParam;

	ULONG status = pDev->PuFileLoad(
									ptr->puId, 
									ptr->fileName, 
									ptr->state
									);
	return DRVERR(status);
}

//=********************** CMD_puState ******************
//=*****************************************************
S32 CMD_puState(BASE_CLASS *pDev, void *pParam )
{
	DEV_CMD_PuState	*ptr = (DEV_CMD_PuState*)pParam;

	ULONG status = pDev->GetPuState(
									ptr->puId, 
									ptr->state
									);
	return DRVERR(status);
}

//=********************** CMD_puList *******************
//=*****************************************************
S32 CMD_puList(BASE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_PuList	ptr = (PDEV_CMD_PuList)pParam;

	*(ptr->pItemReal) = pDev->GetPuCnt();

	// Check List Size
	if(ptr->item < *(ptr->pItemReal))
		return DRVERR(BRDerr_BUFFER_TOO_SMALL);

	// Fill List
	pDev->GetPuList(ptr->pList);

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_puRead *******************
//=*****************************************************
S32 CMD_puRead(BASE_CLASS *pDev, void *pParam )
{
	ULONG status = BRDerr_OK;
	PDEV_CMD_PuRead ptr = (PDEV_CMD_PuRead)pParam;

	ULONG puNum = ptr->puId;
	switch(puNum)
	{
	case BRDpui_BASEICR:
		status = pDev->ReadNvRAM(ptr->pBuf, ptr->size, ptr->offset + 0x80);
		break;
	case BRDpui_SUBICR:
	case BRDpui_SUBICR+1:
		//status = pDev->ReadAdmIdROM(ptr->pBuf,ptr->size, ptr->offset);
		status = pDev->ReadSubICR(puNum - BRDpui_SUBICR, ptr->pBuf, ptr->size, ptr->offset);
		break;
	case BRDpui_FRUID:
		status = pDev->ReadFmcFRUID(ptr->pBuf, ptr->size, ptr->offset);
		break;
	case BRDpui_PLD:
	case BRDpui_ARM:
		status = pDev->PldReadToFile(puNum, (BRDCHAR*)ptr->pBuf);
		break;
	default:
		return DRVERR(BRDerr_BAD_PARAMETER);
	}
	if(BRDerr_OK != status)
		status = BRDerr_HW_ERROR;
	//char msg[128];
	//if(BRDerr_OK != status)
	//{
	//	sprintf(msg, "<CMD_puRead> ERROR by reading submodule ICR, status = 0X%08X", status);
	//	MessageBox(NULL, msg, "BAMBP", MB_OK);
	//	status = BRDerr_HW_ERROR;
	//}
	//else
	//{
	//	sprintf(msg, "<CMD_puRead> OK by reading submodule ICR, status = 0X%08X", status);
	//	MessageBox(NULL, msg, "BAMBP", MB_OK);
	//}
	return DRVERR(status);
	//return DRVERR(BRDerr_OK);
}

//=********************** CMD_puWrite *******************
//=*****************************************************
S32 CMD_puWrite(BASE_CLASS *pDev, void *pParam )
{
	ULONG status = BRDerr_OK;
	PDEV_CMD_PuWrite ptr = (PDEV_CMD_PuWrite)pParam;

	ULONG puNum = ptr->puId;
	switch(puNum)
	{
	case BRDpui_BASEICR:
		status = pDev->WriteNvRAM(ptr->pBuf,ptr->size, ptr->offset + 0x80);
		break;
	case BRDpui_SUBICR:
	case BRDpui_SUBICR+1:
		//status = pDev->WriteAdmIdROM(ptr->pBuf,ptr->size, ptr->offset);
		status = pDev->WriteSubICR(puNum - BRDpui_SUBICR, ptr->pBuf, ptr->size, ptr->offset);
		break;
	case BRDpui_FRUID:
		status = pDev->WriteFmcFRUID(ptr->pBuf,ptr->size, ptr->offset);
		break;
	default:
		return DRVERR(BRDerr_BAD_PARAMETER);
	}
	if(BRDerr_OK != status)
		status = BRDerr_HW_ERROR;
	return DRVERR(status);
	//return DRVERR(BRDerr_OK);
}

//=********************** CMD_capture ******************
//=*****************************************************
S32 CMD_capture( BASE_CLASS *pDev, void *pParam )
{
	//
	// It is a dummy function (may be)
	//

	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}

//=********************** CMD_release ******************
//=*****************************************************
S32 CMD_release( BASE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Release ptr = (PDEV_CMD_Release)pParam;

	S32 status = BRDerr_OK;
	status = pDev->SrvCtrl(ptr->handle, SERVcmd_SYS_RELEASE, NULL, NULL);
	//
	// It is a dummy function (may be)
	//
//	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
	return DRVERR(status);
}

//=********************** CMD_ctrl *********************
//=*****************************************************
S32 CMD_ctrl( BASE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Ctrl ptr = (PDEV_CMD_Ctrl)pParam;

	//
	// TODO: Check Resource Index and Capture Mode
	//

	//
	// TODO: Make Resource operation
	//
	S32 status = BRDerr_OK;
//	pDev->SetCurHandle(ptr->handle);
	status = pDev->SrvCtrl(ptr->handle, ptr->cmd, ptr->arg, NULL);
/*
	switch(ptr->cmd)
	{
		case SYScmd_SERV_AVAILABLE:
		{
			PSYS_CMD_ServAvailable pSrvAvailable = (PSYS_CMD_ServAvailable)ptr->arg;
			status = pDev->IsSrvAvailable(ptr->handle, pSrvAvailable);
			break;
		}
		default:
			status = pDev->SrvCtrl(ptr->handle, ptr->cmd, ptr->arg);
//			return BRDerr_CMD_UNSUPPORTED;
	}
*/
	return DRVERR(status);
}

//=********************** CMD_extension ****************
//=*****************************************************
S32 CMD_extension( BASE_CLASS *pDev, void *pParam )
{
	S32 status = BRDerr_OK;
    PDEV_CMD_Extension ptr = (PDEV_CMD_Extension)pParam;
//	typedef struct { U32 nodeId; U32 cmd; void *arg; } TStruct;
//	TStruct		*ptr = (TStruct*)pParam;
	switch(ptr->cmd)
	{
		case 0:
			break;
		case BRDextn_GET_PLDFILEINFO_ANDOR:
			status = pDev->GetPldFileInfo((PBRDextn_PLDFILEINFO)ptr->arg);
			break;
		case BRDextn_GET_PLDINFO:
			status = pDev->GetPldInfo((PBRDextn_PLDINFO)ptr->arg);
			break;
		case BRDextn_SET_FMCPOWER:
			status = pDev->PowerOnOff((PBRDextn_FMCPOWER)ptr->arg, 1);
			break;
		case BRDextn_GET_FMCPOWER:
			status = pDev->GetPower((PBRDextn_FMCPOWER)ptr->arg);
			break;
		case BRDextn_WRITE_FMCEXT:
			//status = pDev->WriteFmcExt((PBRDextn_FMCEXT)ptr->arg);
		{
			PBRDextn_FMCEXT ext = (PBRDextn_FMCEXT)ptr->arg;
			status = pDev->WriteI2C(0, ext->dev, ext->addr, ext->value, 1);
			break;
		}
		case BRDextn_READ_FMCEXT:
			//status = pDev->ReadFmcExt((PBRDextn_FMCEXT)ptr->arg);
		{
			PBRDextn_FMCEXT ext = (PBRDextn_FMCEXT)ptr->arg;
			status = pDev->ReadI2C(0, ext->dev, ext->addr, &ext->value, 1);
			break;
		}
		case BRDextn_SENSORS:
			//status = pDev->GetSensors((PBRDextn_Sensors)ptr->arg);
			status = pDev->GetSensors(ptr->arg);
			break;
		case BRDextn_SET_IRQMINTIME:
			status = pDev->SetIrqMinTime((ULONG*)ptr->arg);
			break;
		case BRDextn_GET_IRQMINTIME:
			status = pDev->GetIrqMinTime((ULONG*)ptr->arg);
			break;
		case BRDextn_SHOW_DIALOG_ANDOR:
			pDev->ShowDialog((PBRDextn_PropertyDlg)ptr->arg);
			break;
		case BRDextn_DEL_DIALOG_ANDOR:
			pDev->DeleteDialog((PBRDextn_PropertyDlg)ptr->arg);
			break;
		case BRDextn_PUVERIFY:
			pDev->PuVerify((PBRDextn_PuVerify)ptr->arg);
			break;
		case BRDextn_REGCALLBACK:
			pDev->RegisterCallBack(ptr->arg);
			break;
		default:
			return DRVERR(BRDerr_CMD_UNSUPPORTED);
	}

	return DRVERR(status);
}

//=********************** CMD_version ******************
//=*****************************************************
S32 CMD_version( BASE_CLASS *pDev, void *pParam )
{
	PDEV_CMD_Version ptr = (PDEV_CMD_Version)pParam;

	//
	// TODO: Get Product Version
	//

	ptr->pVersion->drvMajor = VER_MAJOR;
	ptr->pVersion->drvMinor = VER_MINOR;

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_read ******************
//=*****************************************************
S32 CMD_read( BASE_CLASS *pDev, void *pParam )
{
	typedef struct { U32 nodeId; void *hostAdr; U32 size; U32 timeout; } TStruct;
	//TStruct		*ptr = (TStruct*)pParam;

	//
	// TODO: Make operation
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_write ******************
//=*****************************************************
S32 CMD_write( BASE_CLASS *pDev, void *pParam )
{
	typedef struct { U32 nodeId; void *hostAdr; U32 size; U32 timeout; } TStruct;
	//TStruct		*ptr = (TStruct*)pParam;

	//
	// TODO: Make operation
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_devHardwareInit **********
//=*****************************************************
S32 CMD_devHardwareInit( BASE_CLASS *pDev, void *pParam )
{
/*	DEV_CMD_DevHardwareInit	*ptr = (DEV_CMD_DevHardwareInit*)pParam;

	LL_ProcReset( pDev );
*/
	ULONG status = pDev->HardwareInit();

	return DRVERR(status);
	//return DRVERR(BRDerr_OK);
}

//=********************** CMD_devHardwareCleanup *******
//=*****************************************************
S32 CMD_devHardwareCleanup( BASE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_DevHardwareCleanup	*ptr = (DEV_CMD_DevHardwareCleanup*)pParam;

	//
	// TODO: Make operation
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareInit **********
//=*****************************************************
S32 CMD_subHardwareInit( BASE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_SubHardwareInit	*ptr = (DEV_CMD_SubHardwareInit*)pParam;

	//
	// TODO: Make operation
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareCleanup *******
//=*****************************************************
S32 CMD_subHardwareCleanup( BASE_CLASS *pDev, void *pParam )
{
//	DEV_CMD_SubHardwareCleanup	*ptr = (DEV_CMD_SubHardwareCleanup*)pParam;

	//
	// TODO: Make operation
	//

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_getServList **************
//=*****************************************************
S32 CMD_getServList( BASE_CLASS *pDev, void *pParam )
{
	DEV_CMD_GetServList	*ptr = (DEV_CMD_GetServList*)pParam;

    if(ptr == NULL )
        return DRVERR(BRDerr_BAD_PARAMETER);

//	ptr->itemReal = 0; // None Services

	ptr->itemReal = pDev->GetSrvCnt();

	// Check List Size
	if(ptr->item < ptr->itemReal)
		return DRVERR(BRDerr_BUFFER_TOO_SMALL);

	// Fill List
	pDev->GetSrvList(ptr->pSLI);

	pDev->SetCandidateSrv();
	//    return Serv_GetServList( pDev, ptr );
	return DRVERR(BRDerr_OK);
}
/*
//=********************** CMD_load ******************
//=*****************************************************
S32 CMD_load( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_reset ******************
//=*****************************************************
S32 CMD_reset( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_start ******************
//=*****************************************************
S32 CMD_start( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_stop *********************
//=*****************************************************
S32 CMD_stop( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_symbol *******************
//=*****************************************************
S32 CMD_symbol( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_peek ******************
//=*****************************************************
S32 CMD_peek( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_poke ******************
//=*****************************************************
S32 CMD_poke( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_readRAM ******************
//=*****************************************************
S32 CMD_readRAM( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_writeRAM ******************
//=*****************************************************
S32 CMD_writeRAM( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_readFIFO ******************
//=*****************************************************
S32 CMD_readFIFO( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_writeFIFO ******************
//=*****************************************************
S32 CMD_writeFIFO( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_readDPRAM ******************
//=*****************************************************
S32 CMD_readDPRAM( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_writeDPRAM ******************
//=*****************************************************
S32 CMD_writeDPRAM( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_getMsg *******************
//=*****************************************************
S32 CMD_getMsg( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_putMsg *******************
//=*****************************************************
S32 CMD_putMsg( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalSend ******************
//=********************************************************
S32 CMD_signalSend( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalGrab ******************
//=*****************************************************
S32 CMD_signalGrab( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalWait ******************
//=*****************************************************
S32 CMD_signalWait( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalIack ******************
//=*****************************************************
S32 CMD_signalIack( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalFresh *****************
//=*****************************************************
S32 CMD_signalFresh( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalInfo ******************
//=*****************************************************
S32 CMD_signalInfo( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_signalList ***************
//=*****************************************************
S32 CMD_signalList( BASE_CLASS *pDev, void *pParam )
{
	return DRVERR(BRDerr_CMD_UNSUPPORTED);
}
*/
//***************** End of file Entry.cpp *****************
