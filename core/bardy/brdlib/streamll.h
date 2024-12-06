//****************** File Streamll.h *********************************
//  Structures for stream service
//
//	Copyright (c) 2006, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  01-12-06 - builded
//
//*******************************************************************

#ifndef __linux__

#pragma pack(push,1)

typedef struct _AMB_MEM_DMA_CHANNEL {
	ULONG	DmaChanNum;		// IN
	ULONG	Direction;
	ULONG	LocalAddr;
	ULONG	MemType;
	ULONG	BlockCnt;
	ULONG	BlockSize;
	PVOID	pStub;
	HANDLE	hBlockEndEvent;
#ifdef _WIN64
	HANDLE	hTransEndEvent;
#endif
	PVOID	pBlock[1];
} AMB_MEM_DMA_CHANNEL, *PAMB_MEM_DMA_CHANNEL;

typedef struct _AMB_START_DMA_CHANNEL {
	ULONG	DmaChanNum;		// IN
	ULONG	IsCycling;
} AMB_START_DMA_CHANNEL, *PAMB_START_DMA_CHANNEL;

typedef struct _AMB_STATE_DMA_CHANNEL {
	ULONG	DmaChanNum;		// IN
	LONG	BlockNum;		// OUT
	ULONG	BlockCntTotal;	// OUT
	ULONG	OffsetInBlock;	// OUT		
	ULONG	DmaChanState;	// OUT		
	LONG	Timeout;		// IN
} AMB_STATE_DMA_CHANNEL, *PAMB_STATE_DMA_CHANNEL;

typedef struct _AMB_SET_DMA_CHANNEL {
	ULONG	DmaChanNum;		// IN
	ULONG	Param;
} AMB_SET_DMA_CHANNEL, *PAMB_SET_DMA_CHANNEL;

typedef struct _AMB_GET_DMA_INFO {
	ULONG	DmaChanNum;		// IN
	ULONG	Direction;		// OUT
	ULONG	FifoSize;		// OUT
	ULONG	MaxDmaSize;		// OUT
//	ULONG	PciAddr;		// OUT
//	ULONG	LocalAddr;		// OUT
} AMB_GET_DMA_INFO, *PAMB_GET_DMA_INFO;

typedef struct _AMB_SET_IRQ_TABLE {
	ULONG	DmaChanNum;		// IN
	ULONG	Mode;
	ULONG	TableNum;
	ULONG	AddrTable[4];
} AMB_SET_IRQ_TABLE, *PAMB_SET_IRQ_TABLE;

#pragma pack(pop)
#endif

#ifdef __linux__
typedef struct _AMB_MEM_DMA_CHANNEL {
        u32	DmaChanNum;		// IN
        u32	Direction;
        u32	LocalAddr;
        u32	MemType;
        u32	BlockCnt;
        u32	BlockSize;
        void*	pStub;
        int	hBlockEndEvent;
#ifdef _WIN64
        HANDLE	hTransEndEvent;
#endif
        void*	pBlock[1];
} AMB_MEM_DMA_CHANNEL, *PAMB_MEM_DMA_CHANNEL;

typedef struct _AMB_START_DMA_CHANNEL {
        u32	DmaChanNum;		// IN
        u32	IsCycling;
} AMB_START_DMA_CHANNEL, *PAMB_START_DMA_CHANNEL;

typedef struct _AMB_STATE_DMA_CHANNEL {
        u32	DmaChanNum;		// IN
        s32	BlockNum;		// OUT
        u32	BlockCntTotal;	// OUT
        u32	OffsetInBlock;	// OUT
        u32	DmaChanState;	// OUT
        s32	Timeout;		// IN
} AMB_STATE_DMA_CHANNEL, *PAMB_STATE_DMA_CHANNEL;

typedef struct _AMB_SET_DMA_CHANNEL {
        u32	DmaChanNum;		// IN
        u32	Param;
} AMB_SET_DMA_CHANNEL, *PAMB_SET_DMA_CHANNEL;

typedef struct _AMB_GET_DMA_INFO {
        u32	DmaChanNum;		// IN
        u32	Direction;		// OUT
        u32	FifoSize;		// OUT
        u32	MaxDmaSize;		// OUT
//	u32	PciAddr;		// OUT
//	u32	LocalAddr;		// OUT
} AMB_GET_DMA_INFO, *PAMB_GET_DMA_INFO;

typedef struct _AMB_SET_IRQ_TABLE {
	u32	DmaChanNum;		// IN
	u32	Mode;
	u32	TableNum;
	u32	AddrTable[4];
} AMB_SET_IRQ_TABLE, *PAMB_SET_IRQ_TABLE;

#endif

/*
// PLD error type
enum {
	PLD_errOK,
	PLD_errFORMAT,		// file format or check sum error
//	PLD_errPROG,		// Programming error
	PLD_errROM,			// PLD programming from ROM only 
	PLD_errNOTRDY		// PLD not ready for programming 
};
*/
// ****************** End of file Streamll.h **********************
