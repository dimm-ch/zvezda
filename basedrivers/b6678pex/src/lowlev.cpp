/********************************************************************
 *  (C) Copyright 1992-2012 InSys Corp.  All rights reserved.
 *
 *  ======== lowlev.cpp ========
 *  Entry point BRD Driver for Boards:
 *		FMC110PDSP
 *		FMC111PDSP
 *      FMC112PDSP
 *      FMC117PDSP
 *		FMC114VDSP
 *      PEX-SRIO
 *      DSP6678PEX
 *
 *  version 1.0
 *
 *	The file defines: LL_xxx().
 *
 *  By DOR	(sep. 2012)
 *
 ********************************************************************/

// ----- inclide files -------------------
#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>
#include	<fcntl.h>

#include	"utypes.h"
#include	"brderr.h"
#include	"brdapi.h"

#include	"b6678pex.h"
#include    "icr.h"
#include    "Icr6678.h"

#include	"icrWrite.h"
#include	"i2cFlashWrite.h"
#include	"i2cFlashRead.h"


//
//==== Constants
//

#define	SYSMEM_HOSTRAM_OFFSET	0x1000	// 0x04000/4 
#define	SYSMEM_HIORAM_OFFSET	0x11000	// 0x44000/4 

#define	MAXFREEBLOCKS			64		// max free blocks for HFIFO

//
//======= Macros
//

#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))

//
//==== Types
//


//
//==== Global Variables
//

//-----Эти переменные формируют и используются только при загрузке программы
/*
long	netBoot[64][64];        // network booting table
long	bootLinkNum[64];		// boot link numbers
long    argc_mp[64];               // счетчики аргументов для каждого процессора
BRDCHAR   argv_mp[64][9][16];        // аргументы для каждого процессора (имя программы + 8 аргументов)
                                   // максимальная длина имени - 16 

BRDCHAR    *argv_mp_curr[]= {       // текущий массив аргументов
	(BRDCHAR*)&argv_mp[0][0],		
	(BRDCHAR*)&argv_mp[1][0],
	(BRDCHAR*)&argv_mp[2][0],
	(BRDCHAR*)&argv_mp[3][0],
	(BRDCHAR*)&argv_mp[4][0],
	(BRDCHAR*)&argv_mp[5][0],
	(BRDCHAR*)&argv_mp[6][0],
	(BRDCHAR*)&argv_mp[7][0],
	(BRDCHAR*)&argv_mp[8][0],
};
*/
//----------------------------------------------------------------

static	int argv_get_curr(BRDDRV_Board *pDev, int idx, BRDCHAR *appName)
{
	int	i,j;

	for(j=0;j<16;j++) {
		pDev->argv_mp[idx][0][j] = appName[j];
	}

	for(i=0;i<9;i++) {
		pDev->argv_mp_curr[i] = (BRDCHAR*)&pDev->argv_mp[idx][i];
	}
	return 0;
}

//
//==== Functions Declaration
//
extern	long	GetBoardCFG(BRDDRV_Board *pDev, void *data, long size, U32 signature);

//static  void    LoadCompare32(long addr, void *compare, void *dbuf, long count);

//
//==== Static Functions Declaration
//
// 

//=******************** Pause() **************
//
/*static int	Pause(int counter)
{
	volatile int i;
	for(i=0;i<counter;i++);
	return 0;
}*/

//=********************** LL_clrHost ********************
//=********************************************************
S32		LL_clrHost( BRDDRV_Board *pDev, U32 node )
{
	// clear outflag
	LL_ProcPokeHost( pDev, node, 0, 0 );
	LL_ProcPokeHost( pDev, node, 4, 0 );
	for(U32 i=0;i<pDev->hostSize;i+=4)
		LL_ProcPokeHost( pDev, node, i, 0 );

	return DRVERR(BRDerr_OK);

}

//=********************** LL_clrHio ********************
//=********************************************************
S32		LL_clrHio( BRDDRV_Board *pDev, U32 node )
{
	// clear outflag
	LL_ProcPokeHio( pDev, node, 0, 0 );
	LL_ProcPokeHio( pDev, node, 4, 0 );
	for(U32 i=0;i<pDev->hioSize;i+=4)
		LL_ProcPokeHio( pDev, node, i, 0 );

	return DRVERR(BRDerr_OK);

}

//=************************ LL_ProcReset ****************
//=******************************************************
S32		LL_ProcReset( BRDDRV_Board *pDev, U32 node)
{
	static int isHWRESET = 0;
	static int isFirst = 0;

	if( (isFirst!=0) && (node!=-1) )	{
		pDev->OPENRESET = 0;
		isFirst++;
	} 
	if(node==-1) {
		pDev->OPENRESET = 1;
		isFirst++;
	}
	

	TI6678_DEVICE *dev = pDev->tidev;

	if( (pDev->HWRESET!=0) && (isHWRESET==0) )
	{
		TI6678_DEVICE *dev = pDev->tidev;

		TI6678PCIE_ProcSoftReset(dev, 0);
		IPC_delay(pDev->HWTIMEOUT);
		RestorePciCfg(pDev->hWDM);
		{
			TI6678PCIE_ReInit(dev);
			U32 x = *(U32*)(dev->mem_pciereg_va);
			x&=0xFFFFFF00;
			if( 0x4E301100 != x ) {
				return DRVERR(BRDerr_HW_ERROR);
			}
		}
		isHWRESET++;
	}

	{
		U32 x = *(U32*)(dev->mem_pciereg_va);
		x&=0xFFFFFF00;
		if( x != 0x4E301100) {
			Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_Procreset> Bad PCIe PID - 0x%08lx (0x4E301100)"), x );
			return DRVERR(BRDerr_ERROR);
		}
	}

	TI6678PCIE_ProcLocalReset(dev, node);

	if(pDev->OPENRESET != 0) { 	// clear OB  
		for(int i=0;i<32;i++) {
			*(U32*)(dev->mem_pciereg_va + 0x200 + 8*i) = 0;
			*(U32*)(dev->mem_pciereg_va + 0x204 + 8*i) = 0;
		}
		RstEvent(pDev->hWDM);
	}


	for(int i=0;i<8;i++) pDev->appEntry[i] = 0;

	if(node==-1) {
		// clear DPRAM 
		for(int i=0;i<8;i++) LL_clrHost( pDev, i );
		// clear HIO 
		for(int i=0;i<8;i++) LL_clrHio( pDev, i );
		// clear event2dsp 
		for(int i=0;i<8;i++) LL_clrEvent2dsp( pDev, i);
		for(int i=0;i<8;i++) LL_clrEvent2host( pDev, i);
	} else {
		LL_clrHost( pDev, node );
		LL_clrHio( pDev, node );
		LL_clrEvent2dsp( pDev, node);
		LL_clrEvent2host( pDev, node);
	}

	// Setting FIFO structures
	unsigned char *pp = (unsigned char*)pDev->sysmem_vadr;

	// ---------------
	// init FIFO run-time variables
	pp = (unsigned char*)pDev->sysmem_vadr;
    pp+=(0x80000-0x100);
    pDev->hfifo_in_global_cnt_adr = (unsigned long long *) (pp);								// global counter address
    pDev->hfifo_out_global_cnt_adr = (unsigned long long *)(pp+sizeof(unsigned long long));	// global counter address
    pDev->hfifo_in_global_offset_adr = (U32 *)(pp+sizeof(unsigned long long)+4);	// global address offset address
    pDev->hfifo_out_global_offset_adr = (U32 *)(pp+sizeof(unsigned long long)+8);	// global address offset address

	pDev->hfifo_out_que = (HFIFO_STRUCT*)(pDev->sysmem_vadr + 0xC0000/4 );
	pDev->hfifo_in_que = (HFIFO_STRUCT*)(pDev->sysmem_vadr + 0x80000/4 );

    pDev->free_blocks_in = (U32*)( pDev->sysmem_vadr+0x78000/4);
    pDev->free_blocks_out = (U32*)( pDev->sysmem_vadr+0x78400/4);

	pDev->hfifo_in_mem_mask = (unsigned long long *)(pp+32);	// global input memory mask
    pDev->hfifo_out_mem_mask = (unsigned long long *)(pp+64);	// global output memory mask

	if(pDev->OPENRESET != 0) 
	{
		for(int i=0;i<64;i++) pDev->hfifo_out_que[i].tag = 0;

		{
			U32 *dsp_hfifo_in_que_curr_adr = (U32 *)(pp+sizeof(unsigned long long)+12);
			U32 *dsp_hfifo_out_que_curr_adr = (U32 *)(pp+sizeof(unsigned long long)+16);
			*dsp_hfifo_in_que_curr_adr = 0;
			*dsp_hfifo_out_que_curr_adr = 0;
		}

		*pDev->hfifo_in_global_cnt_adr = 0xabcdef; // write signature
		*pDev->hfifo_in_global_offset_adr = 0;
		*pDev->hfifo_out_global_offset_adr = 0;

		for(int i=0;i<8;i++) {
			pDev->hfifo_in_que_curr[i] = 0;
			pDev->hfifo_out_que_curr[i] = 0;
			pDev->hfifo_in_local_cnt[i]=0;		// local counter for this CPU
			pDev->hfifo_out_local_cnt[i]=0;		// local counter for this CPU
		}

		U32 x = *pDev->hfifo_out_global_offset_adr;
		if(x!=0) 
			return DRVERR(BRDerr_ERROR);

		for(int i=0;i<4;i++) {
			pDev->hfifo_in_mem_mask[i] = 0;	// global input memory mask
			pDev->hfifo_out_mem_mask[i] = 0;	// global output memory mask
		}

		for(int i=0;i<MAXFREEBLOCKS;i++) {
			pDev->free_blocks_in[i] = 0;
			pDev->free_blocks_out[i] = 0;
		}
		for(int i=0;i<MAXFREEBLOCKS;i++) {	
			pDev->hfifo_in_que[i].tag = 0;
			pDev->hfifo_in_que[i].cpuID = -1;
			pDev->hfifo_in_que[i].local_cnt = -1;
		}
		for(int i=0;i<MAXFREEBLOCKS;i++) {	
			pDev->hfifo_out_que[i].tag = 0;
			pDev->hfifo_out_que[i].cpuID = -1;
			pDev->hfifo_out_que[i].local_cnt = -1;
		}

		for(int i=0;i<8;i++) pDev->sigCounter[i] = 0;
		pDev->sigCounterTotal = 0;
		pDev->sigCounterLast  = 0;
		pDev->firstWrite  = 0;
	}
	for(int i=0;i<9;i++)
		pDev->argv_mp_curr[i] = (BRDCHAR*)&pDev->argv_mp[i][0];		

	return DRVERR(BRDerr_OK);
}

