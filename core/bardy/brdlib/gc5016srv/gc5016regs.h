/*
 ****************** File Gc5016Regs.h *******************
 *
 *  Definitions of tetrad register
 *	structures and constants
 *	for GC5016
 *
 * (C) InSys by Sklyarov A. Nov. 2006
 *
 *************************************************************
*/

#ifndef _GC5016REGS_H_
 #define _GC5016REGS_H_

#pragma pack(push,1)

// QM Control1 register (+0)
typedef union _QM_MODE0 {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG Reset		: 1,	// reset
		 FifoRst	: 1,	// reset fifo
		 IrqEnb		: 1,	// enable IRQ
		 DrqEnb		: 1,	// enable DMA req
		 Master		: 1,	// master/slave
		 Start		: 1,	// start
		 Res		: 10;	// not use
  } ByBits;
} QM_MODE0, *PQM_MODE0;


// DDCSYNC Register
typedef union _GC_DDCSYNC {
	ULONG AsWhole; //  Register as a Whole Word
	struct { // GC_DDCSYNC Register as Bit Pattern
		ULONG	SyncMode: 2,   
				ProgSync: 1,
				Slave:	  1,
				ResDiv:	  1;
	} ByBits;
} GC_DDCSYNC, *PGC_DDCSYNC;
// DDCSYNC Register
typedef union _GC_AGCGAIN {
	ULONG AsWhole; //  Register as a Whole Word
	struct { // GC_ADCGAIN Register as Bit Pattern
		ULONG	Gain0: 1,   
				Gain1: 1,
				Gain2: 1,
				Gain3: 1;
	} ByBits;
} GC_ADCGAIN, *PGC_ADCGAIN;


// Numbers of Command Registers
typedef enum _GC_NUM_CMD_REGS {
	GC5016nr_MODE0			= 0,
	GC5016nr_SUMCNT			= 17,
	GC5016nr_SUMSHIFT		= 18,
	GC5016nr_ADCGAIN		= 21,
	GC5016nr_DDCSYNC		= 22,
	GC5016nr_WRITEDATA		= 23,
	GC5016nr_ADDR			= 24,
	GC5016nr_ADCENABLE		= 25,
	GC5016nr_READCOMMAND	= 27,
	GC5016nr_ADMMODE		= 28,
	GC5016nr_DDCMODE		= 30,
} GC5016_READ_WRITE_REGS;

// Numbers of Direct Registers
typedef enum _GC_NUM_DIRECT_REGS {
	GC5016nr_ADMCONST	= 0x201,	// 
	GC5016nr_READDATA	= 0x202,
	GC5016nr_ADCOVER	= 0x203,
	GC5016nr_TLDATAL	= 0x20c,
	GC5016nr_TLDATAH	= 0x20d,
	GC5016nr_RDATAFLT	= 0x204,
	GC5016nr_IDATAFLT	= 0x205,
	GC5016nr_CONTFLT	= 0x206,
	GC5016nr_RWINDFFT	= 0x207,
	GC5016nr_IWINDFFT	= 0x208,
	GC5016nr_CONTFFT	= 0x209,
	GC5016nr_SKIPSAMPLES = 0x20e,
	GC5016nr_GETSAMPLES	= 0x20f,
	GC5016nr_HEADERENABLE= 0x210,
	GC5016nr_HEADERFRAME= 0x211,
	GC5016nr_DELAYSTART= 0x212,
	GC5016nr_NUMCHANS= 0x213,
} GC5016_REGS;

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

typedef union _CONTFLT {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG EnableFlt	: 1,	// 0 - disable flt, 1 - enable flt 
	     RstFlt		: 1,	// Reset of write flt: 1 - reset
		 Res1		: 3,	// Reserve
		 SizeFlt	: 10,	// Size of Flt
		 Res2		: 1;	// Reserve
  } ByBits;
} CONTFLT, *PCONTFLT;

typedef union _CONTFFT {
  ULONG AsWhole;	// Register as a Whole Word
  struct {			// Register as Bit Pattern
   ULONG EnableCor	: 1,	// 0 - disable correction, 1 - enable correction 
	     RstCor		: 1,	// Reset of write correct coeff: 1 - reset
	     EnableFft	: 1,	// 1 - enable fft
	     EnableOneChan	: 1,// 1 - 0 channel to all channel
		 Res2		: 12;	// Reserve
  } ByBits;
} CONTFFT, *PCONTFFT;

#pragma pack(pop)

#endif //__GC5016REGS_H_

//
// End of file
//