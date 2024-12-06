/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== b6678pex.h ========
 *  TI6678 BASE Driver Declaration for Boards: 
 *		FMC110
 *		FMC112
 *		FMC114
 *		FMC117
 *		FMC114
 *		PEX-SRIO
 *		DSP6678PEX
 *
 *  
 *	version 2.0
 *
 *  By DOR	(sep. 2012)
 ********************************************************************/

#ifndef	__B6678PEX_H_
#define	__B6678PEX_H_


#include	"gipcy.h"
#include	"brdapi.h"
#include	"brdfunx.h"
#include	"ti6678_pcie.h"
#include	"ddw6678pex.h"

//
//==== Constants & Macros
//

#define	CURRFILE		_BRDC("B6678PEX")
#define	btB6678			0x6678
#define	VER_MAJOR		0x00000001L
#define	VER_MINOR		0x00000002L

#define	BOOT_HOST  0x0
#define	BOOT_LINK  0x2
#define	BOOT_EPROM 0x4
#define	BOOT_SLAVE 0x10

#define	STREAM_MAX  2

enum    BOARD_TYPES {
        UNDEFINED = 0,
        FMC110    = 1,
        FMC112    = 2,
        FMC114    = 3
};

enum { szNULL=0, szU08=1, szU16=2, szU32=4 };

typedef struct HFIFO_STRUCT {
    U32 tag;
    U32 cpuID;
    U32 offset;				// data offset address
    U32 size;
    unsigned long long local_cnt;	// local counter for this CPU
    unsigned long long global_cnt;	// global counter for this CPU
    unsigned long long mask[4];		// global counter for this CPU

}HFIFO_STRUCT;

// Define Geniric Device Extension Type BRDDRV_Board
typedef struct SBRDDRV_Board	BRDDRV_Board;
struct SBRDDRV_Board
{
	U32			pid;					// Board PID
	IPC_handle	hWDM;					// WDM Driver Handle if Use
	BRDCHAR		boardName[32];			// Unique Board Name
	
	IPC_handle	hMutex;					// 
	TI6678_DEVICE	*tidev;
	U32		ncores;

	U32			argsAdr,  argsSize;		// .args Segment Adr and Size 

	SINT32		errcode; 		// Driver Last Error Number 
	SINT32		errcnt;	 		// Driver Error Counter 

	U32							baseServListSize;	// Size of Service List

	//	USER MEMBERS
	U32			BOARD;			// Board type
	U32			BOOT;			// BOOT Mode
	U08			eeprom[512];	// ICR copy
	U32			OPENRESET;
	U32			HWRESET;
	U32			HWTIMEOUT;
	//////////////////
	U32			EEPROM_ACCESS;   // флаг разрешения полного доступа к EEPROM (1 - полный доступ)

	U32			appEntry[MAX_TI6678_DEVICES];	// application entry point

	U32			hostAddr;		// DPRAM simulate address
	U32			hostSize;		// DPRAM simulate address

	U32			hioAddr;		// DPRAM simulate address
	U32			hioSize;		// DPRAM simulate address

	U32			start;			// 0 - single core, 1 - all cores

	U32			sysmem_ob_idx;	// system memory OB_BAR number
	U32			sysmem_ob_size;	// OB_BAR size
	unsigned long long sysmem_padr;	// system memory physical address
	U32			*sysmem_vadr;	// system memory virtual address
	U32			sysmem_size;	// system memory size

	U32			appmem_ob_idx;	// app memory OB_BAR number
	U32			appmem_ob_size;	// OB_BAR size
	unsigned long long appmem_padr;	// application system memory physical address (DPRAM)
	U32			*appmem_vadr;	// application system memory virtual address (DPRAM)
	U32			appmem_size;	// application system memory size

	// variables for FIFO functions
	U32			sigCounter[8];
	U32			sigCounterTotal;
	U32			sigCounterLast;
	U32			firstWrite;

	HFIFO_STRUCT	*hfifo_in_que;
	HFIFO_STRUCT	*hfifo_out_que;
	U32	hfifo_in_que_curr[8];
	U32	hfifo_out_que_curr[8];
	unsigned long long hfifo_in_local_cnt[8];		// local counter for this CPU
	unsigned long long hfifo_out_local_cnt[8];		// local counter for this CPU

    U32     	*hfifo_in_global_offset_adr;		    // global address offset address
	unsigned long long *hfifo_in_global_cnt_adr;		// global counter address
    U32     	*hfifo_out_global_offset_adr;           // global address offset address
	unsigned long long *hfifo_out_global_cnt_adr;		// global counter address

