/*
 ****************** File DacRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for DAC
 *
 * (C) InSys by Dorokhin Andrey Oct, 2005
 *
 ************************************************************
*/

#ifndef _DACREGS_H_
 #define _DACREGS_H_

#pragma pack(push,1)

// Numbers of Command Registers
typedef enum _DAC_NUM_CMD_REGS {
	DACnr_INP		= 22, // 0x16
	DACnr_CTRL1		= 23, // 0x17
	DACnr_LOWPLL	= 24, // 0x18
	DACnr_HIPLL		= 25, // 0x19
	DACnr_REGADDR	= 26, // 0x1A
	DACnr_REGDATA	= 27, // 0x1B
} DAC_NUM_CMD_REGS;

// Numbers of Direct Registers
typedef enum _DAC_NUM_DIRECT_REGS {
	DACnr_FLAGCLR		= 0x200,
	DACnr_CHIPREG1		= 0x201,
	DACnr_CHIPREG2		= 0x202,
	DACnr_UNDERFLOW		= 0x208,	// RD - DAC bits underflow status flags; WR - clear DAC bits underflow status flags
} DAC_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_DACREGS_H_

//
// End of file
//