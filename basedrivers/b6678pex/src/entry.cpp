/********************************************************************
 *  (C) Copyright 1992-2012 InSys Corp.  All rights reserved.
 *
 *  ======== entry.cpp ========
 *  Entry point BRD Driver for Boards:
 *		FMC110PDSP
 *		FMC111PDSP
 *		FMC112
 *		FMC114
 *		FMC117
 *		PEX-SRIO
 *		DSP6678PEX
 *      DSP6678V
 *  version 2.1
 *
 *  By DOR	(sep. 2012)
 *  Modify feb 2019 
 ********************************************************************/

// ----- inclide files -------------------
#include    <malloc.h>
#include    <stdio.h>
#include    <string.h>

#define	DEV_API_EXPORTS
#define	DEVS_API_EXPORTS

#include    "utypes.h"
#include    "brderr.h"
#include    "brdapi.h"
#include    "brdpu.h"
#include    "extn.h"
#include    "extn_dex.h"
#include    "extn_6678.h"
#include    "b6678pex.h"

#include    "icr.h"
#include    "Icr6678.h"

//
//==== Constants
//


//
//======= Macros
//

//
//==== Types
//
typedef struct { UINT32 size, linAdr, phyAdr; }		TIMemAlloc;
typedef struct { UINT32 adr, val, size; }			TIEepromWrite;

typedef S32 CMD_EntryPoint( BRDDRV_Board *pDev, void *pParam );

//
//==== Global Variables
//


//
//==== Functions Declaration
//
//static long GetBoardCFG(BRDDRV_Board *pDev, void *data, long size, U32 signature);

//=*************** Prepare DEV_entry *******************
//=*****************************************************
S32 CMD_init          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_cleanup       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_reinit        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_checkExistance( BRDDRV_Board *pDev, void *pParam );
S32 CMD_open          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_close         ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_getinfo       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_load          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_puLoad        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_puState       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_puList        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_reset         ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_start         ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_stop          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_symbol        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_version       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_capture       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_release       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_ctrl          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_extension     ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_peek          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_poke          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_readRAM       ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_writeRAM      ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_readFIFO      ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_writeFIFO     ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_readDPRAM     ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_writeDPRAM    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_read          ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_write         ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_getMsg        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_putMsg        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalSend    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalGrab    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalWait    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalIack    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalFresh   ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalInfo    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_signalList    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_devHardwareInit    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_devHardwareCleanup ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_subHardwareInit    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_subHardwareCleanup ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_getServList    ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_puRead        ( BRDDRV_Board *pDev, void *pParam );
S32 CMD_puWrite       ( BRDDRV_Board *pDev, void *pParam );

CMD_EntryPoint  *g_entryPointArr[DEVcmd_MAX] =
{
    CMD_init          ,
    CMD_cleanup       ,
    CMD_reinit        ,
    CMD_checkExistance,
    CMD_open          ,
    CMD_close         ,
    CMD_getinfo       ,
    CMD_load          ,
    CMD_puLoad        ,
    CMD_puState       ,
    CMD_puList        ,
    CMD_reset         ,
    CMD_start         ,
    CMD_stop          ,
    CMD_symbol        ,
    CMD_version       ,
    CMD_capture       ,
    CMD_release       ,
    CMD_ctrl          ,
    CMD_extension     ,
    CMD_peek          ,
    CMD_poke          ,
    CMD_readRAM       ,
    CMD_writeRAM      ,
    CMD_readFIFO      ,
    CMD_writeFIFO     ,
    CMD_readDPRAM     ,
    CMD_writeDPRAM    ,
    CMD_read          ,
    CMD_write         ,
    CMD_getMsg        , 
    CMD_putMsg        ,
    CMD_signalSend    ,
    CMD_signalGrab	  ,
    CMD_signalWait    ,
    CMD_signalIack    ,
    CMD_signalFresh   ,
    CMD_signalInfo    ,
    CMD_signalList    ,
    CMD_devHardwareInit    ,
    CMD_devHardwareCleanup ,
    CMD_subHardwareInit    ,
    CMD_subHardwareCleanup ,
    CMD_getServList    ,
    CMD_puRead        ,
    CMD_puWrite       
};