	unsigned long long *hfifo_in_mem_mask;			    // global input memory mask
	unsigned long long *hfifo_out_mem_mask;			// global input memory mask

    U32	*free_blocks_in;				// free blocks array HOST to DSP
    U32	*free_blocks_out;				// free blocks array DSP to HOST

	long	netBoot[64][64];        // network booting table
	long	bootLinkNum[64];		// boot link numbers
	long    argc_mp[64];               // счетчики аргументов для каждого процессора
	BRDCHAR   argv_mp[64][9][16];        // аргументы для каждого процессора (имя программы + 8 аргументов)
                                   // максимальная длина имени - 16 
	BRDCHAR    *argv_mp_curr[9];       // текущий массив аргументов
};

typedef struct 
{
	U32		pid;					// Data from Registry or INI File
	U32		pcibus, pcidev, pcislot;// Data from Registry or INI File
	BRDCHAR	boot[16];
	U32		openreset;
	U32		hwreset;
	U32		hwtimeout;
	BRDCHAR	eeprom_access[16];
	U32		hostAddr;
	U32		hostSize;
	U32		hioAddr;
	U32		hioSize;
	BRDCHAR	start[16];
	U32		order;					// order from INI File
} TIniData;



//
//==== Global Variables
//

//
//==== Internal Functions
//

//
// B6678PEX.CPP
//

S32		Dev_ErrorPrintf( S32 errorCode, void *ptr, const BRDCHAR *title, const BRDCHAR *format, ... );
void	Dev_Printf( S32 errLevel, const BRDCHAR *title, const BRDCHAR *format, ... );
S32		Dev_CheckSignature( U32 pid );
S32	 Dev_AllocBoard( void **ppDev, HANDLE hWDM, BRDCHAR *pBoardName );
S32	 Dev_FreeBoard( BRDDRV_Board *pDev );
S32	 Dev_ParseBoard( TIniData *pIniData, void **ppDev, BRDCHAR *pBoardName, S32 idxThisBrd );

//
// and also the standard entry points:
//


//
// LOWLEV.CPP
//

S32		LL_ProcReset( BRDDRV_Board *pDev,U32 node );
S32		LL_ProcStart( BRDDRV_Board *pDev, U32 node );
S32		LL_ProcDSPINT( BRDDRV_Board *pDev, U32 node, U32 sigId );
S32		LL_ProcGrabIrq( BRDDRV_Board *pDev, U32 node, U32 sigId, U32 *pSigCounter, U32 timeout );
S32		LL_ProcWaitIrq( BRDDRV_Board *pDev, U32 node, U32 sigId, U32 *pSigCounter, U32 timeout );
S32		LL_ProcFreshIrq(BRDDRV_Board *pDev, U32 node, U32 sigId, U32 *pSigCounter );
S32		LL_ProcSignalIack(BRDDRV_Board *pDev, U32 node);
S32		LL_ProcReadMem( BRDDRV_Board *pDev, U32 node , U32 adr, U32 *dst, S32 sz );
S32		LL_ProcWriteMem( BRDDRV_Board *pDev, U32 node , U32 adr, U32 src, S32 sz );
S32		LL_ProcReadMemDump( BRDDRV_Board *pDev, U32 node , U32 adr, U32 *pSrc, U32 len );
S32		LL_ProcWriteMemDump( BRDDRV_Board *pDev, U32 node , U32 adr, U32 *pSrc, U32 len );

S32		LL_DevReadMem( BRDDRV_Board *pDev, U32 node , U32 adr, U32 *dst, S32 sz );
S32		LL_DevWriteMem( BRDDRV_Board *pDev, U32 node , U32 adr, U32 src, S32 sz );

S32		LL_ProcPeekHost( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *dst );
S32		LL_ProcPokeHost( BRDDRV_Board *pDev, U32 node, U32 adr, U32 src );
S32		LL_ProcReadHost( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len );
S32		LL_ProcWriteHost(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len );

S32		LL_ProcPeekHio( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *dst );
S32		LL_ProcPokeHio( BRDDRV_Board *pDev, U32 node, U32 adr, U32 src );
S32		LL_ProcReadHio( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len );
S32		LL_ProcWriteHio(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len );

S32		LL_ProcPeekDPRAM( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *dst );
S32		LL_ProcPokeDPRAM( BRDDRV_Board *pDev, U32 node, U32 adr, U32 src );
S32		LL_ProcReadDPRAM( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len );
S32		LL_ProcWriteDPRAM(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len );

