/*
 ****************** File StreamSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : Stream section
 *
 * (C) InSys by Dorokhin Andrey Feb, 2004
 *
 * 27.12.2006 - add functions SysBuf(), FileMapBuf(), VirtualBuf() - implement non-auto component buffer
 *
 ************************************************************
*/

#ifndef _STREAMSRV_H
 #define _STREAMSRV_H

#include "ctrlstrm.h"
#include "service.h"
#include "streamll.h"

#define MAX_NUMBER_OF_STREAMS 2

enum {
	STRMcmd_SETMEMORY	= 1,
	STRMcmd_FREEMEMORY,
	STRMcmd_STARTMEMORY,
	STRMcmd_STOPMEMORY,
	STRMcmd_STATEMEMORY,
	STRMcmd_SETDIR,
	STRMcmd_SETSRC,
	STRMcmd_SETDRQ,
	STRMcmd_RESETFIFO,
	STRMcmd_GETDMACHANINFO,
	STRMcmd_ADJUST,
	STRMcmd_DONE,
	STRMcmd_WAITDMABUFFER,
	STRMcmd_WAITDMABLOCK,
	STRMcmd_SETIRQMODE,
	STRMcmd_GETIRQCNT
};

#pragma pack(push,1)

typedef struct _STREAMSRV_CFG {
	U32		dummy;
} STREAMSRV_CFG, *PSTREAMSRV_CFG;

// Block Structure
typedef struct _STREAMSRV_BLOCK {
	PVOID	pBlock;				// Block Pointer
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_handle	hFileMap;			// Handle of Block
#else
	HANDLE	hFileMap;			// Handle of Block
#endif
} STREAMSRV_BLOCK, *PSTREAMSRV_BLOCK;

// Stub Structures
//typedef struct _STREAMSRV_STUB {
//	PBRD_StreamStub	pStub;			// Stub Pointer
//	HANDLE			hFileMap;		// Handle of Stub
//} STREAMSRV_STUB, *PSTREAMSRV_STUB;

#pragma pack(pop)

class CStreamSrv: public CService
{

protected:

	long	m_StreamNum;
//	long m_MainTetrNum;
//	long m_TetradNum;
	ULONG	m_isCycling;
	ULONG	m_dirs[MAX_NUMBER_OF_STREAMS];
	ULONG	m_dir;
	ULONG	m_isSysMem;
	ULONG	m_blkNum;
	ULONG	m_blkSize;
	PSTREAMSRV_BLOCK	m_pStrmBlk;
	PBRDstrm_Stub		m_pStub;			// Stub Pointer
	ULONG	m_irqMode;
	ULONG	m_irqCount;
//	PSTREAMSRV_STUB		m_pStrmStub;

	//void*	m_SGTableAddress;
	//void*	m_SGTblSaveAddress;
	//ULONG	m_bTableSize;

#ifdef _WIN32
	OVERLAPPED m_OvlStartStream;	// событие окончания всего составного буфера
	HANDLE m_hBlockEndEvent;	// событие окончания блока
#endif

	int SysBuf(CBaseModule* pModule, PBRDctrl_StreamCBufAlloc pStream, PAMB_MEM_DMA_CHANNEL pMemDescrip, int DescripSize);
	int FileMapBuf(CBaseModule* pModule, PBRDctrl_StreamCBufAlloc pStream, PAMB_MEM_DMA_CHANNEL pMemDescrip, int DescripSize);
	int VirtualBuf(CBaseModule* pModule, PBRDctrl_StreamCBufAlloc pStream, PAMB_MEM_DMA_CHANNEL pMemDescrip, int DescripSize);

//	ULONG Stop(void* pDev, PBRD_StreamState pState);
	ULONG Stop(void* pDev);
	ULONG Free(void* pDev);

public:

	CStreamSrv(int idx, int srv_num, CModule* pModule, PSTREAMSRV_CFG pCfg); // constructor
	~CStreamSrv(); // destructor

//	virtual ULONG IsCmd(ULONG cmd);

	int CtrlIsAvailable(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlAllocBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlAttachBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlFreeBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlStartBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlStopBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlStateBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlWaitBufIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlWaitBufIoEx(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlWaitBlockIo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlWaitBlockIoEx(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlSetDir(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlSetSrc(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlSetDrq(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlResetFifo(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlSetIrqMode(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlAdjust(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					);

	int CtrlDone(
				void		*pDev,		// InfoSM or InfoBM
				void		*pServData,	// Specific Service Data
				ULONG		cmd,		// Command Code (from BRD_ctrl())
				void		*args 		// Command Arguments (from BRD_ctrl())
				);

	//int CtrlGetDir(
	//				void		*pDev,		// InfoSM or InfoBM
	//				void		*pServData,	// Specific Service Data
	//				ULONG		cmd,		// Command Code (from BRD_ctrl())
	//				void		*args 		// Command Arguments (from BRD_ctrl())
	//				);

};

#endif // _STREAMSRV_H

//
// End of file
//
