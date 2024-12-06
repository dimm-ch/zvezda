/********************************************************************
 *  (C) Copyright 1992-2012 InSys Corp.  All rights reserved.
 *
 *  ======== b6678pex.cpp ========
 *  BRD Driver  for Boards:
 *		FMC110PDSP
 *		FMC111PDSP
 *		FMC112PDSP
 *		FMC114VDSP
 *		FMC117PDSP
 *      PEX-SRIO
 *      DSP6678PEX
 *      DSP6678V
 *
 *  version 2.1
 *
 *  By DOR	(march 2013)
 *  modify 
 *  modify feb 2019
 ********************************************************************/
 
// ----- include files -------------------------
#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>

#define	DEV_API_EXPORTS
#define	DEVS_API_EXPORTS

#include	"utypes.h"
#include	"brdapi.h"
#include	"b6678pex.h"
#include	"extn_6678.h"

#include    "icr.h"
#include    "Icr6678.h"

//
//======= Macros
//

#define	SIGNATURESIZE	5
#define	SIGREGISTSIZE	32

#define	WDM_DRIVERNAME		TI6678DeviceName

#define	MEMBLOCK_PAGE	4
#define	MEMBLOCK_SIZE	MEMBLOCK_PAGE*4096

//
//==== Constants
//

//
//==== Types
//


//
//==== Global Variables
//

//
//==== Functions Declaration
//

//=********************** DllMain ***********************
//=******************************************************
#ifdef _WIN32
BOOL __stdcall DllMain( HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved )
{
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_THREAD_DETACH:
			break;
	}
	return TRUE;
}


//=********************** DllEntryPoint *****************
//=******************************************************
BOOL __stdcall DllEntryPoint(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
        // Included for compatibility with Borland 
	return DllMain (hinstDLL, fdwReason, lpvReserved);
}
#endif


//=********************** Dev_ErrorPrintf ****************
//=*******************************************************
S32	 Dev_ErrorPrintf( S32 errorCode, void *ptr, const BRDCHAR *title, const BRDCHAR *format, ... )
{
	va_list		arglist;

	if( ptr && errorCode<0 )
	{
		BRDDRV_Board		*pDev = (BRDDRV_Board*)ptr;
	
		pDev->errcnt++;
		pDev->errcode = errorCode;
	}

	va_start( arglist, format );		 
	BRDI_printf( 1<<EXTERRLVL(errorCode), title, format, arglist );
	va_end( arglist );

	return errorCode;
}

//=********************** Dev_Printf *********************
//=*******************************************************
void	Dev_Printf( S32 errLevel, const BRDCHAR *title, const BRDCHAR *format, ... )
{
	va_list		arglist;

	va_start( arglist, format );		 
	BRDI_printf( errLevel, title, format, arglist );
	va_end( arglist );
}


//=********************** Dev_AllocBoard *****************
//=*******************************************************
S32	 Dev_AllocBoard( void **ppDev, HANDLE hWDM, BRDCHAR *pBoardName )
{
	BRDDRV_Board		*pDev = NULL;

	//
	// Allocate Device Extension
	// 
	pDev = (BRDDRV_Board*)HAlloc(sizeof(BRDDRV_Board));
	if( pDev==NULL )
		return Dev_ErrorPrintf( DRVERR(BRDerr_INSUFFICIENT_RESOURCES), 
								NULL, CURRFILE, _BRDC("<Dev_AllocBoard> No enough memory") );

	*ppDev = pDev;

	return DRVERR(BRDerr_OK);
}

//=********************** Dev_FreeBoard ******************
//=*******************************************************
S32	 Dev_FreeBoard( BRDDRV_Board *pDev )
{
	if( pDev != NULL )
	{

		if(pDev->tidev->loadBufByte) {
			IPC_virtFree((void*)pDev->tidev->loadBufByte);
			pDev->tidev->loadBufByte = NULL;
		}
		if(pDev->tidev->checkBuf) {
			IPC_virtFree((void*)pDev->tidev->checkBuf);
			pDev->tidev->checkBuf = NULL;
		}
		if(pDev->tidev->loadBuf) {
			IPC_virtFree((void*)pDev->tidev->loadBuf);
			pDev->tidev->loadBuf = NULL;
		}

		if(pDev->hMutex)
		{
			IPC_deleteMutex(pDev->hMutex);
			pDev->hMutex = NULL;
		}

		//
			// TODO: Clear Resources
			//
		//Serv_ClearServList( pDev );

		if(pDev->hWDM)
			IPC_closeDevice(pDev->hWDM);

		HFree( pDev );
	}

	return DRVERR(BRDerr_OK);
}

