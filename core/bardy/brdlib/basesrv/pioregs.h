/*
 ****************** File PioRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for PIO
 *
 * (C) InSys by Dorokhin Andrey Feb, 2006
 *
 ************************************************************
*/

#ifndef _PIOREGS_H_
#define _PIOREGS_H_

#pragma pack(push,1)

// PIO Mode1 register (+9)
typedef union _PIO_MODE1 {
  ULONG AsWhole; // PIO Mode Register as a Whole Word
  struct { // PIO Register as Bit Pattern
   ULONG LBDir		: 1, // low byte data direction
		 HBDir		: 1, // high byte data direction
		 RdMode		: 4, // read mode
		 WrMode		: 4, // write mode (not used)
		 Lvds		: 1, // 1 - LVDS mode, 0 - LVTTL mode
		 Enbl		: 1, // 1 - enable PIO, 0 - disable PIO
		 PioCtrl	: 1, // control of signal PIO_WR, PIO_RD
		 AckCtrl	: 1, // control of signal ACK_WR, ACK_RD
		 Res		: 2; // reserved
  } ByBits;
} PIO_MODE1, *PPIO_MODE1;

// PIO Mode2 register (+10)
typedef union _PIO_MODE2 {
  ULONG AsWhole; // PIO Mode Register as a Whole Word
  struct { // PIO Register as Bit Pattern
   ULONG CntWr		: 8, // write strobe length
		 CntRd		: 8; // read strobe length
  } ByBits;
} PIO_MODE2, *PPIO_MODE2;

// PIO Status Register
typedef union _PIO_STATUS {
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
				AckWr		: 1, // ACK_WR signal value
				AckRd		: 1, // ACK_RD signal value
				FAckWr		: 1, // 1 - ACK_WR signal front
				FAckRd		: 1, // 1 - ACK_RD signal front
				Reserved	: 3; // Reserved
	} ByBits;
} PIO_STATUS, *PPIO_STATUS;

// Numbers of Constant Registers
typedef enum _PIO_NUM_DIRECT_REGS {
	PIO_nrFLAGCLR	= 0x200,
	PIO_nrSTARTRD	= 0x201,
} PIO_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_PIOREGS_H_
