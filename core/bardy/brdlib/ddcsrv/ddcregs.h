/*
 ****************** File DdcRegs.h *************************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for DDC
 *
 * (C) InSys by Dorokhin Andrey Jan, 2006
 *
 * 12.10.2006 - submodule ver 3.0 - synchronous mode
 *
 ************************************************************
*/

#ifndef _DDCREGS_H_
 #define _DDCREGS_H_

#pragma pack(push,1)

// Mode1 Register (MODE1 +9)
typedef union _DDC_MODE1 {
	ULONG AsWhole; // Mode1 Register as a Whole Word
	struct { // Mode1 Register as Bit Pattern
		ULONG	Sleep	: 1, // 1 - sleep mode of submodule
				Res		: 15; // Reserved
	} ByBits;
} DDC_MODE1, *PDDC_MODE1;

// Title Mode Register (TLMODE +15)
typedef union _DDC_TLMODE {
	ULONG AsWhole; // Title Mode Register as a Whole Word
	struct { // Title Mode Register as Bit Pattern
		ULONG	Size		: 5, // number of 64-bit word into title - 1
				Res			: 1, // Reserved
				TitleOn		: 1, // 1 - enabled title mode
				FrameNumAdr	: 4, // frame number address
				TimeCntAdr	: 4; // time counter address
	} ByBits;
} DDC_TLMODE, *PDDC_TLMODE;

// Gain register (+21)
typedef union _DDC_GAIN {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Chan0	: 1,	// gain of ADC 0 (0 - largest range voltage)
		 Chan1	: 1,	// gain of ADC 1 (0 - largest range voltage)
		 Chan2	: 1,	// gain of ADC 2 (0 - largest range voltage)
		 Chan3	: 1;	// gain of ADC 3 (0 - largest range voltage)
  } ByBits;
} DDC_GAIN, *PDDC_GAIN;

// ADC Control register (+22)
typedef union _DDC_ADCCTRL {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Rand	: 1,	// 1 - Digital Output Randomization
		 Dither	: 1,	// 1 - inside DITHER
		 Res	: 14;	// reserved
  } ByBits;
} DDC_ADCCTRL, *PDDC_ADCCTRL;

// Write data register (+23)
typedef union _DDC_WRDATA {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Data	: 8,	// data for writing into DDC
		 Cmd	: 1,	// 1 - read command, 0 - write command
		 Res	: 7;	// Reserved
  } ByBits;
} DDC_WRDATA, *PDDC_WRDATA;

// Address register (+24)
typedef union _DDC_ADR {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Mask	: 8,	// mask selected DDC for programming
		 RegAdr	: 8;	// address of internal registers DDC
  } ByBits;
} DDC_ADR, *PDDC_ADR;

// ADC enable register (+25)
typedef union _DDC_ADCEN {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Enbl	: 4,	// mask enabled ADC for power
		 Res	: 12;	// Reserved
  } ByBits;
} DDC_ADCEN, *PDDC_ADCEN;

// Mode2 Register (MODE2 +26)
typedef union _DDC_MODE2 {
	ULONG AsWhole; // Mode1 Register as a Whole Word
	struct { // Mode1 Register as Bit Pattern
		ULONG	SyncMode : 2,	// (+0) sync (signal SIA) mode
				Prog	: 1,	// (+2) 1 - run program sync
				Res0	: 1,	// (+3) Reserved
				Async	: 1,	// (+4) 1 - async mode
				Res1	: 1,	// (+5) Reserved
				AdcFmt	: 1,	// (+6) ADC format: 0 - 14 bit, 1 - 16 bit
				Res		: 9;	// (+7) Reserved
	} ByBits;
} DDC_MODE2, *PDDC_MODE2;

// ADM const register (+201)
typedef union _DDC_CONST {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Build		: 4,	// build number of project
	     VerLow		: 4,	// version number (low part)
	     VerHigh	: 4,	// version number (high part)
		 Mod		: 4;	// modification
  } ByBits;
} DDC_CONST, *PDDC_CONST;

// Read data register (+202)
typedef union _DDC_RDDATA {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Data	: 8,	// reading data
		 Res	: 8;	// not use
  } ByBits;
} DDC_RDDATA, *PDDC_RDDATA;

// Numbers of Command Registers
typedef enum _DDC_NUM_CMD_REGS {
	DDCnr_TLMODE	= 15, // 0x0F
	DDCnr_GAIN		= 21, // 0x15
	DDCnr_ADCCTRL	= 22, // 0x16
	DDCnr_WRDATA	= 23, // 0x17
	DDCnr_ADR		= 24, // 0x18
	DDCnr_ADCEN		= 25, // 0x19
	DDCnr_MODE2		= 26, // 0x1A
	DDCnr_FRLEN		= 31, // 0x1A
} DDC_NUM_CMD_REGS;

// Numbers of Direct Registers
typedef enum _DDC_NUM_DIRECT_REGS {
	DDCnr_ADMCONST		= 0x201,	// 
	DDCnr_RDDATA		= 0x202,	// read data from DDC
	DDCnr_PIO0			= 0x204,	// register 0 from PIOX
	DDCnr_PIO1			= 0x205,	// register 1 from PIOX
	DDCnr_PIO2			= 0x206,	// register 2 from PIOX
	DDCnr_PIO3			= 0x207,	// register 3 from PIOX
	DDCnr_TLDATAL		= 0x20C,	// title data (low part)
	DDCnr_TLDATAH		= 0x20D,	// title data (high part)
} DDC_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_DDCREGS_H_

//
// End of file
//