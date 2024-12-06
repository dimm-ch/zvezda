/*
 ****************** File DdsRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for DDS
 *
 * (C) InSys by Sklyarov A. Mar, 2007
 *
 ************************************************************
*/

#ifndef _DDSREGS_H_
#define _DDSREGS_H_

#pragma pack(push,1)

// DDS Control1 register
typedef union _DDS_CONTROL1 {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Div0		: 3,	// Set value Divider 0
		 ResDiv0	: 1,	// 1-reset Divider 0
		 Div1		: 3,	// Set value Divider 1
		 ResDiv1	: 1,	// 1-reset Divider 1
		 ChipReset	: 1,	// 1 - reset tetrade and DDS chip
		 UpdateDds	: 1,	// Update after prog DDs chip
		 RefClkSrc	: 1,	// Source ref clk: 0-external, 1-internal
		 ClkSel		: 1,	// Mode:0-OutFreq<=100 MHz,1-OutFreq>100 MHz
		 					// V2: clock for start: FPGA_CLK0, FPGA_CLK1
		 MuxOut		: 2,	// V2: multiplexer of DDS output
		 Res0		: 1,	// V2: reserved
		 DdsEn		: 1;	// V2: DDS enable
  } ByBits;
} DDS_CONTROL1, *PDDS_CONTROL1;

// Dds  regs
typedef enum _DDS_NUM_CMD_REGS {
	DDSnr_CONTROL1		= 23, // 0x17
} DDS_NUM_CMD_REGS;

// Dds read/write regs
typedef enum _DDS_NUM_DIRECT_REGS {
	DDSnr_INST		= 0x201,
	DDSnr_WRITEDATA	= 0x202,
	DDSnr_READDATA	= 0x203,
	DDSnr_DELAY		= 0x240,
	DDSnr_PULSE		= 0x241,
	DDSnr_PERIOD_L	= 0x242,
	DDSnr_PERIOD_H	= 0x243,
	DDSnr_QUANTITY	= 0x244,
} DDS_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_DDSREGS_H_

//
// End of file
//