//=********************** LL_clrEvent2dsp ********************
//=********************************************************
S32		_LL_clrEvent2dsp( BRDDRV_Board *pDev)
{
	int	i;
	unsigned long long *evt2dsp = ((unsigned long long *)pDev->sysmem_vadr)+0x400;

	for(i=0;i<0x400;i++)
		evt2dsp[i] = 0;

	return DRVERR(BRDerr_OK);

}

S32		LL_clrEvent2dsp( BRDDRV_Board *pDev, U32 node)
{
	int	i;
	volatile U32 *evt2dsp_base = (U32*)((U08*)pDev->sysmem_vadr + 0x74100);
	TI6678_DEVICE *dev = pDev->tidev;

	// read DSP event address
	U32 evt2dsp_adr = evt2dsp_base[node];

	for(i=0;i<8;i++) {
		unsigned long long evt2dsp = 0;
		evt2dsp_adr+=i*8;
		TI6678PCIE_ProcWriteMem(dev, node, evt2dsp_adr, (U32*)&evt2dsp, 8);
	}
	
	return DRVERR(BRDerr_OK);

}

S32		LL_clrEvent2host( BRDDRV_Board *pDev, U32 node)
{
	int	i;
	volatile U32 *evt2host_base = (U32*)((U08*)pDev->sysmem_vadr);
	TI6678_DEVICE *dev = pDev->tidev;

	// read HOST event address
	U32 evt2host_adr = evt2host_base[node];

	for(i=0;i<8;i++) {
		unsigned long long evt2host = 0;
		evt2host_adr+=i*8;
		TI6678PCIE_ProcWriteMem(dev, node, evt2host_adr, (U32*)&evt2host, 8);
	}
	
	return DRVERR(BRDerr_OK);

}

//=********************** LL_ProcStart ********************
//=********************************************************
S32		LL_ProcStart( BRDDRV_Board *pDev, U32 node )
{
	TI6678_DEVICE *dev = pDev->tidev;
	{
		U32 sig = 0x193A5B7C;

		*(U32*)(dev->mem_pciereg_va+GPR(0)) = sig; 
		*(U32*)(dev->mem_pciereg_va+GPR(1)) = (pDev->sysmem_ob_idx<<16) | pDev->sysmem_ob_size; 
		*(U32*)(dev->mem_pciereg_va+GPR(2)) = (U32)pDev->sysmem_padr; 
		*(U32*)(dev->mem_pciereg_va+GPR(3)) = (U32)(pDev->sysmem_padr>>32); 

		U32 *pp = (U32 *)(pDev->sysmem_vadr + 0x00064000/4); 
		pp[0] = pDev->appmem_ob_idx;
		pp[1] = pDev->appmem_ob_size;
		pp[2] = (U32)pDev->appmem_padr;
		pp[3] = (U32)(pDev->appmem_padr>>32);

		for(int i=0;i<8;i++) {
			if( pDev->appEntry[i] != 0) 
			{

				LL_clrHost( pDev, i );

				TI6678PCIE_ProcStart(dev, i, pDev->appEntry[i]);
				//Dev_Printf( BRDdm_WARN, CURRFILE, _BRDC("<LL_ProcStart> Core %d successfully started."), i);
				//printf( "<LL_ProcStart> Core %d successfully started.\n",i);
				IPC_delay(100);
			}
		}
	}

	return DRVERR(BRDerr_OK);

}


//=********************** LL_ProcDSPINT *******************
//=********************************************************
S32		__LL_ProcDSPINT( BRDDRV_Board *pDev, U32 node, U32 sigId )
{
	TI6678_DEVICE *dev = pDev->tidev;
	U32 _sigId;
	volatile unsigned long long *evt2dsp = (unsigned long long *)(pDev->sysmem_vadr);
	evt2dsp+=0x400;

	if(node==(U32)-1) { // all nodes
		for(int i=0;i<8;i++) {
			_sigId = 8*i+sigId;
			volatile unsigned long long x = evt2dsp[_sigId];
			x++;
			evt2dsp[_sigId] = x;
		}
	} else {
		_sigId = 8*node+sigId;
		volatile unsigned long long x = evt2dsp[_sigId];
		x++;
		evt2dsp[_sigId] = x;
	}
//---------------------------------------------------------------
	{
		TI6678PCIE_SendInterrupt(dev, node);
	}
//---------------------------------------------------------------

	return DRVERR(BRDerr_OK);
}

