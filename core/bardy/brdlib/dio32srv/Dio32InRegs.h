/*
 ****************** File Dio32InRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for DIO32IN
 *
 * (C) InSys by Dorokhin Andrey Sep, 2007
 *
 ************************************************************
*/

#ifndef _DIO32INREGS_H_
#define _DIO32INREGS_H_

#pragma pack(push,1)

// DIO32IN Mode1 register (+9)
typedef union _DIO32IN_MODE1 {
  ULONG AsWhole; // DIO32IN Mode1 Register as a Whole Word
  struct { // DIO32IN Mode1 Register as Bit Pattern
   ULONG ChanSel	: 2,	// Channel select
		 Packing	: 1,	// 1 - 8 bits mode
		 Reserved0	: 1,	// Reserved
		 ExtClkInv	: 1,	// 1 - External clock inverting
		 Reserved1	: 11;	// Reserved
  } ByBits;
} DIO32IN_MODE1, *PDIO32IN_MODE1;

// DIO32IN Status Register
typedef union _DIO32IN_STATUS {
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
				FixPnpk		: 1, // 1 - зафиксировано PNPK=0
				Pnpk		: 1, // значение сигнала PNPK
				Reserved1	: 2; // Reserved
	} ByBits;
} DIO32IN_STATUS, *PDIO32IN_STATUS;

// DIO32IN FLAG_CLR register
typedef union _DIO32IN_FLAGCLR {
  ULONG AsWhole; // DIO32IN FLAG_CLR Register as a Whole Word
  struct { // DIO32IN FLAG_CLR Register as Bit Pattern
   ULONG Reserved0	: 12, // Reserved
		 ClrFPNPK	: 1,  // read strobe length
		 Reserved1	: 3;  // reserved
  } ByBits;
} DIO32IN_FLAGCLR, *PDIO32IN_FLAGCLR;

// Numbers of Constant Registers
typedef enum _DIO32IN_NUM_DIRECT_REGS {
	DIO32INnr_FLAGCLR	= 0x200,
} DIO32IN_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_DIO32INREGS_H_