S32		LL_HexGetByte( BRDCHAR **ppLine, U32 *pByte );
S32		LL_LoadArg(BRDDRV_Board *pDev, U32 node,int argc,BRDCHAR *argv[]);
S32		LL_ProcLoad( BRDDRV_Board *pDev, U32 node, const BRDCHAR *filename, int argc, BRDCHAR *argv[] );
S32		LL_MpConfig(BRDDRV_Board *pDev, const BRDCHAR *filename);
S32		LL_MpLoad(BRDDRV_Board *pDev, U32 node, const BRDCHAR *fname);

S32		LL_LoadIcrBIN(BRDDRV_Board *pDev, U32 pldId, FILE *fin );
S32		LL_LoadIcrHEX(BRDDRV_Board *pDev, U32 pldId, FILE *fin );
S32		LL_LoadPlis( BRDDRV_Board *pDev, U32 pldId, const BRDCHAR *fname, U32 *pState );
S32		LL_LoadPlisReset( BRDDRV_Board *pDev, U32 pldId );
S32		LL_LoadPlisWrite( BRDDRV_Board *pDev, U32 pldId, U32 val, U32 size );
S32		LL_LoadPlisState( BRDDRV_Board *pDev, U32 pldId, U32 *ptr );
S32		LL_LoadArgv( BRDDRV_Board *pDev, U32 node, int argc, BRDCHAR *argv[], BRDCHAR *envp[] );

S32		LL_putMsg( BRDDRV_Board *pDev, U32 node, void *pParam );
S32		LL_getMsg( BRDDRV_Board *pDev, U32 node, void *pParam );

S32		LL_write( BRDDRV_Board *pDev, U32 node, void *pParam );
S32		LL_read( BRDDRV_Board *pDev, U32 node, void *pParam );

S32		LL_readFIFO(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pDst, U32 size, U32 timeout);
S32		LL_writeFIFO(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 size, U32 timeout);

S32		LL_icrRead( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size );
S32		LL_icrWrite( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size );

S32		LL_i2cFlashWrite( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size );
S32		LL_i2cFlashRead( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size );

S32		LL_dmaReadStart(BRDDRV_Board *pDev, U32 chan, U32 locAdr, U32 pciAdr, U32 size, U32 flag, U32 timeout);
S32		LL_dmaWriteStart(BRDDRV_Board *pDev, U32 chan, U32 locAdr, U32 pciAdr, U32 size, U32 flag, U32 timeout);

S32		LL_dmaReadState(BRDDRV_Board *pDev, U32 chan, U32 *state, U32 timeout);
S32		LL_dmaWriteState(BRDDRV_Board *pDev, U32 chan, U32 *state, U32 timeout);

S32		LL_dmaAllocBuf(BRDDRV_Board *pDev, U64 *padr, U32 **vadr, U32 size);
S32		LL_dmaFreeBuf(BRDDRV_Board *pDev, U32 **vadr);

S32		LL_clrHost( BRDDRV_Board *pDev, U32 node );
S32		LL_clrEvent2dsp( BRDDRV_Board *pDev, U32 node);
S32		LL_clrEvent2host( BRDDRV_Board *pDev, U32 node);

ULONG	GetDeviceID(IPC_handle hWDM, USHORT& DeviceID);
ULONG	GetRevisionID(IPC_handle hWDM, UCHAR& RevisionID);
ULONG	GetLocation(IPC_handle hWDM, TI6678_LOCATION* pLocation);
ULONG	GetConfiguration(IPC_handle hWDM, TI6678_CONFIGURATION* pConfiguration);
ULONG	GetICR(IPC_handle hWDM, UCHAR* buf_icr);
ULONG	ReadICR(IPC_handle hWDM);
ULONG	RstEvent(IPC_handle hWDM);
ULONG	WaitEvent(IPC_handle hWDM, TI6678_EVENT* data_event);
ULONG	RestorePciCfg(IPC_handle hWDM);

//
// SERVICE.CPP
//
S32		Serv_FillServList( BRDDRV_Board *pDev );
S32		Serv_ClearServList( BRDDRV_Board *pDev );
S32		Serv_GetServList( BRDDRV_Board *pDev, DEV_CMD_GetServList *pGRL );
S32		Serv_Ctrl( BRD_Handle handle, BRDDRV_Board *pDev, U32 cmd, void *args );

// SERVSTRM.CPP
S32		Serv_FillServListWithStream( BRDDRV_Board *pDev, S32 *pIdxMain );
S32		Serv_ClearServListWithStream( BRDDRV_Board *pDev );
S32		Serv_StreamSupportEntry( BRD_Handle handle, void *pSub, void *pServData, void *pContext, U32 cmd, void *args );

#endif	// __B6678PEX_H_

//
// End of File
//