//=************************ DEV_entry ******************
//=*****************************************************
DEV_API S32     STDCALL DEV_entry( void *pDev, S32 cmd, void *pParam )
{
    if( cmd<0 || cmd>=DEVcmd_MAX )
        return DRVERR(BRDerr_BAD_PARAMETER);
    if( g_entryPointArr[cmd] == NULL )
        return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
    return g_entryPointArr[cmd]( (BRDDRV_Board*)pDev, pParam );
}

//=********************** CMD_init ******************
//=*****************************************************
S32 CMD_init( BRDDRV_Board *pDev, void *pParam )
{
	return DRVERR(BRDerr_OK);
}

//=********************** CMD_cleanup ******************
//=*****************************************************
S32 CMD_cleanup( BRDDRV_Board *pDev, void *pParam )
{
    Dev_FreeBoard( pDev );
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_reinit ******************
//=*****************************************************
S32 CMD_reinit( BRDDRV_Board *pDev, void *pParam )
{
		//
		//=== Dummy fuction
		//
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_checkExistance ******************
//=*****************************************************
S32 CMD_checkExistance( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_CheckExistance  *ptr = (DEV_CMD_CheckExistance*)pParam;

		//
		//=== TODO: If Board is in Work, return 1.
		//
    *(ptr->state) = 1;

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_open ******************
//=*****************************************************
S32 CMD_open( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Open    *ptr = (DEV_CMD_Open*)pParam;

		//
		//=== TODO: Check BRDopen_SHARE flag
		//
    if( ptr->flag & BRDopen_SHARED )
    {
		if( pDev->OPENRESET == 0)    
		    return DRVERR(BRDerr_OK);

		LL_ProcReset( pDev, (U32)-1);

			// If Device is already opened without BRDopen_SHARE flag
			//return DRVERR(BRDerr_DEVICE_UNSHAREABLE);
    }
    else
    {
			// If Device is already opened
			//return DRVERR(BRDerr_DEVICE_UNSHAREABLE);
    }
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_close ******************
//=*****************************************************
S32 CMD_close( BRDDRV_Board *pDev, void *pParam )
{
    //
    //=== TODO: Make close operation
    //
	return DRVERR(BRDerr_OK);
}

//=********************** CMD_getinfo ******************
//=*****************************************************
S32 CMD_getinfo( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Getinfo *ptr = (DEV_CMD_Getinfo*)pParam;

    //
    //=== TODO: Fill BRD_Info structure
    //
    if( ptr->pInfo->size < sizeof(BRD_Info) )
        return DRVERR(BRDerr_BUFFER_TOO_SMALL);
    ptr->pInfo->boardType   = (pDev->tidev->deviceId<<16)|(pDev->tidev->revision);
    ptr->pInfo->pid         = pDev->pid;        
   
	switch(pDev->tidev->deviceId) {
	case 0x6610: BRDC_strcpy(ptr->pInfo->name, _BRDC("FMC110PDSP") );  break;
	case 0x6611: BRDC_strcpy(ptr->pInfo->name, _BRDC("FMC111PDSP"));  break;
	case 0x6612: BRDC_strcpy(ptr->pInfo->name, _BRDC("FMC112PDSP") );  break;
	case 0x6614: BRDC_strcpy(ptr->pInfo->name, _BRDC("FMC114VDSP") );  break;
	case 0x6615: BRDC_strcpy(ptr->pInfo->name, _BRDC("PEX-SRIO") );    break;  
	case 0x6617: BRDC_strcpy(ptr->pInfo->name, _BRDC("FMC117PDSP") );  break;  
	case 0x6620: BRDC_strcpy(ptr->pInfo->name, _BRDC("DSP6678PEX") );  break;  
	case 0x6622: BRDC_strcpy(ptr->pInfo->name, _BRDC("DSP6678V") );    break;  
	default:     BRDC_strcpy(ptr->pInfo->name, _BRDC("Unknown") );  
	}

	ptr->pInfo->busType     = BRDbus_PCI;

    ptr->pInfo->bus         = pDev->tidev->pcibus;     
    ptr->pInfo->dev         = pDev->tidev->pcidev;     
    ptr->pInfo->slot        = pDev->tidev->pcislot;       
    ptr->pInfo->verMajor        = VER_MAJOR;    
    ptr->pInfo->verMinor        = VER_MINOR;    
    ptr->pInfo->base[0]         = (U32)pDev->tidev->mem_pciereg_va;
    ptr->pInfo->base[1]         = (U32)pDev->tidev->mem_bar1_va;
    ptr->pInfo->base[2]         = (U32)pDev->tidev->mem_bar2_va;
    ptr->pInfo->base[3]         = (U32)pDev->tidev->mem_bar3_va;
	ptr->pInfo->base[4]         = (U32)((U64)pDev->sysmem_vadr&0xFFFFFFFF);
	ptr->pInfo->base[5]         = (U32)((U64)pDev->sysmem_vadr>>32);
	ptr->pInfo->base[6]         = (U32)((U64)pDev->appmem_vadr&0xFFFFFFFF);
	ptr->pInfo->base[7]         = (U32)((U64)pDev->appmem_vadr>>32);
    ptr->pInfo->vectors[0]      = pDev->tidev->irq;
    
	return DRVERR(BRDerr_OK);
}


//=********************** GetPathToFile ****************
//=*****************************************************
static	int	GetPathToFile(const BRDCHAR	*fileName, BRDCHAR *path)
{
	int i;
	
	for(i=(int)BRDC_strlen(fileName);i>0;i--)
	{
		if(fileName[i]=='\\') break;
	}
	if( i==0) return 1;

	for(int j=0;j<i+1;j++)
		path[j] = fileName[j];

	return 0;
}


//=********************** CMD_load ******************
//=*****************************************************
//BRDCHAR	loadPath[256] = _BRDC(".\\");

S32 CMD_load( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Load    *ptr = (DEV_CMD_Load*)pParam;

    FILE        *fin;               // Load File handle
    S32         err;                // Error Code

	int			nodeId = ptr->nodeId;

	//printf("start CMD_load board (%d:%d)\n", pDev->tidev->pcibus,pDev->tidev->pcidev);

	//
    //=== Open File
    //
    fin = BRDC_fopen( ptr->fileName, _BRDC("rb") );
    if( fin == NULL )
        return Dev_ErrorPrintf( DRVERR(BRDerr_BAD_FILE), 
                                pDev, CURRFILE, _BRDC("<CMD_load> Can't open Load file '%s'"), ptr->fileName );

	// get PATH to file 
	BRDCHAR	loadPath[256] = _BRDC(".\\");
	BRDC_strcpy(&loadPath[0], _BRDC(".//") );
	GetPathToFile(ptr->fileName, &loadPath[0]); 

    //
    //=== Reset Board
    //

	//
    //=== Zero Program Params
    //
    pDev->argsAdr   = 0;
    pDev->argsSize  = 0;

    //
    //=== Load Code to board
    //
    err = LL_ProcLoad( pDev, nodeId, ptr->fileName,ptr->argc, (BRDCHAR **)ptr->argv );
	if( err<=0 ) {
		//printf("ERROR:end CMD_load board (%d:%d)\n", pDev->tidev->pcibus,pDev->tidev->pcidev);
        return Dev_ErrorPrintf( err, pDev, CURRFILE, _BRDC("<CMD_load> Can't parse file '%s'"), ptr->fileName );
	}

	//printf("end CMD_load board (%d:%d)\n", pDev->tidev->pcibus,pDev->tidev->pcidev);

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_puLoad *******************
//=*****************************************************
S32 CMD_puLoad( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_PuLoad  *ptr = (DEV_CMD_PuLoad*)pParam;
    FILE            *fin;
    S32             err = DRVERR(BRDerr_BAD_PARAMETER);

	//
	//=== Check pldId
	//
    if( (ptr->puId!=0x1)         // EEPROM (Configuration Table) 
		)
        return Dev_ErrorPrintf( DRVERR(BRDerr_BAD_NODEID), 
                                pDev, CURRFILE, _BRDC("<CMD_puLoad> Bad Programmable Unit ID=%d"), ptr->puId );

	//
	//=== Open File
	//
    fin = ( ptr->fileName[0] != '\0' ) ? BRDC_fopen( ptr->fileName, _BRDC("rb") ) : NULL;
    if( fin == NULL )
        return Dev_ErrorPrintf( DRVERR(BRDerr_BAD_FILE), 
                                pDev, CURRFILE, _BRDC("<CMD_puLoad> Can't open data file '%s'"), ptr->fileName );

    if( ptr->puId==0x1 )	//=== Load ID EEPROM
    {
        char        ch = fgetc(fin);

        if(ch==':')	//=== hex format 
            err = LL_LoadIcrHEX( pDev, ptr->puId, fin );
        else		//=== binary format
            err = LL_LoadIcrBIN( pDev, ptr->puId, fin );
        *(ptr->state) = (err<0) ? 0 : 1;
    }

    fclose( fin );

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_puState ******************
//=*****************************************************
S32 CMD_puState( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_PuState     *ptr = (DEV_CMD_PuState*)pParam;

	//=== Get PLD State
	*(ptr->state) = 1;
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_puList *******************
//=*****************************************************
S32 CMD_puList( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_PuList  *ptr = (DEV_CMD_PuList*)pParam;
    
	*(ptr->pItemReal) = 2;

    //=== Check List Size
    if( ptr->item < 2 )
    {
        return DRVERR(BRDerr_BUFFER_TOO_SMALL);
    }

	//
	//=== Fill List
	//
	ptr->pList[0].puId   = BRDpui_BASEICR;
    ptr->pList[0].puCode = 0x4953;
    ptr->pList[0].puAttr = BRDpua_Read | BRDpua_Write | BRDpua_Danger;
	if( pDev->EEPROM_ACCESS==0x11223344)
	    ptr->pList[0].puAttr = BRDpua_Read | BRDpua_Write;
    BRDC_strcpy( ptr->pList[0].puDescription, _BRDC("ICR C6678") );

	ptr->pList[1].puId   = BRDpui_CFG6678;
    ptr->pList[1].puCode = 0x6678;
    ptr->pList[1].puAttr = BRDpua_Read | BRDpua_Write;
    BRDC_strcpy( ptr->pList[1].puDescription, _BRDC("CFG C6678") );

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_puRead *******************
//=*****************************************************
S32 CMD_puRead( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_PuRead  *ptr = (DEV_CMD_PuRead*)pParam;

	//=== only EEPROM readable
    if( (ptr->puId!=0x1) && (ptr->puId!=0x20) )
        return Dev_ErrorPrintf( DRVERR(BRDerr_BAD_FILE), 
                                pDev, CURRFILE, _BRDC("<CMD_puRead> This not readable Unit ID=%d"), ptr->puId );

	if( ptr->puId==0x1 ) LL_icrRead(pDev,ptr->puId,ptr->offset,ptr->pBuf,ptr->size);
//	if( ptr->puId==0x20 ) LL_i2cFlashRead(pDev,ptr->puId,0xFE00,ptr->pBuf,ptr->size);
	if( ptr->puId==0x20 ) LL_i2cFlashRead(pDev,ptr->puId,ptr->offset,ptr->pBuf,ptr->size);

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_puWrite ******************
//=*****************************************************

S32 CMD_puWrite( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_PuWrite *ptr = (DEV_CMD_PuWrite*)pParam;

		//=== only EEPROM writable
    if( (ptr->puId!=0x1) && (ptr->puId!=0x20) )
        return Dev_ErrorPrintf( DRVERR(BRDerr_BAD_FILE), 
                                pDev, CURRFILE, _BRDC("<CMD_puWrite> Bad writable Unit ID=%d"), ptr->puId );

	if( ptr->puId==0x1 ) LL_icrWrite(pDev,ptr->puId,ptr->offset,ptr->pBuf,ptr->size);
	if( ptr->puId==0x20 ) LL_i2cFlashWrite(pDev,ptr->puId,ptr->offset,ptr->pBuf,ptr->size);

	return DRVERR(BRDerr_OK);
}

//=********************** CMD_reset ******************
//=*****************************************************
S32 CMD_reset( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Reset   *ptr = (DEV_CMD_Reset*)pParam;
	int	nodeId = ptr->nodeId;

	IPC_TIMEVAL time_start, time_stop;
	IPC_getTime(&time_start);
	S32 ret = LL_ProcReset( pDev,nodeId );
	IPC_getTime(&time_stop);
	double ms_time = IPC_getDiffTime(&time_start, &time_stop);
	
	return ret;
	//return LL_ProcReset( pDev,nodeId );
}

//=********************** CMD_start ********************
//=*****************************************************
S32 CMD_start( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Start   *ptr = (DEV_CMD_Start*)pParam;
	int	nodeId = ptr->nodeId;

	fflush(stdout);
	setbuf(stdout, NULL);

    return LL_ProcStart( pDev, nodeId );
}

//=********************** CMD_stop *********************
//=*****************************************************
S32 CMD_stop( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Stop    *ptr = (DEV_CMD_Stop*)pParam;
	int	nodeId = ptr->nodeId;
    LL_ProcReset( pDev,nodeId );
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_capture ******************
//=*****************************************************
S32 CMD_capture( BRDDRV_Board *pDev, void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_release ******************
//=*****************************************************
S32 CMD_release( BRDDRV_Board *pDev, void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_ctrl *********************
//=*****************************************************
S32 CMD_ctrl( BRDDRV_Board *pDev, void *pParam )
{
    //
    // TODO: Check Service Index and Capture Mode
    //

    //
    // TODO: Make Service operation
    //
    //return Serv_Ctrl( ptr->handle, pDev, ptr->cmd, ptr->arg );
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}

//=********************** CMD_extension ****************
//=*****************************************************
S32 CMD_extension( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Extension   *ptr = (DEV_CMD_Extension*)pParam;
	int nodeId = ptr->nodeId;

    switch(ptr->cmd)
    {
	    //=== Allocate nonpageble block of system memory
    case BRDextn_GET_MINPERBYTE:    
        {
            BRDextn_GetMinperbyte  *pGM = (BRDextn_GetMinperbyte*)ptr->arg;

            //pGM->val = 4;
			pGM->val = 1;
            break;
        }

	case BRDextn_DMA_START_ADDR:  
        {
            BRDextn_dmaStartAddr  *pDMA = (BRDextn_dmaStartAddr*)ptr->arg;

			// remap window
			U32 base = pDMA->padr_ti&0xFFF80000; // получаем базовый адрес дл€ окна BAR3
			*(U32*)(pDev->tidev->mem_pciereg_va+IB_OFFSET(3)) = base; 
			for(int i=0;i<20000;i++);
			//Pause(2000);
			
			uintptr_t offset = pDMA->padr_ti - base;
			pDMA->vadr = (U32*)(pDev->tidev->mem_bar3_va + offset);
			pDMA->physadr = pDev->tidev->mem_bar3 + offset;
			pDMA->maxsize = pDev->tidev->bar3_size - offset;
			//pDMA->maxsize = 0x80000;

			if(nodeId<4)	pDMA->phyInt = pDev->tidev->mem_bar2 + 0x20;
			else			pDMA->phyInt = pDev->tidev->mem_bar2 + 0x4000 + 0x20;
			
			pDMA->intFpgaCode = 50+(nodeId&3);

			pDMA->cnt_vadr    = (U32*)(pDev->tidev->mem_pciereg_va+GPR(0)); 
			pDMA->cnt_physadr = (U32)(pDev->tidev->mem_pciereg+GPR(0)); 

			break;
        }

	case BRDextn_DEX_DMA_START_ADDR:  
        {
            BRDextn_dexDmaStartAddr  *pDMA = (BRDextn_dexDmaStartAddr*)ptr->arg;

			// remap window
			U32 base = pDMA->padr_dsp&0xFFF80000; // получаем базовый адрес дл€ окна BAR3
			*(U32*)(pDev->tidev->mem_pciereg_va+IB_OFFSET(3)) = base; 
			for(int i=0;i<20000;i++);
			//Pause(2000);
			
			uintptr_t offset = pDMA->padr_dsp - base;
			pDMA->vadr = (U32*)(pDev->tidev->mem_bar3_va + offset);
			pDMA->physadr = pDev->tidev->mem_bar3 + offset;
			pDMA->maxsize = pDev->tidev->bar3_size - offset;
			//pDMA->maxsize = 0x80000;

			if(nodeId<4)	pDMA->phyInt = pDev->tidev->mem_bar2 + 0x20;
			else			pDMA->phyInt = pDev->tidev->mem_bar2 + 0x4000 + 0x20;
			
			pDMA->intFpgaCode = 50+(nodeId&3);
			break;
        }

	case BRDextn_DEX_PARAM:  
        {
            BRDextn_dexParam  *pDexParam = (BRDextn_dexParam*)ptr->arg;

			U08* pSysMemBuf = (U08*)(pDev->sysmem_vadr); // специальна€ раздел€ема€ область пам€ти дл€ обмена служебными данными между прикладным ѕќ и C6678
			pDexParam->pDaqParam = (U08*)(pSysMemBuf + 0x00070000); // сюда копируем параметры сбора данных
			pDexParam->pStubParam = (BRDstrm_Stub*)(pSysMemBuf + 0x00074000); // сюда копируем блочек дл€ отслеживани€ процесса сбора данных
			
			break;
        }

    case BRDextn_SOFT_RESET:  
        {
			TI6678_DEVICE *dev = pDev->tidev;
			BRDextn_softReset *pSr = (BRDextn_softReset *)ptr->arg;

			TI6678PCIE_ProcSoftReset(dev, 0);
			IPC_delay(pSr->timeout);
			RestorePciCfg(pDev->hWDM);
			{
				TI6678PCIE_ReInit(dev);
				U32 x = *(U32*)(dev->mem_pciereg_va);
				x&=0xFFFFFF00;
				if( 0x4E301100 == x ) {
					break;
				} else {
					return DRVERR(BRDerr_HW_ERROR);
				}
			}

			break;
        }


	default:
            return DRVERR(BRDerr_CMD_UNSUPPORTED);

    }

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_symbol *******************
//=*****************************************************
S32 CMD_symbol( BRDDRV_Board *pDev, void *pParam )
{
    return DRVERR(BRDerr_CMD_UNSUPPORTED);
}

//=********************** CMD_version ******************
//=*****************************************************
S32 CMD_version( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Version *ptr = (DEV_CMD_Version*)pParam;

    ptr->pVersion->drvMajor = VER_MAJOR;
    ptr->pVersion->drvMinor = VER_MINOR;

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_peek *********************
//=*****************************************************
S32 CMD_peek( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Peek    *ptr = (DEV_CMD_Peek*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcReadMem( pDev, nodeId, ptr->brdAdr, &ptr->val, szU32 );

}

//=********************** CMD_poke *********************
//=*****************************************************
S32 CMD_poke( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_Poke    *ptr = (DEV_CMD_Poke*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcWriteMem( pDev, nodeId, ptr->brdAdr, ptr->val, szU32 );
}

//=********************** CMD_readRAM ******************
//=*****************************************************
S32 CMD_readRAM( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_ReadRAM *ptr = (DEV_CMD_ReadRAM*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcReadMemDump( pDev, nodeId, ptr->brdAdr, (U32*)ptr->hostAdr, ptr->itemNum*ptr->itemSize);
}

//=********************** CMD_writeRAM ******************
//=*****************************************************
S32 CMD_writeRAM( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_WriteRAM    *ptr = (DEV_CMD_WriteRAM*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcWriteMemDump( pDev, nodeId, ptr->brdAdr, (U32*)ptr->hostAdr, ptr->itemNum*ptr->itemSize );
}

//=********************** CMD_readFIFO ******************
//=*****************************************************
S32 CMD_readFIFO( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_ReadFIFO    *ptr = (DEV_CMD_ReadFIFO*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_readFIFO(pDev,nodeId,ptr->brdAdr,(U32*)ptr->hostAdr,ptr->size,ptr->timeout);
}

//=********************** CMD_writeFIFO ******************
//=*****************************************************
S32 CMD_writeFIFO( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_WriteFIFO   *ptr = (DEV_CMD_WriteFIFO*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_writeFIFO(pDev,nodeId,ptr->brdAdr,(U32*)ptr->hostAdr,ptr->size,ptr->timeout);
}

//=********************** CMD_readDPRAM ******************
//=*****************************************************
S32 CMD_readDPRAM( BRDDRV_Board *pDev, void *pParam )
{
	DEV_CMD_ReadDPRAM   *ptr = (DEV_CMD_ReadDPRAM*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcReadDPRAM( pDev, nodeId, ptr->offset, (U32*)ptr->hostAdr, ptr->size);
}

//=********************** CMD_writeDPRAM ******************
//=*****************************************************
S32 CMD_writeDPRAM( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_WriteDPRAM  *ptr = (DEV_CMD_WriteDPRAM*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcWriteDPRAM( pDev, nodeId, ptr->offset, (U32*)ptr->hostAdr, ptr->size);
}

//=********************** CMD_read ******************
//=*****************************************************
S32 CMD_read( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_ReadWrite  *ptr = (DEV_CMD_ReadWrite*)pParam;
	int	nodeId = ptr->nodeId;
    return LL_read(pDev,nodeId,pParam);
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_write ******************
//=*****************************************************
S32 CMD_write( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_ReadWrite  *ptr = (DEV_CMD_ReadWrite*)pParam;
	int	nodeId = ptr->nodeId;
    return LL_write(pDev,nodeId,pParam);
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_GetMsg *******************
//=*****************************************************
S32 CMD_getMsg( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_GetMsg  *ptr = (DEV_CMD_GetMsg*)pParam;
	int	nodeId = ptr->nodeId;
    return LL_getMsg(pDev,nodeId,pParam);
}


//=********************** CMD_putMsg *******************
//=*****************************************************
S32 CMD_putMsg( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_PutMsg  *ptr = (DEV_CMD_PutMsg*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_putMsg(pDev,nodeId,pParam);
}


//=********************** CMD_signalSend ******************
//=********************************************************
S32 CMD_signalSend( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_SignalSend  *ptr = (DEV_CMD_SignalSend*)pParam;
	int	sigId = ptr->sigId;
	int	nodeId = ptr->nodeId;
	LL_ProcDSPINT( pDev , nodeId, sigId);
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_signalGrab ******************
//=*****************************************************
S32 CMD_signalGrab( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_SignalGrab  *ptr = (DEV_CMD_SignalGrab*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcGrabIrq( pDev, nodeId, ptr->sigId, &ptr->sigCounter, ptr->timeout );
}

//=********************** CMD_signalWait ******************
//=*****************************************************
S32 CMD_signalWait( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_SignalWait  *ptr = (DEV_CMD_SignalWait*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcWaitIrq( pDev, nodeId, ptr->sigId, &ptr->sigCounter, ptr->timeout );
}

//=********************** CMD_signalIack ******************
//=*****************************************************
S32 CMD_signalIack( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_SignalIack  *ptr = (DEV_CMD_SignalIack*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcSignalIack( pDev, nodeId);
}

//=********************** CMD_signalFresh *****************
//=*****************************************************
S32 CMD_signalFresh( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_SignalFresh *ptr = (DEV_CMD_SignalFresh*)pParam;
	int	nodeId = ptr->nodeId;
	return LL_ProcFreshIrq( pDev, nodeId, ptr->sigId, &ptr->sigCounter );
}

//=********************** CMD_signalInfo ******************
//=*****************************************************
S32 CMD_signalInfo( BRDDRV_Board *pDev, void *pParam )
{
    //DEV_CMD_SignalInfo  *ptr = (DEV_CMD_SignalInfo*)pParam;
    return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}

//=********************** CMD_signalList ***************
//=*****************************************************
S32 CMD_signalList( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_SignalList  *ptr = (DEV_CMD_SignalList*)pParam;
    
    *(ptr->pItemReal) = 8;

    //
    //=== Check List Size
    //
    if( ptr->item < 8 ) return DRVERR(BRDerr_BUFFER_TOO_SMALL);

    //
    //=== Fill List DSP to HOST
    //
    ptr->pList[0].sigId   = 0x0;
    ptr->pList[0].sigCode = 0x1;
    ptr->pList[0].sigAttr = BRDsig_ATTR2HOST;
    BRDC_strcpy( ptr->pList[0].sigDescription, _BRDC("Major User IRQ from Device to HOST") );

	int sigid = 4;
	for(int i=1;i<4;i++) {
		ptr->pList[i].sigId   = sigid++;
		ptr->pList[i].sigCode = 0x1;
		ptr->pList[i].sigAttr = BRDsig_ATTR2HOST;
		BRDC_strcpy( ptr->pList[i].sigDescription, _BRDC("Addon IRQ from Device to HOST") );
	}

    //
    //=== Fill List HOST to DSP
    //
	ptr->pList[4].sigId   = 0x0;
    ptr->pList[4].sigCode = 0x2;
    ptr->pList[4].sigAttr = BRDsig_ATTR2DEVICE;
    BRDC_strcpy( ptr->pList[4].sigDescription, _BRDC("Major User IRQ from HOST to Device") );

	sigid = 4;
	for(int i=5;i<8;i++) {
		ptr->pList[i].sigId   = sigid++;
		ptr->pList[i].sigCode = 0x2;
		ptr->pList[i].sigAttr = BRDsig_ATTR2DEVICE;
		BRDC_strcpy( ptr->pList[i].sigDescription, _BRDC("Addon IRQ from HOST to Device") );
	}

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_devHardwareInit **********
//=*****************************************************
S32 CMD_devHardwareInit( BRDDRV_Board *pDev, void *pParam )
{
	DEV_CMD_Reset           sReset = {(U32)NODEALL};
	if( pDev->OPENRESET == 0) return DRVERR(BRDerr_OK);
	return CMD_reset( pDev, &sReset );
}

//=********************** CMD_devHardwareCleanup *******
//=*****************************************************
S32 CMD_devHardwareCleanup( BRDDRV_Board *pDev, void *pParam )
{
    //DEV_CMD_DevHardwareCleanup  *ptr = (DEV_CMD_DevHardwareCleanup*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareInit **********
//=*****************************************************
S32 CMD_subHardwareInit( BRDDRV_Board *pDev, void *pParam )
{
    //DEV_CMD_SubHardwareInit *ptr = (DEV_CMD_SubHardwareInit*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareCleanup *******
//=*****************************************************
S32 CMD_subHardwareCleanup( BRDDRV_Board *pDev, void *pParam )
{
    //DEV_CMD_SubHardwareCleanup  *ptr = (DEV_CMD_SubHardwareCleanup*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_getServList ***************
//=*****************************************************
S32 CMD_getServList( BRDDRV_Board *pDev, void *pParam )
{
    DEV_CMD_GetServList *ptr = (DEV_CMD_GetServList*)pParam;

    if( ptr==NULL )
        return DRVERR(BRDerr_BAD_PARAMETER);
//    return Serv_GetServList( pDev, ptr );
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);

}


//
// End of file
//