//=********************** Dev_ParseBoard *****************
//=*******************************************************
S32	 Dev_ParseBoard( TIniData *pIniData, void **ppDev, BRDCHAR *pBoardName, S32 idxThisBrd )
{
	S32					err;				// Error Code
	IPC_handle			hWDM;
	int					idx;
	ULONG				PID = -1;
	USHORT				DeviceID;
	UCHAR				RevisionID;
	TI6678_LOCATION		brdLocation;
	TI6678_CONFIGURATION brdConfiguration;


	//
	//=== Prepare idxThisBrd
	//
	if( (pIniData->pcibus!=0xFFFFFFFF && pIniData->pcidev!=0xFFFFFFFF ) ||
		//((pIniData->pid&0x0FFFFFFF)!=0x0 && (pIniData->pid&0x0FFFFFFF)!=0x0FFFFFFF )
		//((pIniData->pid)!=0x0 && (pIniData->pid)!=0x0FFFFFFF && (pIniData->pid)!=0xF0000000 && (pIniData->pid)!=0x10000000 ) 
		((pIniData->pid)!=0x0FFFFFFF && (pIniData->pid)!=0xF0000000 ) 

	  )
		idxThisBrd = -1;
		
//	idxThisBrd = -1;



	//
	//=== Find PCI Card 
	//
	for(idx=0;;idx++)
	{
		IPC_str deviceName[256] = _BRDC("");
		int DeviceNumber = (0<=idxThisBrd) ? idxThisBrd : idx;
		hWDM = IPC_openDevice(deviceName, WDM_DRIVERNAME, DeviceNumber);
		if(!hWDM)
			return DRVERR(BRDerr_NO_KERNEL_SUPPORT);

		if((idx>idxThisBrd) && (idxThisBrd!=-1) )
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);

	    UCHAR	icr_buf[256];

		//=== Get ICR
		if(GetICR(hWDM, icr_buf) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}

		ICR_Id4953	*pIcrId = (ICR_Id4953	*)icr_buf;
		PID = pIcrId->dSerialNum;

		{
			ICR_Cfg6678	*pCfg6678 = (ICR_Cfg6678 *)(icr_buf+sizeof(ICR_Id4953));
			U16 tag = pCfg6678->wTag;
			U08 cpuCnt = pCfg6678->bCpuCnt;
			U08 cpuType = pCfg6678->bCpuType;
			U08 order = pCfg6678->bOrder;
			if(pCfg6678->bOrder>3) order = 0xFF;
			else if(pCfg6678->bOrder<0) order = 0xFF;
			//else if(pCfg6678->bOrder==0xFF) order = 0;
			U32 oo = order;
			PID = PID+(oo<<28);
		}

		if(PID==0xFFFFFFFF) PID=0;

		//=== Get DeviceID
		if(GetDeviceID(hWDM, DeviceID) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}

		//=== get revision
		if(GetRevisionID(hWDM, RevisionID) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}
		//=== Get Slot, Bus and Dev
		if(GetLocation(hWDM, &brdLocation) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}

		//=== Get Resource Config
		if(GetConfiguration(hWDM, &brdConfiguration) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}

			//
			//=== Read BoardID and compare it with PID or PCIBUS & PCIDEV
			//
		if( ((pIniData->pid&0x0FFFFFFF) != (U32)0x0FFFFFFF ) &&
			((pIniData->pid&0xF0000000) != (U32)0xF0000000 ) )
		{
			if( (0   != (pIniData->pid&0x0FFFFFFF)) &&
				(PID != (pIniData->pid&0x0FFFFFFF)) && 
				((PID&0xF0000000)!= (pIniData->pid&0xF0000000)) )

			{
				IPC_closeDevice( hWDM );	// Wrong Board
				continue;
			} else if ( (PID&0xF0000000)!= (pIniData->pid&0xF0000000)) {
				IPC_closeDevice( hWDM );	// Wrong Board
				continue;
				//return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
			}
		}
		else if(pIniData->pcibus != 0xFFFFFFFF)
		{
			if(  pIniData->pcibus!=brdLocation.BusNumber || pIniData->pcidev!=brdLocation.DeviceNumber )
			{
				IPC_closeDevice( hWDM );	// Wrong Board
				continue;
			}
		}

		if(idxThisBrd == -1)
			idxThisBrd = idx;
		break;		// Board are found
	}

	//
	//=== Allocate Device Extension
	//
	err = Dev_AllocBoard( ppDev, hWDM, pBoardName );
	if( err<0 )
	{
		IPC_closeDevice( hWDM );	// Wrong Board
		return err;
	}

	BRDC_sprintf( pBoardName, _BRDC("B6678PEX_%X"), PID);

	SBRDDRV_Board	*pDev = *(SBRDDRV_Board**)ppDev;

	pDev->tidev = TI6678PCIE_Open(idxThisBrd);

	pDev->errcode = 0;
	pDev->errcnt = 0;
	pDev->hWDM   = hWDM;
	pDev->pid = PID;
	pDev->tidev->pcibus = brdLocation.BusNumber;
	pDev->tidev->pcidev = brdLocation.DeviceNumber;
	pDev->tidev->pcislot = brdLocation.SlotNumber;
	pDev->tidev->deviceId = DeviceID;
	pDev->tidev->revision = RevisionID;
	pDev->tidev->mem_pciereg = brdConfiguration.PhysAddress[0];
	pDev->tidev->mem_bar1 = brdConfiguration.PhysAddress[1];
	pDev->tidev->mem_bar2 = brdConfiguration.PhysAddress[2];
	pDev->tidev->mem_bar3 = brdConfiguration.PhysAddress[3];
	pDev->tidev->mem_pciereg_va = (uintptr_t)brdConfiguration.VirtAddress[0];
	pDev->tidev->mem_bar1_va = (uintptr_t)brdConfiguration.VirtAddress[1];
	pDev->tidev->mem_bar2_va = (uintptr_t)brdConfiguration.VirtAddress[2];
	pDev->tidev->mem_bar3_va = (uintptr_t)brdConfiguration.VirtAddress[3];
	pDev->tidev->bar1_size = brdConfiguration.Size[1];
	pDev->tidev->bar2_size = brdConfiguration.Size[2];
	pDev->tidev->bar3_size = brdConfiguration.Size[3];
	pDev->tidev->irq = brdConfiguration.InterruptLevel;

	pDev->OPENRESET = pIniData->openreset;
	pDev->HWRESET = pIniData->hwreset;
	pDev->HWTIMEOUT = pIniData->hwtimeout;
	pDev->hostAddr = pIniData->hostAddr;
	pDev->hostSize = pIniData->hostSize;
	pDev->hioAddr = pIniData->hioAddr;
	pDev->hioSize = pIniData->hioSize;

	pDev->ncores = TI6678_CORES;

	pDev->sysmem_ob_idx = 30;	// system memory OB_BAR number
	pDev->sysmem_ob_size = 2;	// OB_BAR size (0-1M,1-2M,2-4M,3-8M)
	pDev->appmem_ob_idx = 28;	// app memory OB_BAR number
	pDev->appmem_ob_size = pDev->sysmem_ob_size;	// OB_BAR size (0-1M,1-2M,2-4M,3-8M)
	pDev->sysmem_padr = brdConfiguration.PhysAddress[4];
	pDev->sysmem_vadr = (U32*)brdConfiguration.VirtAddress[4];
	pDev->sysmem_size = 0x400000; // default 4MBytes

	pDev->appmem_padr = brdConfiguration.PhysAddress[5];
	pDev->appmem_vadr = (U32*)brdConfiguration.VirtAddress[5];
	pDev->appmem_size = 0x400000; // default 4MBytes

	// устанавливаем флаг типа загрузки
	if ( BRDC_strncmp(pIniData->boot, _BRDC("HOST"),4)==0)
	     pDev->BOOT = BOOT_HOST;
	else if ( BRDC_strncmp(pIniData->boot, _BRDC("EPROM"),5)==0)
	     pDev->BOOT = BOOT_EPROM;
	else pDev->BOOT = BOOT_HOST;

	BRDCHAR nameMutex[MAX_PATH];
	BRDC_sprintf(nameMutex, _BRDC("mutex_B6678PEX%d"), idxThisBrd);
	pDev->hMutex = IPC_createMutex(nameMutex, FALSE);

	for(int i=0;i<(int)BRDC_strlen(pIniData->start);i++) {
		if( islower(pIniData->start[i]) !=0 )
			pIniData->start[i] = BRDC_toupper(pIniData->start[i]);
	}
	if( BRDC_strncmp(pIniData->start, _BRDC("ALL"),3)==0)	pDev->start = 8;
	else										pDev->start = 0;

	pDev->tidev->loadBufByte = (UCHAR*)IPC_virtAlloc(0x200000);
	if(pDev->tidev->loadBufByte==NULL) 
		return Dev_ErrorPrintf( DRVERR(BRDerr_INSUFFICIENT_RESOURCES), 
								NULL, CURRFILE, _BRDC("<Dev_AllocBoard> No enough loadBufByte memory") );

	pDev->tidev->loadBuf = (ULONG*)IPC_virtAlloc(0x200000);
	if(pDev->tidev->loadBuf==NULL) 
		return Dev_ErrorPrintf( DRVERR(BRDerr_INSUFFICIENT_RESOURCES), 
								NULL, CURRFILE, _BRDC("<Dev_AllocBoard> No enough loadBufByte memory") );

	pDev->tidev->checkBuf = (ULONG*)IPC_virtAlloc(0x200000);
	if(pDev->tidev->checkBuf==NULL) 
		return Dev_ErrorPrintf( DRVERR(BRDerr_INSUFFICIENT_RESOURCES), 
								NULL, CURRFILE, _BRDC("<Dev_AllocBoard> No enough loadCheckBuf memory") );

	return btB6678;
}




