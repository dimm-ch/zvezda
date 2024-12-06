//=********************************************************
//
// BASESTRM.H
//
// BRD_BaseStream Base Class
//
// (C) Instrumental Systems
//
// Created: Ekkore Feb. 2003
//
//=********************************************************


#ifndef	__BASESTRM_H_
#define	__BASESTRM_H_

#include	"ctrlstrm.h"
#include	"gipcy.h"

//
// Shareable (Global) Control Structure of CBUF
//
typedef struct
{
//	S08		rootBlockName[128];		// Root Name of Blocks
	U32		rootCounter;			// Counter to Form Root Name of Blocks
	U32		blkNum;					// Number of Blocks
	U32		blkSize;				// Size of every Block (bytes)
	U32		state;					// CBUF global state
	U32		dir;					// Direction
	U32		isCont;					// Allocation Method: 0-FileMapping, 1-System Pool (Ring 0)
} BRDstrm_ShareData, *PBRDstrm_ShareData;

//
// Block Structure
//
typedef struct
{
	U08			*pBlock;				// Block Pointer
	IPC_handle	hFileMap;				// Handle of Block
} BRDstrm_BlockPointer, *PBRDstrm_BlockPointer;


typedef struct
{
	BRDstrm_Stub	*pStub;			// Stub Pointer
	IPC_handle		hFileMap;		// Handle of Stub
} BRDstrm_StubPointer, *PBRDstrm_StubPointer;


//=********************************************************
//
// Class BRD_BaseStrm
//
//=********************************************************
class BRD_BaseStrm
{
public:

	//
	// Variables
	//
	BRDstrm_ShareData*		m_pShareData;		// Shareable Control Structure of CBUF
	IPC_handle				m_hFileMap;			// Handle of shareable memory for "pShareControl"
    IPC_handle				m_hMutex;			// Named Mutex to protect "pShareControl"
	BRDCHAR					m_rootName[128];	// Root Name of Blocks

	//
	// Variables for ECLUSIVE Capture Mode
	//
	U32						m_rootCounterExcl;		// Counter of Current Root Name

	S32						m_cntCaptModeExcl;		// Resource Capture Mode (-1-unused, 0-exclu, 2-spy)
	BRDstrm_BlockPointer*	m_pBlockPointersExcl;	// Array of Block Pointers
	BRDstrm_StubPointer		m_stubPointerExcl;		// Stub Pointer
	U32						m_blkNumExcl;			// Number of Blocks
	U32						m_blkSizeExcl;			// Size of every Block (bytes)
	U32						m_isCont;				// Keep isCont value

	//
	// Variables for SPY Capture Mode
	//
	U32						m_rootCounterSpy;		// Counter of Current Root Name

	S32						m_cntCaptModeSpy;		// Resource Capture Mode (-1-unused, 0-exclu, 2-spy)
	BRDstrm_BlockPointer*	m_pBlockPointersSpy;	// Array of Block Pointers
	BRDstrm_StubPointer		m_stubPointerSpy;		// Stub Pointer
	U32						m_blkNumSpy;			// Number of Blocks
	U32						m_blkSizeSpy;			// Size of every Block (bytes)


	//
	// Methods
	//

	BRD_BaseStrm( const BRDCHAR *devName, const BRDCHAR *rsrcName );
	virtual ~BRD_BaseStrm( );

    PVOID operator new(size_t Size) {return IPC_heapAlloc(Size);};
    VOID operator delete(PVOID pVoid) {if(pVoid) IPC_heapFree(pVoid); };
	
	virtual S32	IsValid( void );

	S32			CBufAllocExclusive( U32 blkNum, U32 blkSize, U32 dir, U32 isCont, U08 **ppBlk, BRDstrm_Stub **ppStub );
	S32			CBufAllocSpy( U32 blkNum, U32 *pBlkNumReal, U32 *pBlkSize, U32 *pDir, U08 **ppBlk, BRDstrm_Stub **ppStub );
	S32			CBufFreeExclusive( void );
	S32			CBufFreeSpy( void );

	virtual S32	LockBlocks( void )     {return BRDerr_ERROR;};
	virtual S32	UnlockBlocks( void )   {return BRDerr_ERROR;};
	virtual S32	AllocBlocks( void )    {return BRDerr_ERROR;};
	virtual S32	DisallocBlocks( void ) {return BRDerr_ERROR;};

	//
	// Helper Functions
	//
protected:
	S32			CBufAllocDiscontinuous( U32 blkNum, U32 blkSize, U32 dir, U08 **ppBlk, BRDstrm_Stub **ppStub );	// isCont=0
	S32			CBufFreeDiscontinuous( void );
	S32			CBufAllocContinuous( U32 blkNum, U32 blkSize, U32 dir, U08 **ppBlk, BRDstrm_Stub **ppStub );	// isCont=1
	S32			CBufFreeContinuous( void );
	S32			CBufAllocSelfcreated( U32 blkNum, U32 blkSize, U32 dir, U08 **ppBlk, BRDstrm_Stub **ppStub );	// isCont=2
	S32			CBufFreeSelfcreated( void );
	void		ClearBlocksExcl( void );
	void		ClearBlocksSpy( void );

	//=***************** BRD_BaseStrm::MutexWait **************
	//=********************************************************
	void		MutexWait( void )
	{
        if( m_hMutex ) IPC_captureMutex( m_hMutex, INFINITE );
	}

	//=***************** BRD_BaseStrm::MutexFree **************
	//=********************************************************
	void		MutexFree( void )
	{
        if( m_hMutex ) IPC_releaseMutex( m_hMutex );
	}

};


#endif	//__BASESTRM_H_


//
// End of File
//