//=********************** LL_ProcDSPINT *******************
//=********************************************************
S32		LL_ProcDSPINT( BRDDRV_Board *pDev, U32 node, U32 sigId )
{
	TI6678_DEVICE *dev = pDev->tidev;
	volatile U32 *evt2dsp_base = (U32*)((U08*)pDev->sysmem_vadr + 0x74100);

	// read DSP event address
	U32 evt2dsp_adr = evt2dsp_base[node];

	evt2dsp_adr+=sigId*8;

	volatile unsigned long long evt2dsp;

	TI6678PCIE_ProcReadMem(dev, node, evt2dsp_adr, (U32*)&evt2dsp, 8);
	evt2dsp++;
	TI6678PCIE_ProcWriteMem(dev, node, evt2dsp_adr, (U32*)&evt2dsp, 8);

	TI6678PCIE_SendInterrupt(dev, node);

	return DRVERR(BRDerr_OK);
}

//=********************** LL_ProcGrabIrq ******************
//=********************************************************
S32		LL_ProcGrabIrq( BRDDRV_Board *pDev, U32 node, U32 sigId, U32 *pSigCounter, U32 timeout )
{
	TI6678_EVENT data_event;  

	data_event.EventNum = (node << 3) + sigId;
	data_event.EventCount = 0;

	int			 to = (int)timeout;
	if(to<0) data_event.Timeout = 200000;
	else data_event.Timeout = timeout;

	if(pDev->hMutex) 
        IPC_captureMutex(pDev->hMutex, INFINITE);

	if(WaitEvent(pDev->hWDM, &data_event) != BRDerr_OK)
	{
		if(pDev->hMutex)
			IPC_releaseMutex(pDev->hMutex);
		*pSigCounter = 0;
		return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
	}

	if(pDev->hMutex)
        IPC_releaseMutex(pDev->hMutex);

	if( data_event.EventCount == 0) 
		return DRVERR(BRDerr_NO_DATA);

	*pSigCounter = data_event.EventCount;
	return DRVERR(BRDerr_OK);
}

//=********************** LL_ProcWaitIrq ******************
//=********************************************************
S32		LL_ProcWaitIrq( BRDDRV_Board *pDev, U32 node, U32 sigId, U32 *pSigCounter, U32 timeout )
{
	TI6678_EVENT data_event;  

	data_event.EventNum = (sigId << 3) + node;
	data_event.EventCount = 0;

	int			 to = (int)timeout;
	if(to<0) data_event.Timeout = 200000;
	else data_event.Timeout = timeout;

	unsigned long long *pEvents = (unsigned long long *)pDev->sysmem_vadr;
	pEvents[data_event.EventNum] = 0;

	if(WaitEvent(pDev->hWDM, &data_event) != BRDerr_OK)
	{
		*pSigCounter = 0;
		return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
	}

	if( data_event.EventCount == 0 )
		return DRVERR(BRDerr_NO_DATA);
	
	*pSigCounter = data_event.EventCount;
	return DRVERR(BRDerr_OK);
}

//=********************** LL_ProcFreshIrq ******************
//=********************************************************
S32		LL_ProcFreshIrq( BRDDRV_Board *pDev, U32 node, U32 sigId, U32 *pSigCounter )
{
	U32 event_num = (sigId << 3) + node;
	unsigned long long *pEvents = (unsigned long long *)pDev->sysmem_vadr;
	pEvents[event_num] = 0;

	return DRVERR(BRDerr_OK);
}

/********************** LL_ProcSignalIack ************
********************************************************/
S32		LL_ProcSignalIack(BRDDRV_Board *pDev, U32 node)
{
	return DRVERR(BRDerr_OK);
}


/********************** LL_ProcReadMem ************
********************************************************/
S32		LL_ProcReadMem( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *dst, S32 sz )
{
	TI6678_DEVICE *dev = pDev->tidev;
	U32 core = node;
	TI6678PCIE_ProcReadMem(dev, core, adr, (U32*)dst, sz);
	return DRVERR(BRDerr_OK);
}

/********************** LL_ProcWriteMem ***********
********************************************************/
S32		LL_ProcWriteMem( BRDDRV_Board *pDev, U32 node , U32 adr, U32 src, S32 sz )
{
	TI6678_DEVICE *dev = pDev->tidev;
	U32 core = node;
	TI6678PCIE_ProcWriteMem(dev, core, adr, (U32*)&src, sz);
	return DRVERR(BRDerr_OK);
}

/********************** LL_ProcReadMemDump ********
 len - dump size in bytes.
********************************************************/
S32		LL_ProcReadMemDump( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pDst, U32 len )
{
	TI6678_DEVICE *dev = pDev->tidev;
	U32 core = node;
	TI6678PCIE_ProcReadMem(dev, core, adr, (U32*)pDst, len);
	return DRVERR(BRDerr_OK);
}