//=******************************************************
//
// DRIVER EXPORTED FUNCTIONS
//
//


//=************************ DEV_initData ****************
// Return:  if OK returns boardType, else >0.
//*******************************************************
DEV_API	S32		STDCALL DEV_initData( S32 idxThisBrd, void **ppDev, BRDCHAR *pBoardName, TBRD_InitData *pInitData, S32 size )
{
	TIniData			iniData;		// Data from Registry or INI File
	S32					ret;			// Return value
	S32					isIntoBlock = 0;// Where Driver is Described: 0 - LID-Section, 1 - #begin/#end Block

	U32					keyValDec[8] = {
							0xFFFFFFFF,
							0xFFFFFFFF,
							0xFFFFFFFF,
							0xFFFFFFFF,
							1,
							0,
							5000,
							0xFF
						};
	U32					keyValHex[4] = {
							0x00000000,
							0x2000,
							0x00000000,
							0x2000
						};
	BRDCHAR				keyValString[3][256] = {
							_BRDC(""),
							_BRDC(""),
							_BRDC("")
						};
	BRDCHAR				keyWordDec[8][16] = {	// decimal value keyword
							_BRDC("pid"), 
							_BRDC("pcibus"), 
							_BRDC("pcidev"), 
							_BRDC("pcislot"),
							_BRDC("openreset"),
							_BRDC("hwreset"),
							_BRDC("hwtimeout"),
							_BRDC("order")
						};
	BRDCHAR				keyWordHex[4][16] = {	// hex value keyword
							_BRDC("hostaddr"),
							_BRDC("hostsize")
							_BRDC("hioaddr"),
							_BRDC("hiosize")
						};
	BRDCHAR				keyWordString[3][256] = {	// string value keyword
							_BRDC("boot"), 
							//_BRDC("eeprom_access"),
							_BRDC("start")
						};
	int			ii;
	unsigned int jj;
	BRDCHAR	*endptr;


		//
		//=== If Driver is Described in the #begin/#end Block
		//
	ii = 0;
	if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#begin") ) )
	{
		ii++;
		isIntoBlock = 1;
	}

		//
		//=== Search Key words
		//
	for( ; ii<size; ii++ )
	{
			//
			//=== Go through #begin/#end block 
			//		
		if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#begin") ) )	// Found keyword "#begin"
		{
			int beginCnt = 1;
			for(;;)
			{
				if( ++ii >= size )
					break;
				if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#begin") ) )		// Begin of nested SubSection
					beginCnt++;
				if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#end") ) )			// End of this or nested SubSection
					if( --beginCnt == 0 )
						break;
			}
			continue;
		}

			//
			// === Keyword "#end" - end of Driver SubSection
			// (Driver was Described in the #begin/#end Block, not in the [LID:##] Section)
			//
		if( isIntoBlock )
		if( !BRDC_stricmp( pInitData[ii].key, _BRDC("#end") ) )	 
		{
			break;
		}

			//
			//=== Compare Decimal Key Words
			//
		for( jj=0; jj<sizeof(keyValDec)/sizeof(keyValDec[0]); jj++ )
		{
			if( !BRDC_stricmp( pInitData[ii].key, keyWordDec[jj] ) ) 
			{
				keyValDec[jj] = (pInitData[ii].val[0]) ? BRDC_strtoul( pInitData[ii].val, &endptr, 0 ) : 0xFFFFFFFF;
				break;
			}
		}
			//
			//=== Compare Hex Key Words
			//
		for( jj=0; jj<sizeof(keyValHex)/sizeof(keyValHex[0]); jj++ )
		{
			if( !BRDC_stricmp( pInitData[ii].key, keyWordHex[jj] ) ) 
			{
				keyValHex[jj] = (pInitData[ii].val[0]) ? BRDC_strtoul( pInitData[ii].val, &endptr, 0 ) : 0xFFFFFFFF;
				break;
			}
		}
			//
			//=== Compare String Key Words
			//
		for( jj=0; jj<sizeof(keyValString)/sizeof(keyValString[0]); jj++ )
		{
			if( !BRDC_stricmp( pInitData[ii].key, keyWordString[jj] ) ) 
			{
				BRDC_strcpy(keyValString[jj],pInitData[ii].val);
				break;
			}
		}
	}


		//
		//=== Attach Board and Create DEV Driver Extension
		//
	iniData.pid = keyValDec[0];
	iniData.pcibus = keyValDec[1];
	iniData.pcidev = keyValDec[2];
	iniData.pcislot = keyValDec[3];
	iniData.openreset = keyValDec[4];
	iniData.hwreset = keyValDec[5];
	iniData.hwtimeout = keyValDec[6];
	iniData.order = keyValDec[7];

	iniData.pid += iniData.order<<28;

	iniData.hostAddr = keyValHex[0];
	iniData.hostSize = keyValHex[1];
	iniData.hioAddr = keyValHex[2];
	iniData.hioSize = keyValHex[3];

	if (BRDC_strcmp(keyValString[0], _BRDC("") ) != 0) 
			BRDC_strcpy(iniData.boot,keyValString[0]);
	else	BRDC_strcpy(iniData.boot,_BRDC("boot"));

	if (BRDC_strcmp(keyValString[2], _BRDC("") ) != 0) 
			BRDC_strcpy(iniData.start,keyValString[2]);
	else	BRDC_strcpy(iniData.start, _BRDC("start") );

	if( iniData.pid==0xFFFFFFFF && 
		iniData.pcislot==0xFFFFFFFF &&
		(iniData.pcibus==0xFFFFFFFF||iniData.pcidev==0xFFFFFFFF) )
			iniData.pid = 0x0;

	ret = Dev_ParseBoard( &iniData, ppDev, pBoardName, idxThisBrd );
	if( ret <=0 )
		return ret;


	return ret;
}


