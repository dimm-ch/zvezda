//********************* File AD9956.h ******************
//	AD9956 - DDS-Based AgileRF Synthesizer chip
//	Hardware resources, constants and structures
//	Copyright (c) 2005, Instrumental Systems,Corp.
//	Written by Dorokhin Andrey
//*********************************************************

#ifndef _AD9956_H
 #define _AD9956_H

#pragma pack(1)    

typedef struct _R2BYTES {
  BYTE Byte1;  // 
  BYTE Byte0;  // 
} R2BYTES;

typedef struct _R3BYTES {
  BYTE Byte2;  // 
  BYTE Byte1;  // 
  BYTE Byte0;  // 
} R3BYTES;

typedef struct _R4BYTES {
  BYTE Byte3;  // 
  BYTE Byte2;  // 
  BYTE Byte1;  // 
  BYTE Byte0;  // 
} R4BYTES;

typedef struct _R5BYTES {
  BYTE Byte4;  // 
  BYTE Byte3;  // 
  BYTE Byte2;  // 
  BYTE Byte1;  // 
  BYTE Byte0;  // 
} R5BYTES;

typedef struct _R6BYTES {
  BYTE Byte5;  // 
  BYTE Byte4;  // 
  BYTE Byte3;  // 
  BYTE Byte2;  // 
  BYTE Byte1;  // 
  BYTE Byte0;  // 
} R6BYTES;

typedef struct _R8BYTES {
  BYTE Byte7;  // 
  BYTE Byte6;  // 
  BYTE Byte5;  // 
  BYTE Byte4;  // 
  BYTE Byte3;  // 
  BYTE Byte2;  // 
  BYTE Byte1;  // 
  BYTE Byte0;  // 
} R8BYTES;

// Control Functional Register 1 (address - 0x00, bit width - 32)
typedef union _AD9956_CFR1 {
  DWORD AsWhole; // Control Register as a Whole
  struct { // Control Register as Bit Pattern
    BYTE Byte3;  // Address 00
    BYTE Byte2;  // Address 01
    BYTE Byte1;  // Address 02
    BYTE Byte0;  // Address 03
  } ByBytes;
  struct { // Control Register as Bit Pattern
    UINT PllLkErr  : 1, // PLL Lock Error
         Open0     : 7; // must be 0
    UINT LinSweep  : 2, // Linear Sweep No Dwell & Linear Sweep Enable
         Clean     : 2, // Clean Phase Accum. & Clean Frequency Accum.
         EnSineOut : 1, // Enable Sine Output
         AutoClr   : 2, // Auto-Clr Phase Accum. & Auto-Clr Frequency Accum.
         LoadUpd   : 1; // LOAD SRR @ I/O_UPDATE
    UINT Open1     : 6, // must be 0
         InpOnly   : 1, // SDI/O Input Only 
         LsbFirst  : 1; // LSB First
    UINT HiSpdSync : 1, // High Speed Sync Enable
         HardMSync : 1, // Hardware Manual Sync
         SoftMSync : 1, // Software Manual Sync
         ASyncMlt  : 1, // Auto Sync Multiple AD9956s
         SyncDis   : 1, // SYNC_CLK Disable
         PllRefEn  : 1, // PLLREF Crystal Enable
         PFDInp_PD : 1, // PFD Input Power-Down
         Dig_PD    : 1; // Digital Power-Down
  } ByBits;
} AD9956_CFR1, *PAD9956_CFR1;

// Control Functional Register 2 (address - 0x01, bit width - 40)
typedef union _AD9956_CFR2 {
  R5BYTES AsWhole; // Control Register as a Whole
  struct { // Control Register as Byte Pattern
    BYTE Byte4;  // Address 00
    BYTE Byte3;  // Address 01
    BYTE Byte2;  // Address 02
    BYTE Byte1;  // Address 03
    BYTE Byte0;  // Address 04
  } ByBytes;
  struct { // Control Register as Bit Pattern
    UINT DRV_RSET   : 1, // Internal CML Driver DRV_RSET
		 BandGap_PD : 1, // Internal Band Gap Power-Down
         Open0      : 5, // must be 0
         DAC_PD		: 1; // DAC Power-Down
    UINT PllLkMode  : 1, // PLL Lock Detect Mode
         PllLkEn    : 1, // PLL Lock Detect Enable
         ClkFalling : 3, // Clock Driver Falling Edge Control
         ClkRising  : 3; // Clock Driver Rising Edge
    UINT RFDivMux   : 1, // RF Div REFCLK Mux Bit
         SlewRate   : 1, // Slew Rate Control
         ClkInp     : 2, // Clock Driver Input Select
         Clk_PD     : 1, // Clock Driver Power-Down
		 RFDivRatio : 2, // RF Divider Ratio
		 RFDiv_PD   : 1; // RF Divider Power-Down
    UINT DivM	    : 4, // Divider M Control
		 DivN		: 4; // Divider N Control
    UINT CPCurScale : 3, // CP Current Scale
         CPQuickPD  : 1, // CP Quick PD
         CPFullPD   : 1, // CP Full PD
         CPPolar	: 1, // CP Polarity
         Open1      : 2; // must be 0
  } ByBits;
} AD9956_CFR2, *PAD9956_CFR2;


// Chip configuration data structures
typedef struct _AD9956_CFG {
	AD9956_CFR1	CtrlCFR1;			// Control Functional Register 1 (address - 0x0, bit width - 32)
	AD9956_CFR2	CtrlCFR2;			// Control Functional Register 2 (address - 0x1, bit width - 40)
	R8BYTES		CtrlPCR0;			// Profile Control Register 0 (address - 0x06, bit width - 64)
} AD9956_CFG, *PAD9956_CFG;

#pragma pack()    

enum AD9956ErrorCodes {
  AD9956_errOK,
  AD9956_errHIGHRATE = 0xE0001000UL,
  AD9956_errLOWRATE,
  AD9956_errBADPARAM,
  AD9956_errILLEGAL
};

void AD9956_setDefaults(PAD9956_CFG cfg);

UINT AD9956_getPhase(PAD9956_CFG cfg);
int AD9956_setPhase(PAD9956_CFG cfg, UINT mask);
double AD9956_getFreq(PAD9956_CFG cfg, double SysClk);
int AD9956_setFreq(PAD9956_CFG cfg, double& fc, double SysClk);
double AD9956_getFreqFbl(PAD9956_CFG cfg, double SysClk);
int AD9956_setFreqFbl(PAD9956_CFG cfg, double& fc, double SysClk);

#endif // _AD9956_H

// ****************** End of file AD9956.h *****************
