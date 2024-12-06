/*
 ****************** File Dio32OutRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for DIO32OUT
 *
 * (C) InSys by Dorokhin Andrey Sep, 2007
 *
 ************************************************************
*/

#ifndef _DIO32OUTREGS_H_
#define _DIO32OUTREGS_H_

#pragma pack(push,1)

// DIO32OUT Mode1 register (+9)
typedef union _DIO32OUT_MODE1 {
  ULONG AsWhole; // DIO32OUT Mode1 Register as a Whole Word
  struct { // DIO32OUT Mode1 Register as Bit Pattern
   ULONG ChanSel	: 2,	// Channel select
//		 Packing	: 1,	// 1 - 8 bits mode
		 Reserved0	: 2,	// Reserved
		 OutClkInv	: 1,	// 1 - External clock inverting
		 OutClkMode	: 1,	// 0 - continued clock, 1 - only when data transfering
		 Reserved1	: 1,	// Reserved
		 PnpkMode	: 1,	// 1 - when PNPK-signal=1 non output
		 InitOut	: 1,	// init signal when InitMode=11
		 InitMode	: 2,	// init signal mode
		 Reserved2	: 5;	// Reserved
  } ByBits;
} DIO32OUT_MODE1, *PDIO32OUT_MODE1;

// DIO32OUT Status Register
typedef union _DIO32OUT_STATUS {
	ULONG AsWhole; // Board Status Register as a Whole Word
	struct { // Status Register as Bit Pattern
		ULONG	CmdRdy		: 1, // Ready to do command
				FifoRdy		: 1, // Ready FIFO 
				Empty		: 1, // Empty FIFO
				AlmostEmpty	: 1, // Almost Empty FIFO
				HalfFull	: 1, // Half Full FIFO
				AlmostFull	: 1, // Almost Full FIFO
				Full		: 1, // Full FIFO
				Overflow	: 1, // Overflow FIFO
				Underflow	: 1, // Underflow FIFO
				Reserved0	: 3, // Reserved
				F_Frame		: 1, // 1 - было передано хотя бы одно слово
				Start		: 1, // 0 - действует условие передачи данных
				Reserved1	: 2; // Reserved
	} ByBits;
} DIO32OUT_STATUS, *PDIO32OUT_STATUS;

// DIO32OUT FLAG_CLR register
typedef union _DIO32OUT_FLAGCLR {
  ULONG AsWhole; // DIO32OUT FLAG_CLR Register as a Whole Word
  struct { // DIO32OUT FLAG_CLR Register as Bit Pattern
   ULONG Reserved0	: 12, // Reserved
		 ClrF_Frame	: 1,  // reset F_Frame
		 Reserved1	: 3;  // reserved
  } ByBits;
} DIO32OUT_FLAGCLR, *PDIO32OUT_FLAGCLR;

// Numbers of Constant Registers
typedef enum _DIO32OUT_NUM_DIRECT_REGS {
	DIO32OUT_nrFLAGCLR	= 0x200,
} DIO32OUT_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_DIO32OUTREGS_H_
