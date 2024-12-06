///////////////////////////////////////////////////////////////////////
//	MAINREGS.H - Main tetrad definitions
//	Copyright (c) 2004-2010, Instrumental Systems,Corp.
///////////////////////////////////////////////////////////////////////

#ifndef _MAINREGS_H_
#define _MAINREGS_H_

#pragma pack(push,1)

// Main ThDacAddrReg register (+10)
typedef union _MAIN_THDACADR {
  ULONG AsWhole; // Main ThDacAddrReg Register as a Whole Word
  struct { // Main ThDacAddrReg Register as Bit Pattern
   ULONG Addr	: 4, // threshold DAC number
		 Res	: 3, // reserved
		 AllAdr	: 1, // 1 - write to all DACs (Addr = 0)
		 RegNum	: 3, // internal register number of threshold DAC (0 - DAC output)
		 Res1	: 5; // reserved
  } ByBits;
} MAIN_THDACADR, *PMAIN_THDACADR;

// Main ThDac register (+14)
typedef union _MAIN_THDAC {
  ULONG AsWhole; // Main ThDac Register as a Whole Word
  struct { // Main ThDac Register as Bit Pattern
   ULONG Data	: 8, // data
		 Num	: 4, // threshold DAC number
		 Res	: 4; // reserved
  } ByBits;
} MAIN_THDAC, *PMAIN_THDAC;

// Main MUX EXT register (+15)
typedef union _MAIN_MUX {
  ULONG AsWhole; // Main MUX Register as a Whole Word
  struct { // Main MUX Register as Bit Pattern
   ULONG Compar0	: 2, // comparator0
		 Res		: 2, // reserved
		 Compar1	: 2, // comparator1
		 Res1		: 2, // reserved
		 Mode		: 1, // 0 - legacy mode
		 CmpEn		: 1, // 1 - enable comparator of external clock input CLKIN (tetrada MAIN v17 - FMC105P, FMC106P)
		 GenEn		: 1, // 1 - enable internal generator (tetrada MAIN v17 - FMC105P, FMC106P)
		 Res2		: 5; // reserved
  } ByBits;
} MAIN_MUX, *PMAIN_MUX;

// Resources of PLD (0x10D)
typedef union _MAIN_MRES {
	USHORT AsWhole; // Resources of PLD Register as a Whole Word
	struct { // Register as Bit Pattern
		USHORT	ThdacType	: 2, // 0 - 8-bit threshold DAC, 1 - 12/14/16-bit threshold DAC
				MuxType		: 2, // 1 - EXTENDED mode control multiplexer of comparators is presence
				SysMonEn	: 1, // 1 - system monitor is presence
				Res0		: 2, // Reserved
				IsAcePlay	: 1, // 1 - ACE-player is presence
				Res			: 11; // Reserved
	} ByBits;
} MAIN_MRES, *PMAIN_MRES;

// Numbers of Command Registers
typedef enum _MAIN_NUM_CMD_REGS {
	MAINnr_THDADDR	= 10,
	MAINnr_THDDATA	= 11,
	MAINnr_SYNX		= 13,
	MAINnr_THDAC	= 14,
	MAINnr_CMPMUX	= 15,
} MAIN_NUM_CMD_REGS;

// Numbers of Constant Registers
typedef enum _MAIN_NUM_CONST_REGS {
	MAINnr_SIG		= 0x108,
	MAINnr_ADMVER	= 0x109,
	MAINnr_FPGAVER	= 0x10A,
	MAINnr_FPGAMODE	= 0x10B,
	MAINnr_TMASK	= 0x10C,
	MAINnr_MRES		= 0x10D,
	MAINnr_BASEID	= 0x110,
	MAINnr_BASEVER	= 0x111,
	MAINnr_SMODID	= 0x112,
	MAINnr_SMODVER	= 0x113,
	MAINnr_FPGABLD	= 0x114,
} MAIN_NUM_CONST_REGS;

// Numbers of Direct Registers
typedef enum _MAIN_NUM_DIRECT_REGS {
	MAINnr_IRQENST		= 0x200,
	MAINnr_IRQENCL		= 0x201,
	MAINnr_SYNXIN		= 0x202,
	MAINnr_JTAGINDATA0	= 0x204,
	MAINnr_JTAGOUTDATA0	= 0x205,
	MAINnr_JTAGINDATA1	= 0x206,
	MAINnr_JTAGOUTDATA1	= 0x207,
} MAIN_NUM_DIRECT_REGS;

// Numbers of Direct Registers (tetrada MAIN v17 - FMC105P, FMC106P)
typedef enum _MAIN17_NUM_DIRECT_REGS {
	MAIN17nr_IRQENST		= 0x200,
	MAIN17nr_IRQENCL		= 0x201,
	MAIN17nr_SYNXIN			= 0x202,
	MAIN17nr_SPDDEVICE		= 0x203,
	MAIN17nr_SPDCTRL		= 0x204,
	MAIN17nr_SPDADDR		= 0x205,
	MAIN17nr_SPDDATAL		= 0x206,
	MAIN17nr_SPDDATAH		= 0x207,
	MAIN17nr_SMONADDR		= 0x210,
	MAIN17nr_SMONDATA		= 0x211,
	MAIN17nr_SMONSTAT		= 0x212,
	MAIN17nr_SMONMUX		= 0x213,
} MAIN17_NUM_DIRECT_REGS;

#pragma pack(pop)

#endif //_MAINREGS_H_