/********************** LL_ProcWriteMemDump ***************
********************************************************/
S32		LL_ProcWriteMemDump( BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 len )
{
	TI6678_DEVICE *dev = pDev->tidev;
	U32 core = node;
	TI6678PCIE_ProcWriteMem(dev, core, adr, (U32*)pSrc, len);
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcPeekHost **************************
// offset - bytes
//=********************************************************
S32		LL_ProcPeekHost( BRDDRV_Board *pDev, U32 node, U32 offset, U32 *dst )
{
	U32 adr;
	adr = (offset/4)+(node*0x2000)/4;
	*dst = *(pDev->sysmem_vadr + SYSMEM_HOSTRAM_OFFSET + adr);
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcPokeHost **************************
//=********************************************************
S32		LL_ProcPokeHost( BRDDRV_Board *pDev, U32 node, U32 offset, U32 src )
{
	U32 adr;
	adr = (offset/4)+(node*0x2000)/4;
	*(pDev->sysmem_vadr + SYSMEM_HOSTRAM_OFFSET + adr) = src;
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcReadHost **************************
//=********************************************************
S32		LL_ProcReadHost( BRDDRV_Board *pDev, U32 node, U32 offset, U32 *pDst, U32 len )
{
	U32 *ptr = pDst;
	for(U32 i=0;i<len;i+=4) {
		LL_ProcPeekHost( pDev, node, offset+i, ptr );
		ptr++;
	}
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcWriteHost **************************
//=********************************************************
S32		LL_ProcWriteHost(BRDDRV_Board *pDev, U32 node, U32 offset, U32 *pSrc, U32 len )
{
	U32 *ptr = pSrc;
	for(unsigned int i=0;i<len;i+=4) {
		LL_ProcPokeHost( pDev, node, offset+i, *ptr );
		ptr++;
	}
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcPeekDPRAM **************************
// offset - bytes
//=********************************************************
S32		LL_ProcPeekDPRAM( BRDDRV_Board *pDev, U32 node, U32 offset, U32 *dst )
{
	U32 adr = (offset/4);
	*dst = *(pDev->appmem_vadr + adr);
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcPokeDPRAM **************************
//=********************************************************
S32		LL_ProcPokeDPRAM( BRDDRV_Board *pDev, U32 node, U32 offset, U32 src )
{
	U32 adr = (offset/4);
	*(pDev->appmem_vadr + adr) = src;
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcReadDPRAM **************************
//=********************************************************
S32		LL_ProcReadDPRAM( BRDDRV_Board *pDev, U32 node, U32 offset, U32 *pDst, U32 len )
{
	U32 *ptr = pDst;

	for(U32 i=0;i<len;i+=4) {
		LL_ProcPeekDPRAM( pDev, node, offset+i, ptr );
		ptr++;
	}
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcWriteDPRAM **************************
//=********************************************************
S32		LL_ProcWriteDPRAM(BRDDRV_Board *pDev, U32 node, U32 offset, U32 *pSrc, U32 len )
{
	U32 *ptr = pSrc;
	for(unsigned int i=0;i<len;i+=4) {
		LL_ProcPokeDPRAM( pDev, node, offset+i, *ptr );
		ptr++;
	}
	return DRVERR(BRDerr_OK);
}


//=********************** LL_hexGetByte ****************
//=*****************************************************
S32 LL_hexGetByte( BRDCHAR **ppLine, U32 *pByte )
{
	U32		symb1, symb2;

	symb1 = BRDC_toupper(*(*ppLine)++);
	symb1 = (symb1>='0' && symb1<='9') ? symb1-'0' :
			(symb1>='A' && symb1<='F') ? symb1-'A'+10 : 0xFE;

	symb2 = BRDC_toupper(*(*ppLine)++);
	symb2 = (symb2>='0' && symb2<='9') ? symb2-'0' :
			(symb2>='A' && symb2<='F') ? symb2-'A'+10 : 0xFE;

	if( pByte!=NULL )
		*pByte = symb1*16 + symb2;

	return (symb1==0xFE||symb2==0xFE) ? -1 : 0;
}

//=********************** LL_ProcLoad ****************
//=*****************************************************
S32		LL_ProcLoad( BRDDRV_Board *pDev, U32 node, const BRDCHAR *filename, int argc, BRDCHAR *argv[] )
{
        int      i,ret;
        int      length = (int)BRDC_strlen(filename);
        BRDCHAR  fname[512];

        BRDC_strcpy(fname,filename);

		//=== check file extension
		//for(i=0;i<length;i++) {
		for(i=length-1;i>0;i--) {
			if( filename[i] == '.') {
				i++;	        
				if( (filename[i] != '.') && (filename[i] != '\\') ){
					i--;
					break;
				}
			}	
		}

		if( i==length) {		// can't find extension
			BRDC_strcat(fname, _BRDC(".bin") );
        } else {
			if( BRDC_strncmp(&fname[i], _BRDC(".tix"),3) == 0) {	// multicore config file
		       ret = LL_MpConfig(pDev,fname);
               if( ret != -1) {
                   ret = LL_MpLoad(pDev,node,fname);
               } else {
                   return 0;
               }
               
			   if( ret != 0)	return 1;
               else				return 0;

			} 
			else if( BRDC_strncmp(&fname[i], _BRDC(".bin"),4) == 0) {  // load binary file
				U32	entry;
				TI6678_DEVICE *dev = pDev->tidev;
				LL_ProcReset(pDev,node);
				if ( TI6678PCIE_ProcLoadBin(dev, node, filename, &entry) < 0 ) {
					Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_LinkBootLoad> Error Booting File '%s' (cpu %ld)"),fname,1);
					return 0;
				}
				pDev->appEntry[node] = entry;
				LL_LoadArg(pDev,node,argc,argv);

				return 1;
			}
        }
		return DRVERR(BRDerr_ERROR);
}


//=********************* LL_LoadIcrBIN *****************
//=*****************************************************
S32		LL_LoadIcrBIN(BRDDRV_Board *pDev, U32 pldId, FILE *fin )
{
	U32		byte;
	U32		addr;


	if(fin==NULL)
		return DRVERR(BRDerr_BAD_FILE);
	//
	//=== Load Code
	//
	addr = 0x80;
	fseek( fin, 0L, SEEK_SET );
	for(;;)
	{
		byte=fgetc(fin);
		if( byte==(U32)EOF )
			if(feof(fin))								// If End of File
				break;
		addr++;
	}

	return DRVERR(BRDerr_OK);
}

//=********************* LL_LoadIcrHEX *****************
//=*****************************************************
S32		LL_LoadIcrHEX(BRDDRV_Board *pDev, U32 pldId, FILE *fin )
{
	U32		byte, val, size, tag;
	U32		eaddr, addr;
	BRDCHAR	line[200], *pLin;
	U32		ii;

	if(fin==NULL)
		return DRVERR(BRDerr_BAD_FILE);

	//
	//=== Load Code
	//
	eaddr = 0;
	fseek( fin, 0L, SEEK_SET );
	for(;;)
	{
		if(feof(fin))									// If End of File
			break;
		if(0==BRDC_fgets( line, 200, fin ))
			continue;
		if( line[0] != ':' )
			continue;
		pLin = line+1;

		//=== Read Line Header
		LL_hexGetByte( &pLin, &size );
		LL_hexGetByte( &pLin, &val );
		LL_hexGetByte( &pLin, &addr );
		LL_hexGetByte( &pLin, &tag );
		addr |= val<<8;

		//=== Check Tag
		if( tag == 1 )									// If End of File
			break;
		if( tag == 4 )									// If Extended Adr
		{
			LL_hexGetByte( &pLin, &val );
			LL_hexGetByte( &pLin, &eaddr );
			eaddr <<= 16;
			eaddr  |= val<<24;
			continue;
		}
		if( tag != 0 )									// If Unknown Tag
			continue;

		//
		//=== Read Data from Line
		//
		addr += 0x80;
		addr |= eaddr;
		for( ii=0; ii<size; ii++ )
		{
			//int		loop;

			if( 0>LL_hexGetByte( &pLin, &byte ) )
			{
				return Dev_ErrorPrintf( DRVERR(BRDerr_HW_ERROR), 
							pDev, CURRFILE, _BRDC("<LL_LoadIcrHEX> HEX file contents bad data") );
			}
			
			addr++;
		}
	}

	return DRVERR(BRDerr_OK);
}


//=************************ LL_LoadArg ***************
//=*****************************************************
S32		LL_LoadArg(BRDDRV_Board *pDev, U32 node, int argc, BRDCHAR *argv[])
{
	unsigned char *aPtr08 = (U08 *)(pDev->sysmem_vadr + 0x00065000/4 + (node*0x100)/4);
	U32           *aPtr32 = (U32 *)(pDev->sysmem_vadr + 0x00065000/4 + (node*0x100)/4);

	// save arguments
	{
		int         i, j;
		for(j=0;j<256;j++)
			aPtr08[j] = 0;
		aPtr32[0] = argc; // write argc counter
		for(i=0; i<argc; i++) 
		{
			aPtr32[3+i] = (int)BRDC_strlen(argv[i]); // write size of string argv[i]
		}
		aPtr08 = (U08*)&aPtr32[3+argc];
		aPtr08+=(4*argc);
		for(i=0; i<argc; i++) 
		{
			for(j=0; j<(int)BRDC_strlen(argv[i]) ; j++) 
				aPtr08[j] = argv[i][j]; // write string
			aPtr08[j] = 0;			// write zero end string
			aPtr08 += ((int)BRDC_strlen(argv[i])+1); 
		}
	}

	return(0);
}



//=********************** LL_putMsg *******************
//=*****************************************************
S32 LL_putMsg( BRDDRV_Board *pDev, U32 node, void *pParam )
{
	DEV_CMD_PutMsg	*ptr = (DEV_CMD_PutMsg*)pParam;

	volatile U32				count, currSize;
    U32				*hostAdr = (U32*)ptr->hostAdr;
	volatile U32				val;
	volatile U32	tCnt = 0;

	//
	// Send Total Size to Board (in U32)
	//
    currSize = ptr->size;
    for( ;; ) 
	{
		LL_ProcPeekHost( pDev, node, 0x0*4, (U32*)&val );
		if( val == 0 ) 
			break;
		if(ptr->timeout!=(U32)-1) 
		{
			tCnt++;
			if(tCnt>=ptr->timeout) 
			{
				ptr->size = 0;
				return BRDerr_WAIT_TIMEOUT;
			}
		}
    	//_kbhit();
	}

    for( ;; ) 
	{
		LL_ProcPeekHost( pDev, node, 0x1*4, (U32*)&val );
		if( val == 0 ) 
			break;
		if(ptr->timeout!=(U32)-1) 
		{
			tCnt++;
			if(tCnt>=ptr->timeout) 
			{
				ptr->size = 0;
				return BRDerr_WAIT_TIMEOUT;
			}
		}
		//_kbhit();
    }

	LL_ProcPokeHost( pDev, node, 0x0*4, currSize );

	//
	// Write Buffer to Board
	//
    while( currSize>0) 
	{
		count=min( 0x800l, currSize );
		tCnt = 0;
		for( ;; ) 
		{
			LL_ProcPeekHost( pDev, node, 0x1*4, (U32*)&val );
			if( val == 0 ) 
				break;
			if(ptr->timeout!=(U32)-1) 
			{
				tCnt++;
				if(tCnt>=ptr->timeout) {
					ptr->size = 0;
					return BRDerr_WAIT_TIMEOUT;
				}
			}
			//_kbhit();
		}
 		LL_ProcWriteHost( pDev, node, 4*4, hostAdr, count );
		LL_ProcPokeHost( pDev, node, 0x1*4, count );
		currSize -= count;

		hostAdr  += count/4;
	}
	return DRVERR(BRDerr_OK);
}

//=********************** LL_getMsg *******************
//=*****************************************************
S32		LL_getMsg( BRDDRV_Board *pDev, U32 node, void *pParam )
{
	DEV_CMD_GetMsg	*ptr = (DEV_CMD_GetMsg*)pParam;
    volatile U32				count,currSize,_size;
    U32				*hostAdr = (U32*)ptr->hostAdr;
	volatile U32	tCnt = 0;
	
	//
    // Get Buffer Size
	//
	for( ;; ) 
	{
		LL_ProcPeekHost( pDev, node, 0x2*4, (U32*)&_size );
		if( _size != 0l ) 
		{
			break;
		}
		if(ptr->timeout!=(U32)-1) 
		{ 
			tCnt++;
			if(tCnt>=ptr->timeout) 
			{
				ptr->size = 0;
				return BRDerr_WAIT_TIMEOUT;
			}
			IPC_delay(0);
		}
#ifdef __linux__
        if(IPC_kbhit())
            return 0;
#endif		//_kbhit();
	}	

	//
	// Check Buffer Size
	//
    //if( _size*4 > ptr->size ) 
	if( _size > ptr->size ) 
	{
		return Dev_ErrorPrintf( DRVERR(BRDerr_BUFFER_TOO_SMALL), 
								pDev, CURRFILE, _BRDC("<LL_getMsg> BIG SIZE OF ARRAY %08lX (%08lX)."), _size, ptr->size );
    }
	//
	// Read Buffer from Board
	//
	currSize = _size;
    while( currSize>0) 
	{
		tCnt = 0;
		for( ;; ) 
		{
 			LL_ProcPeekHost( pDev, node, 0x3*4, (U32*)&count );

			if( count != 0l ) 
				break;
			if(ptr->timeout!=(U32)-1) 
			{
				tCnt++;
				if(tCnt>=ptr->timeout) 
				{
					ptr->size = 0;
					return BRDerr_WAIT_TIMEOUT;
				}
				IPC_delay(0);
			}
			//_kbhit();
		}

		LL_ProcReadHost( pDev, node, 4*4+0x800, hostAdr, count );
 		LL_ProcPokeHost( pDev, node, 0x3*4, 0 );

		currSize -= count;

		hostAdr  += count/4;
    }
 	LL_ProcPokeHost( pDev, node, 0x2*4, 0 );

	ptr->size = _size;

	return DRVERR(BRDerr_OK);
}
//=********************** LL_icrRead ************
//=*****************************************************
S32		LL_icrRead( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size )
{
	//=== Get ICR
	if(GetICR(pDev->hWDM, pDev->eeprom) != BRDerr_OK)
	{
		IPC_closeDevice(pDev->hWDM);
		return DRVERR(BRDerr_NO_KERNEL_SUPPORT);
	}
	memcpy(buf,pDev->eeprom+offset,size);
	return 0;
}

//=********************** LL_icrWrite ************
//=*****************************************************
static UCHAR	tmpbuf[1024];
S32		LL_icrWrite( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size )
{
	U32	entry;
	TI6678_DEVICE *dev = pDev->tidev;

	LL_ProcReset(pDev,-1);

	if ( TI6678PCIE_ProcLoadMem(dev, 0, icrWrite, sizeof(icrWrite), &entry) ) {
		Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_puWrite> Error Load ICR Write App") );
		return 0;
    } else {
		pDev->appEntry[0] = entry;
//	    Dev_Printf( BRDdm_INFO, CURRFILE, "<LL_puWrite> Load ICR Write App success.");
    }

	LL_ProcStart(pDev, 0);

	DEV_CMD_PutMsg pmsg;
	pmsg.nodeId = 0;
	pmsg.hostAdr = buf;
	pmsg.size = 0x100;//size;
	pmsg.timeout = (U32)-1;
	LL_putMsg(pDev,0,&pmsg);

	// read ack
	DEV_CMD_GetMsg gmsg;
	gmsg.nodeId = 0;
	gmsg.hostAdr = tmpbuf;
	gmsg.size = 0x100;//size;
	gmsg.timeout = (U32)-1;
	LL_getMsg(pDev,0,&gmsg);

	// read ICR memory
	gmsg.nodeId = 0;
	gmsg.hostAdr = tmpbuf;
	gmsg.size = 0x100;//size;
	gmsg.timeout = (U32)-1;
	LL_getMsg(pDev,0,&gmsg);

	LL_ProcReset(pDev,-1);

	if(ReadICR(pDev->hWDM) != BRDerr_OK)
		return DRVERR(BRDerr_NO_KERNEL_SUPPORT);

	return 0;
}

//=********************** LL_i2cFlashWrite ************
//=*****************************************************
S32		LL_i2cFlashWrite( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size )
{
	U32	entry;
	TI6678_DEVICE *dev = pDev->tidev;

	LL_ProcReset(pDev,-1);

	if ( TI6678PCIE_ProcLoadMem(dev, 0, i2cFlashWrite, sizeof(i2cFlashWrite), &entry) ) {
		Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_puWrite> Error Write I2C ROM") );
		return 0;
    } else {
		pDev->appEntry[0] = entry;
//	    Dev_Printf( BRDdm_INFO, CURRFILE, "<LL_puWrite> Load ICR Write App success.");
    }

	LL_ProcStart(pDev, -1);

	// send offset and size
	DEV_CMD_PutMsg pmsg;

	U32		x[2];
	x[0] = offset;
	x[1] = size;
	
	pmsg.nodeId = 0;
	pmsg.hostAdr = &x;
	pmsg.size = 8;
	pmsg.timeout = (U32)-1;
	LL_putMsg(pDev,0,&pmsg);

	// send data
	pmsg.nodeId = 0;
	pmsg.hostAdr = buf;
	pmsg.size = size;
	pmsg.timeout = (U32)-1;
	LL_putMsg(pDev,0,&pmsg);

	// read ack
	DEV_CMD_GetMsg gmsg;
	gmsg.nodeId = 0;
	gmsg.hostAdr = tmpbuf;
	gmsg.size = size;
	gmsg.timeout = (U32)-1;
	LL_getMsg(pDev,0,&gmsg);

	// read memory
	gmsg.nodeId = 0;
	gmsg.hostAdr = tmpbuf;
	gmsg.size = size;
	gmsg.timeout = (U32)-1;
	LL_getMsg(pDev,0,&gmsg);

	LL_ProcReset(pDev,-1);

	return 0;
}

//=********************** LL_i2cFlashRead ************
//=*****************************************************
S32		LL_i2cFlashRead( BRDDRV_Board *pDev, U32 puId, U32 offset, void* buf, U32 size )
{
	U32	entry;
	TI6678_DEVICE *dev = pDev->tidev;

	LL_ProcReset(pDev,-1);

	if ( TI6678PCIE_ProcLoadMem(dev, 0, i2cFlashRead, sizeof(i2cFlashRead), &entry) ) {
		Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_puRead> Error Read I2C ROM ") );
		return 0;
    } else {
		pDev->appEntry[0] = entry;
//	    Dev_Printf( BRDdm_INFO, CURRFILE, "<LL_puWrite> Load ICR Write App success.");
    }

	LL_ProcStart(pDev, 0);

	// send offset and size
	DEV_CMD_PutMsg pmsg;
	U32		x[2];
	x[0] = offset;
	x[1] = size;
	pmsg.nodeId = 0;
	pmsg.hostAdr = &x;
	pmsg.size = 8;
	pmsg.timeout = (U32)-1;
	LL_putMsg(pDev,0,&pmsg);

	// read memory
	DEV_CMD_PutMsg gmsg;
	gmsg.nodeId = 0;
	gmsg.hostAdr = buf;
	gmsg.size = size;
	gmsg.timeout = (U32)-1;
	LL_getMsg(pDev,0,&gmsg);

	LL_ProcReset(pDev,-1);

	return 0;
}

//=********************** LL_readFIFO ************
//=***********************************************
S32		LL_readFIFO(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pDst, U32 size, U32 timeout)
{
	static U32 sigId = 2;
	S32	ret;
	UCHAR	*data;
	//S32	_SIZE=0;
	//static int	cnt_dbg = 0;

	// read current struct
	pDev->hfifo_out_que = (HFIFO_STRUCT*)(pDev->sysmem_vadr + 0xC0000/4 );

	if(pDev->sigCounter[node]==0) {
		// wait for Event
		ret = LL_ProcGrabIrq(pDev, node, sigId, &pDev->sigCounter[node], timeout );
		if(ret != DRVERR(BRDerr_OK)) {
//			printf("Timeout\n");
			return DRVERR(BRDerr_WAIT_TIMEOUT);
		}
		if(!pDev->sigCounter) {
//			printf("Timeout - sigCounter = 0\n");
			return DRVERR(BRDerr_WAIT_TIMEOUT);
		}

		pDev->sigCounterTotal+=pDev->sigCounter[node];
	} else {
//		printf("Is Sig = %d\n", sigCounter);
	}
//		printf("SigCounterTotal - %d\n", sigCounterTotal);

	volatile S32 cpuID = -1;
	int i;
	for(i=0;i<64;i++) 
	{
		if( (pDev->hfifo_out_que[i].cpuID == node) &&
		    (pDev->hfifo_out_que[i].tag == 0x11223344) &&
			(pDev->hfifo_out_que[i].local_cnt==pDev->hfifo_out_local_cnt[node]) )
		{
				pDev->hfifo_out_que_curr[node] = i;
				cpuID = pDev->hfifo_out_que[i].cpuID;
				break;
		}
	}

	if(i == 64) {
//		Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_readFIFO> Error fifo data from node %d "), node );
		return -1;
	}

	U32 que_curr = pDev->hfifo_out_que_curr[node];

	//volatile U32 tag = pDev->hfifo_out_que[que_curr].tag;
	volatile U32 offset = pDev->hfifo_out_que[que_curr].offset;
	volatile U32 _size = pDev->hfifo_out_que[que_curr].size;
	//volatile unsigned long long global_cnt = pDev->hfifo_out_que[que_curr].global_cnt;
	//volatile unsigned long long local_cnt = pDev->hfifo_out_que[que_curr].local_cnt;
	//volatile U64 mask = pDev->hfifo_out_que[que_curr].mask[0];
	//volatile U64 gmask = pDev->hfifo_out_mem_mask[0];

	if(offset>=0x400000) {
		//printf("ERROR: BIG OFFSET = %08Xh\n", offset);
		return -2;
	}


	data = (UCHAR*)(pDev->sysmem_vadr + 0x200000/4 + offset/4);

	memcpy(pDst,data,_size);

	pDev->sigCounter[node]--;

	for(int i=0;i<MAXFREEBLOCKS;i++) {
   		U64 m = 0x8000000000000000ULL;
   		m>>=i;
   		if( (pDev->hfifo_out_que[que_curr].mask[0] & m) == 0)
   			pDev->free_blocks_out[i]=0;
	}
	pDev->hfifo_out_local_cnt[node]++;
	
	// free blocks
	pDev->hfifo_out_que[que_curr].cpuID = -1;
	pDev->hfifo_out_que[que_curr].local_cnt = -1;
	pDev->hfifo_out_que[que_curr].tag = 0;

//	__asm	nop;

	return _size;
}

//=********************** LL_writeFIFO ************
//=***********************************************
S32		LL_writeFIFO(BRDDRV_Board *pDev, U32 node, U32 adr, U32 *pSrc, U32 size, U32 timeout)
{
	UCHAR	*data;
	S32	offset;
	U32 que_curr;

    // find free QUEUE 
	int		i;
	for(i=0;i<8;i++) {
		if( (pDev->hfifo_in_que[8*node+i].tag == 0) &&
			(pDev->hfifo_in_que[8*node+i].cpuID == (U32)-1) &&
			(pDev->hfifo_in_que[8*node+i].local_cnt == (unsigned long long)-1) )
		{
			que_curr = 8*node+i;
			break;
		}
	}

	if(i==8) {
    	return -1;
	}

    // make global memory mask
    unsigned long long g_mask=0x0000000000000000ULL;
	for(U32 i=(node*8);i<((node*8)+8);i++) {
    	unsigned long long m = 0x8000000000000000ULL;
    	if(pDev->free_blocks_in[i]!=0) g_mask|=(m>>i);
    }

	// check for memory free
    {
    	// get current mask
    	int sect = size/0x4000;
    	if( size%0x4000 ) sect++;
    	long long mask = 0x8000000000000000ULL;
    	mask>>=(sect-1);

    	offset = -1;
    	for(int i=(node*8);i<=MAXFREEBLOCKS-sect;i++) {
    		unsigned long long p_mask = mask;
    		unsigned long long n_mask = ~mask;
    		p_mask>>=i;
    		n_mask=~p_mask;
    		if((g_mask|n_mask) == n_mask) {	// is memory space
    			offset = 0x4000*i;
    			pDev->hfifo_in_mem_mask[0] |= p_mask;
    			pDev->hfifo_in_que[que_curr].mask[0] = n_mask;
    			break;
    		}
    	}
    }

    if(offset==-1) {
    	return -2;
    }

	for(int i=0;i<MAXFREEBLOCKS;i++) {
    	unsigned long long m = 0x8000000000000000ULL;
    	m>>=i;
    	if( (pDev->hfifo_in_que[que_curr].mask[0] & m) == 0)
    		pDev->free_blocks_in[i]=0x43211234;
    }

	data = (UCHAR*)(pDev->sysmem_vadr + 0x100000/4 + offset/4);

	memcpy(data, pSrc, size);

    pDev->hfifo_in_que[que_curr].cpuID = node;
    pDev->hfifo_in_que[que_curr].offset = offset;
    pDev->hfifo_in_que[que_curr].size = size;
	pDev->hfifo_in_que[que_curr].global_cnt=*pDev->hfifo_in_global_cnt_adr;
	pDev->hfifo_in_que[que_curr].local_cnt=pDev->hfifo_in_local_cnt[node];
	pDev->hfifo_in_que[que_curr].tag = 0x44332211;

	pDev->hfifo_in_local_cnt[node]++;

	LL_ProcDSPINT( pDev, node, 2 );

	return DRVERR(BRDerr_OK);
} 

#ifdef __linux__
U32 GetLastError(void)
{
    //printf("GetLastError(): %s\n", strerror(IPC_sysError()));
    return -1;
}
#endif

//***************************************************************************************
ULONG GetDeviceID(IPC_handle hWDM, USHORT& DeviceID)
{
	TI6678_DATA_BUF PciConfigPtr = {NULL, sizeof(USHORT), 2};

	int res = IPC_ioctlDevice(
            hWDM, 
			IOCTL_TI6678_GET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(TI6678_DATA_BUF),
            &DeviceID,
            sizeof(USHORT));
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG GetRevisionID(IPC_handle hWDM, UCHAR& RevisionID)
{
	TI6678_DATA_BUF PciConfigPtr = {NULL, sizeof(UCHAR), 8};

	int res = IPC_ioctlDevice(
            hWDM, 
			IOCTL_TI6678_GET_BUS_CONFIG,
            &PciConfigPtr,
            sizeof(TI6678_DATA_BUF),
            &RevisionID,
            sizeof(UCHAR));
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG GetLocation(IPC_handle hWDM, TI6678_LOCATION* pLocation)
{
	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_GET_LOCATION,
            NULL,
            0,
            pLocation,
            sizeof(TI6678_LOCATION));
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG GetConfiguration(IPC_handle hWDM, TI6678_CONFIGURATION* pConfiguration)
{
	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_GET_CONFIGURATION,
            NULL,
            0,
            pConfiguration,
            sizeof(TI6678_CONFIGURATION));
	if(res < 0){
        return GetLastError();
    }
    for(int i = 0; i < 6; i++)
    {
			if(IPC_mapPhysAddr(hWDM, &pConfiguration->VirtAddress[i], pConfiguration->PhysAddress[i], pConfiguration->Size[i]))
                return BRDerr_ACCESS_VIOLATION;
	}

	return BRDerr_OK;
}

//***************************************************************************************
ULONG GetICR(IPC_handle hWDM, UCHAR* buf_icr)
{
	TI6678_DATA_BUF data_icr = {NULL, 256, 0};

	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_GET_ICR,
			&data_icr,
			sizeof(TI6678_DATA_BUF),
			buf_icr,
			256);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG ReadICR(IPC_handle hWDM)
{
	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_READ_ICR,
			NULL,
			0,
			NULL,
			0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG RstEvent(IPC_handle hWDM)
{
	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_RESET_EVENT,
			NULL,
			0,
			NULL,
			0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//***************************************************************************************
ULONG WaitEvent(IPC_handle hWDM, TI6678_EVENT* data_event)
{
	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_WAIT_EVENT,
			data_event,
			sizeof(TI6678_EVENT),
			data_event,
			sizeof(TI6678_EVENT));
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}
//***************************************************************************************
ULONG RestorePciCfg(IPC_handle hWDM)
{
	int res = IPC_ioctlDevice(
            hWDM, 
            IOCTL_TI6678_RESTORE_PCICFG,
			NULL,
			0,
			NULL,
			0);
	if(res < 0){
        return GetLastError();
    }
	return BRDerr_OK;
}

//=********************** LL_MpConfig ************
//  Get Multiprocessing Configuration from file
//=*****************************************************
//extern	BRDCHAR	loadPath[512];
S32	LL_MpConfig(BRDDRV_Board *pDev, const BRDCHAR *fname)
{
    FILE    *in;
	BRDCHAR    buf[256];
	BRDCHAR    str[256],s[256];
    BRDCHAR    *ptr;
    int     i, j;
    int     length;
	
    //=== init arguments table
    for(i=0;i<64;i++)
    {
        pDev->argc_mp[i] = 0;
        for(j=0;j<8;j++)
            pDev->argv_mp[i][j][0] = 0;
    }

	//=== open network configuration file *.tix
    in = BRDC_fopen(fname, _BRDC("rt"));
    if(in == NULL) return 1;


	// get arguments
    int k=0;
    for (i=0;i<64;i++)
    {
	    fseek(in, 0, SEEK_SET);
	    BRDC_strcpy(str, _BRDC("ARGS"));
	    BRDC_strcat(str, BRDC_itoa(i,s,10));
	    length = (int)BRDC_strlen(str);
	    // find number
	    for (;;)
        {
		    BRDC_fgets(buf,128,in);
	        if(feof(in)) break;
	        if( BRDC_strncmp(str,buf,length) != 0) continue;
	        if( *(buf+length+1)!=' ') continue;
	        ptr = buf+length+1;
            k = 1;
            pDev->argc_mp[i] = 1;   // один аргумент - имя самой программы 
            int endOfStr = 0;
            for(;;)
            {
                while(1) { 
                    if( *ptr == 0x0a ) { endOfStr++; break; }
                    if( isspace(*ptr)!=0 ) ptr++; 
                    else { break; }
                }
                if( endOfStr !=0 ) break;
                j = 0;
                while (isspace(*ptr)==0) {
                    pDev->argv_mp[i][k][j++] = *ptr++;
                }
                pDev->argv_mp[i][k][j] = 0;
                k++;
                pDev->argc_mp[i]++;
	        }
        }
    }

    fclose(in);

	return DRVERR(BRDerr_OK);
}

//=********************** LL_MpLoad ************
//  Load Applications 
//=*****************************************************
S32	LL_MpLoad(BRDDRV_Board *pDev, U32 node, const BRDCHAR *fname)
{
	FILE    *in;
	BRDCHAR   *ptr;
	BRDCHAR   str[512];
    BRDCHAR   buf[512];
	BRDCHAR   bFileName[512];
	int     i,j,length;

	//=== read network configuration file *.tix
	in = BRDC_fopen(fname, _BRDC("rt") );
	if(in == NULL) return 1;

	for (i=0;i<64;i++)
    {
	    fseek(in,0L,SEEK_SET);
			//=== find node
	    BRDC_strcpy(str,_BRDC("NODE") );
	    BRDC_itoa(i,buf,10);
	    BRDC_strcat(str,buf);
	    length = (int)BRDC_strlen(str);
	    for (;;)
        {
		    BRDC_fgets(buf,127,in);
		    if(feof(in)) break;
		    if (BRDC_strncmp(str,buf,length) != 0) continue;
		    if( *(buf+length+1) != ' ') continue;
		    ptr = buf+length;
			for(j=0;j<(int)BRDC_strlen(ptr);j++) {
				if( isalnum(*ptr++) != 0) break;
			}
		    ptr--;
			for (j=0;j<(int)BRDC_strlen(ptr);j++) {
		        if( isspace(ptr[j]) != 0 ) break;
			}
		    BRDC_strncpy(bFileName,ptr,j);
		    bFileName[j]='\0';
			if(	bFileName[0]=='\0' ) break;

			// add path to file
			BRDCHAR	bFile[512];
                        BRDCHAR	loadPath[256] = _BRDC("./");
			BRDC_strcpy(bFile,loadPath);
			BRDC_strcat(bFile,bFileName);

			U32	entry;
			TI6678_DEVICE *dev = pDev->tidev;
			if ( TI6678PCIE_ProcLoadBin(dev, i, bFile, &entry) < 0 ) {
					Dev_Printf( BRDdm_ERROR, CURRFILE, _BRDC("<LL_MpLoad> Error Load File '%s' (cpu %ld)"),bFile,i);
					return 0;
		    } else {
				pDev->appEntry[i] = entry;
			    Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<LL_MpLoad> Core %d loaded success."),i);
				argv_get_curr(pDev, i, bFileName);
				LL_LoadArg(pDev,i,pDev->argc_mp[i],pDev->argv_mp_curr);
			    Dev_Printf( BRDdm_INFO, CURRFILE, _BRDC("<LL_MpLoad> Core %d loaded args success."),i);
            }
		    //printf( "<LL_MpLoad> Core %d loaded args success.",i);

			break;
	    }
	}

    fclose(in);

	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcPeekHio **************************
// offset - bytes
//=********************************************************
S32		LL_ProcPeekHio( BRDDRV_Board *pDev, U32 node, U32 offset, U32 *dst )
{
	U32 adr;
	adr = (offset/4)+(node*0x2000)/4;
	*dst = *(pDev->sysmem_vadr + SYSMEM_HIORAM_OFFSET + adr);
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcPokeHio **************************
//=********************************************************
S32		LL_ProcPokeHio( BRDDRV_Board *pDev, U32 node, U32 offset, U32 src )
{
	U32 adr;
	adr = (offset/4)+(node*0x2000)/4;
	*(pDev->sysmem_vadr + SYSMEM_HIORAM_OFFSET + adr) = src;
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcReadHio **************************
//=********************************************************
S32		LL_ProcReadHio( BRDDRV_Board *pDev, U32 node, U32 offset, U32 *pDst, U32 len )
{
	U32 *ptr = pDst;
	for(U32 i=0;i<len;i+=4) {
		LL_ProcPeekHio( pDev, node, offset+i, ptr );
		ptr++;
	}
	return DRVERR(BRDerr_OK);
}

//=*************** LL_ProcWriteHio **************************
//=********************************************************
S32		LL_ProcWriteHio(BRDDRV_Board *pDev, U32 node, U32 offset, U32 *pSrc, U32 len )
{
	U32 *ptr = pSrc;
	for(unsigned int i=0;i<len;i+=4) {
		LL_ProcPokeHio( pDev, node, offset+i, *ptr );
		ptr++;
	}
	return DRVERR(BRDerr_OK);
}

//=********************** LL_read *******************
//=*****************************************************
S32		LL_read( BRDDRV_Board *pDev, U32 node, void *pParam )
{
	DEV_CMD_ReadWrite	*ptr = (DEV_CMD_ReadWrite*)pParam;
    volatile U32				count,currSize,_size;
    U32				*hostAdr = (U32*)ptr->hostAdr;
	volatile U32	tCnt = 0;
	
	//
    // Get Buffer Size
	//
	for( ;; ) 
	{
		LL_ProcPeekHio( pDev, node, 0x2*4, (U32*)&_size );
		if( _size != 0l ) 
		{
			break;
		}
		if(ptr->timeout!=(U32)-1) 
		{ 
			tCnt++;
			if(tCnt>=ptr->timeout) 
			{
				ptr->size = 0;
				return BRDerr_WAIT_TIMEOUT;
			}
			IPC_delay(0);
		}
#ifdef __linux__
        if(IPC_kbhit())
            return 0;
#endif		//_kbhit();
	}	

	//
	// Check Buffer Size
	//
    //if( _size*4 > ptr->size ) 
	if( _size > ptr->size ) 
	{
		return Dev_ErrorPrintf( DRVERR(BRDerr_BUFFER_TOO_SMALL), 
								pDev, CURRFILE, _BRDC("<LL_read> BIG SIZE OF ARRAY %08lX (%08lX)."), _size, ptr->size );
    }
	//
	// Read Buffer from Board
	//
	currSize = _size;
    while( currSize>0) 
	{
		tCnt = 0;
		for( ;; ) 
		{
 			LL_ProcPeekHio( pDev, node, 0x3*4, (U32*)&count );

			if( count != 0l ) 
				break;
			if(ptr->timeout!=(U32)-1) 
			{
				tCnt++;
				if(tCnt>=ptr->timeout) 
				{
					ptr->size = 0;
					return BRDerr_WAIT_TIMEOUT;
				}
				IPC_delay(0);
			}
			//_kbhit();
		}

		LL_ProcReadHio( pDev, node, 4*4+0x800, hostAdr, count );
 		LL_ProcPokeHio( pDev, node, 0x3*4, 0 );

		currSize -= count;

		hostAdr  += count/4;
    }
 	LL_ProcPokeHio( pDev, node, 0x2*4, 0 );

	ptr->size = _size;

	return DRVERR(BRDerr_OK);
}

//=********************** LL_write *******************
//=*****************************************************
S32 LL_write( BRDDRV_Board *pDev, U32 node, void *pParam )
{
	DEV_CMD_ReadWrite	*ptr = (DEV_CMD_ReadWrite*)pParam;
	volatile U32				count, currSize;
    U32				*hostAdr = (U32*)ptr->hostAdr;
	volatile U32				val;
	volatile U32	tCnt = 0;

	//
	// Send Total Size to Board (in U32)
	//
    currSize = ptr->size;
    for( ;; ) 
	{
		LL_ProcPeekHio( pDev, node, 0x0*4, (U32*)&val );
		if( val == 0 ) 
			break;
		if(ptr->timeout!=(U32)-1) 
		{
			tCnt++;
			if(tCnt>=ptr->timeout) 
			{
				ptr->size = 0;
				return BRDerr_WAIT_TIMEOUT;
			}
		}
    	//_kbhit();
	}

    for( ;; ) 
	{
		LL_ProcPeekHio( pDev, node, 0x1*4, (U32*)&val );
		if( val == 0 ) 
			break;
		if(ptr->timeout!=(U32)-1) 
		{
			tCnt++;
			if(tCnt>=ptr->timeout) 
			{
				ptr->size = 0;
				return BRDerr_WAIT_TIMEOUT;
			}
		}
		//_kbhit();
    }

	LL_ProcPokeHio( pDev, node, 0x0*4, currSize );

	//
	// Write Buffer to Board
	//
    while( currSize>0) 
	{
		count=min( 0x800l, currSize );
		tCnt = 0;
		for( ;; ) 
		{
			LL_ProcPeekHio( pDev, node, 0x1*4, (U32*)&val );
			if( val == 0 ) 
				break;
			if(ptr->timeout!=(U32)-1) 
			{
				tCnt++;
				if(tCnt>=ptr->timeout) {
					ptr->size = 0;
					return BRDerr_WAIT_TIMEOUT;
				}
			}
			//_kbhit();
		}
 		LL_ProcWriteHio( pDev, node, 4*4, hostAdr, count );
		LL_ProcPokeHio( pDev, node, 0x1*4, count );
		currSize -= count;

		hostAdr  += count/4;
	}
	return DRVERR(BRDerr_OK);
}

//
// End of file
//

