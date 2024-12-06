/*
 ****************** File AdcRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for ADC
 *
 * (C) InSys by Dorokhin Andrey Apr, 2004
 *
 ************************************************************
*/

#ifndef _ADCREGS_H_
 #define _ADCREGS_H_

#pragma pack(push,1)

// ADC Input register (+22)
typedef union _ADC_INP {
  U32 AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG InpR0		: 1,	// input resistance of channel 0 (0 - 1 MOm, 1 - 50 Om)
		 InpType0	: 1,	// input type of channel 0 (1 - DC coupling, open input)
		 LpfOn0		: 1,	// turn LPF of channel 0 (0 - off, 1 - on)
		 Res		: 1;	// not use
   ULONG InpR1		: 1,	// input resistance of channel 1 (0 - 1 MOm, 1 - 50 Om)
		 InpType1	: 1,	// input type of channel 1 (1 - DC coupling, open input)
		 LpfOn1		: 1,	// turn LPF of channel 1 (0 - off, 1 - on)
		 Res1		: 1;	// not use
   ULONG InpR2		: 1,	// input resistance of channel 2 (0 - 1 MOm, 1 - 50 Om)
		 InpType2	: 1,	// input type of channel 2 (1 - DC coupling, open input)
		 LpfOn2		: 1,	// turn LPF of channel 2 (0 - off, 1 - on)
		 Res2		: 1;	// not use
   ULONG InpR3		: 1,	// input resistance of channel 3 (0 - 1 MOm, 1 - 50 Om)
		 InpType3	: 1,	// input type of channel 3 (1 - DC coupling, open input)
		 LpfOn3		: 1,	// turn LPF of channel 3 (0 - off, 1 - on)
		 Res3		: 1;	// not use
  } ByBits;
} ADC_INP, *PADC_INP;

// ADC Status Register
typedef union _ADC_STATUS {
	U32 AsWhole; // Status Register as a Whole Word
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
				ErrorAccess	: 1, // register access error
				Start		: 1, // work enabled
				OverAdc		: 1, // Bits Overflow of one or more ADC
				Reserved0	: 2, // Reserved
				OverAdc0	: 1, // Bits Overflow of ADC0
				OverAdc1	: 1; // Bits Overflow of ADC1
	} ByBits;
} ADC_STATUS, *PADC_STATUS;

// ADC FLAG_CLR register
typedef union _ADC_FLAGCLR {
  U32 AsWhole; // Register as a Whole Word
  struct { // Register as Bit Pattern
   ULONG Reserved0	: 11, // Reserved
		 ClrOvrAdc	: 1,  // 1 - clear flag overflow all ADC
		 Reserved1	: 2,  // Reserved
		 ClrOvrAdc0	: 1,  // 1 - clear flag overflow ADC0
		 ClrOvrAdc1	: 1;  // 1 - clear flag overflow ADC1
  } ByBits;
} ADC_FLAGCLR, *PADC_FLAGCLR;

// ADC Delay register (+0x2F0)
//typedef union _ADC_DELAY {
//  U32 AsWhole;	// Register as a Whole Word
//  struct {			// Register as Bit Pattern
//   ULONG Delay 		: 6,	// delay value
//		 Reserved0	: 1,	// Reserved
//		 Used		: 1,	// read 1 - used specify ID
//		 ID			: 4,	// ID: 0 - ADC input 0, ..., 8 - external start, ..., 12 - SYNX inp0, ...
//		 Reserved1	: 2,	// Reserved
//		 Write		: 1,	// 1 - command write
//		 Reset		: 1;	// 1 - command reset
//  } ByBits;
//} ADC_DELAY, *PADC_DELAY;

// Numbers of Command Registers
typedef enum _ADC_NUM_CMD_REGS {
	ADCnr_TESTSEQ	= 12, // 0x0C
	ADCnr_INP		= 22, // 0x16
	ADCnr_CTRL1		= 23, // 0x17
	ADCnr_LOWPLL	= 24, // 0x18
	ADCnr_HIPLL		= 25, // 0x19
} ADC_NUM_CMD_REGS;

// Numbers of Direct Registers
typedef enum _ADC_NUM_DIRECT_REGS {
	ADCnr_FLAGCLR		= 0x200,
	ADCnr_ADJUST		= 0x207,	// 
	ADCnr_OVERFLOW		= 0x208,	// RD - ADC bits overflow status flags; WR - clear ADC bits overflow status flags
	ADCnr_PREVPREC		= 0x209,	// start event by pretrigger mode: sample number into word
	ADCnr_PRTEVENTLO	= 0x20A,	// start event by pretrigger mode: word number into buffer (low part)
	ADCnr_PRTEVENTHI	= 0x20B,	// start event by pretrigger mode: word number into buffer (high part)
	ADCnr_TLDATAL		= 0x20C,	// title data (low part)
	ADCnr_TLDATAH		= 0x20D,	// title data (high part)
	ADCnr_EVENTCNTL		= 0x20E,	// current event counter (low part)
	ADCnr_EVENTCNTH		= 0x20F,	// current event counter (high part)
	ADCnr_BLOCKCNTL		= 0x210,	// current block counter (low part)
	ADCnr_BLOCKCNTH		= 0x211,	// current block counter (high part)
//	ADCnr_STDDELAY		= 0x2F0,	// write/read/reset delay for ADC input / external start / SYNX
} ADC_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_ADCREGS_H_

//
// End of file
//