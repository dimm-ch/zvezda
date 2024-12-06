/*
 ****************** File Dac1624x192Regs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for DAC16214X192
 *
 * (C) InSys by Dorokhin Andrey Jun, 2007
 *
 ************************************************************
*/

#ifndef _DAC16214X192REGS_H_
 #define _DAC16214X192REGS_H_

#pragma pack(push,1)

// DAC Control register (+23)
typedef union _DAC_CTRL {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Lrck		: 2,	// LRCK mode: 0 - SINGLE (CLK/512), 1 - DOUBLE (CLK/256), 2 - QUAD (CLK/128)
		 Res		: 2,	// not use
		 Sync		: 1,	// 0->1 - synchro pulse
		 Mute		: 1,	// MUTE
		 Dem		: 1,	// De-Emphasis filter
		 Res1		: 9;	// not use
  } ByBits;
} DAC_CTRL, *PDAC_CTRL;

#pragma pack(pop)

#endif //_DAC16214X192REGS_H_

//
// End of file
//