//=******************** DEV_initAuto **************
//=*****************************************************
DEV_API	S32		STDCALL DEV_initAuto( S32 idxThisBrd, void **ppDev, BRDCHAR *pBoardName, BRDCHAR *pLin, S32 linSize, 
									 TBRD_SubEnum *pSubEnum, U32 sizeSubEnum )
{
	IPC_handle			hWDM;
	S32					err;
	USHORT				DeviceID;
	UCHAR				RevisionID;
	TI6678_LOCATION		brdLocation;
	TI6678_CONFIGURATION brdConfiguration;


		//
		//=== Find PCI Card
		//

	IPC_str deviceName[256] = _BRDC("");
	hWDM = IPC_openDevice(deviceName, WDM_DRIVERNAME, idxThisBrd);
	if(!hWDM)
	{
			//ErrorPrintf(DRVERR(BRDerr_WARN),
			//				  CURRFILE,
			//				  _BRDC("<Init> No device driver for %s"), deviceName);
			return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
	}

		//=== Get DeviceID
		if(GetDeviceID(hWDM, DeviceID) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}

		//=== get revision
		if(GetRevisionID(hWDM, RevisionID) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}
		//=== Get Slot, Bus and Dev
		if(GetLocation(hWDM, &brdLocation) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}

		//=== Get Resource Config
		if(GetConfiguration(hWDM, &brdConfiguration) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}
	
		//
		//=== Allocate Device Extension
		//
	err = Dev_AllocBoard( ppDev, hWDM, pBoardName );
	if( err<0 )
	{
		//CloseHandle( hWDM );
		IPC_closeDevice(hWDM);
		return err;
	}

		//=== Get ICR
	    UCHAR	icr_buf[256];
		if(GetICR(hWDM, icr_buf) != BRDerr_OK)
		{
			IPC_closeDevice(hWDM);
			return DRVERR(BRDerr_BAD_DEVICE_VITAL_DATA);
		}
		ICR_Id4953	*pIcrId = (ICR_Id4953	*)icr_buf;

	BRDC_sprintf( pBoardName, _BRDC("B6678PEX_%X"), pIcrId->dSerialNum);

	SBRDDRV_Board	*pDev = *(SBRDDRV_Board**)ppDev;

	pDev->tidev = TI6678PCIE_Open(idxThisBrd);

	pDev->errcode = 0;
	pDev->errcnt = 0;
	pDev->hWDM   = hWDM;
	pDev->tidev->pcibus = brdLocation.BusNumber;
	pDev->tidev->pcidev = brdLocation.DeviceNumber;
	pDev->tidev->pcislot = brdLocation.SlotNumber;
	pDev->tidev->deviceId = DeviceID;
	pDev->tidev->revision = RevisionID;
	pDev->tidev->mem_pciereg = brdConfiguration.PhysAddress[0];
	pDev->tidev->mem_bar1 = brdConfiguration.PhysAddress[1];
	pDev->tidev->mem_bar2 = brdConfiguration.PhysAddress[2];
	pDev->tidev->mem_bar3 = brdConfiguration.PhysAddress[3];
	pDev->tidev->mem_pciereg_va = (uintptr_t)brdConfiguration.VirtAddress[0];
	pDev->tidev->mem_bar1_va = (uintptr_t)brdConfiguration.VirtAddress[1];
	pDev->tidev->mem_bar2_va = (uintptr_t)brdConfiguration.VirtAddress[2];
	pDev->tidev->mem_bar3_va = (uintptr_t)brdConfiguration.VirtAddress[3];
	pDev->tidev->bar1_size = brdConfiguration.Size[1];
	pDev->tidev->bar2_size = brdConfiguration.Size[2];
	pDev->tidev->bar3_size = brdConfiguration.Size[3];
	pDev->tidev->irq = brdConfiguration.InterruptLevel;

	pDev->ncores = TI6678_CORES;

	pDev->sysmem_ob_idx = 30;	// system memory OB_BAR number
	pDev->sysmem_ob_size = 2;	// OB_BAR size (0-1M,1-2M,2-4M,3-8M)

	pDev->sysmem_padr = brdConfiguration.PhysAddress[4];
	pDev->sysmem_vadr = (U32*)brdConfiguration.VirtAddress[4];
	pDev->sysmem_size = 0x400000; // default 4MBytes

	pDev->appmem_padr = brdConfiguration.PhysAddress[5];
	pDev->appmem_vadr = (U32*)brdConfiguration.VirtAddress[5];
	pDev->appmem_size = 0x400000; // default 4MBytes

	BRDCHAR nameMutex[MAX_PATH];
	BRDC_sprintf(nameMutex, _BRDC("mutex_B6678PEX%d"), idxThisBrd);
	pDev->hMutex = IPC_createMutex(nameMutex, FALSE);

	return btB6678;

}


//
// End of file
//

