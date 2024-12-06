///////////////////////////////////////////////////////////////////////
//	ADM2IF.H - ADM2-interface definitions
//	Copyright (c) 2004, Instrumental Systems,Corp.
///////////////////////////////////////////////////////////////////////

#ifndef _ADM2IF_H_
#define _ADM2IF_H_

#include <chrono>
#include <thread>

#include "utypes.h"

#define ADM_VERSION 0x200  // ADM version

#pragma pack(push, 1)

// Command Registers Layout
typedef volatile struct _TETR_CMD_REGS {
  ULONG MODE0;    // (0x00) Control register
  ULONG IRQMASK;  // (0x04) Interrupt mask register
  ULONG IRQINV;   // (0x08) Interrupt inversion register
  ULONG FMODE;    // (0x0C) Clock source register
  ULONG FDIV;     // (0x10) Clock divider register
  ULONG STMODE;   // (0x14) Start mode register
  ULONG CNT0;     // (0x18) Delay counter register
  ULONG CNT1;     // (0x1C) Take counter register
  ULONG CNT2;     // (0x20) Skip counter register
  ULONG MODE1;    // (0x24) Control register
  ULONG MODE2;    // (0x28) Control register
  ULONG MODE3;    // (0x2C) Control register
  ULONG Res[18];  // (0x30) Reserved space
} TETR_CMD_REGS, *PTETR_CMD_REGS;

// Constant Registers Layout
typedef volatile struct _TETR_CONST_REGS {
  ULONG ID;       // (0x00) Control register
  ULONG IDMOD;    // (0x04) Interrupt mask register
  ULONG VER;      // (0x08) Interrupt inversion register
  ULONG TRES;     // (0x0C) Clock source register
  ULONG FSIZE;    // (0x10) Clock divider register
  ULONG FTYPE;    // (0x14) Start mode register
  ULONG PATH;     // (0x18) Delay counter register
  ULONG IDNUM;    // (0x1C) Take counter register
  ULONG Res[24];  // (0x30) Reserved space
} TETR_CONST_REGS, *PTETR_CONST_REGS;

// Board Mode Register (MODE0 +0)
typedef union _ADM2IF_MODE0 {
  U32 AsWhole;        // Board Mode Register as a Whole Word
  struct {            // Mode Register as Bit Pattern
    ULONG Reset : 1,  // PLD or tetrad reset
        FifoRes : 1,  // FIFO Reset
        IrqEnbl : 1,  // Interrupt request from tetrad enable
        DrqEnbl : 1,  // DMA request from tetrad enable
        Master : 1,   // Master(Single)/Slave mode
        Start : 1,    // Program start
        AdmClk : 1,   // Clock source on submodule
        Cycle : 1,    // Cycling enable
        Cnt0En : 1,   // Counter0 enable
        Cnt1En : 1,   // Counter1 enable
        Cnt2En : 1,   // Counter2 enable
        Res1 : 1,     // Reserved
        DrqSrc : 2,   // DMA request sources
        Res2 : 1,     // Reserved
        extFifo : 1;  // External FIFO used flag
  } ByBits;
} ADM2IF_MODE0, *PADM2IF_MODE0;

// Start Mode Register (STMODE +5)
typedef union _ADM2IF_STMODE {
  U32 AsWhole;           // Start Mode Register as a Whole Word
  struct {               // Start Mode Register as Bit Pattern
    ULONG SrcStart : 5,  // select of start signal
        Res1 : 1,        // Reserved
        InvStart : 1,    // 1 - inverting start signal
        TrigStart : 1,   // 1 - trigger start mode
        SrcStop : 5,     // select of stop signal
        Res2 : 1,        // Reserved
        InvStop : 1,     // 1 - inverting stop signal
        Restart : 1;     // 1 - Restart mode
  } ByBits;
} ADM2IF_STMODE, *PADM2IF_STMODE;

// Mode1 Register (MODE1 +9)
typedef union _ADM2IF_MODE1 {
  U32 AsWhole;       // Mode1 Register as a Whole Word
  struct {           // Mode1 Register as Bit Pattern
    ULONG Out : 4,   // output data flow
        MsSync : 1,  // sync pulse for MASTER
        Res : 2,     // Reserved
        Test : 1,    // 1 - enabled test mode
        InHalf : 1,  // разделение потоков с АЦП пополам
        Res0 : 7;  // Reserved
  } ByBits;
} ADM2IF_MODE1, *PADM2IF_MODE1;

// Pretrigger Mode Register (PRTMODE +14)
typedef union _ADM2IF_PRTMODE {
  U32 AsWhole;         // Pretrigger Mode Register as a Whole Word
  struct {             // Pretrigger Mode Register as Bit Pattern
    ULONG Enable : 1,  // 1 - enabled pretrigger mode
        External : 1,  // 1 - enabled external pretrigger mode
        Assur : 1,     // 1 - enabled garanting pretrigger mode
        Res : 1;       // Reserved
  } ByBits;
} ADM2IF_PRTMODE, *PADM2IF_PRTMODE;

// Title Mode Register (TLMODE +15)
typedef union _ADM2IF_TLMODE {
  U32 AsWhole;        // Title Mode Register as a Whole Word
  struct {            // Title Mode Register as Bit Pattern
    ULONG Size : 7,   // title size
        TitleOn : 1,  // 1 - enabled title mode
        Res : 8;      // Reserved
  } ByBits;
} ADM2IF_TLMODE, *PADM2IF_TLMODE;

// Chan1 register (+16)
typedef union _ADM2IF_CHAN1 {
  U32 AsWhole;           // Register as a Whole Word
  struct {               // Register as Bit Pattern
    ULONG ChanSel : 16;  // Channel select
  } ByBits;
} ADM2IF_CHAN1, *PADM2IF_CHAN1;

// Chan2 register (+17)
typedef union _ADM2IF_CHAN2 {
  U32 AsWhole;           // Register as a Whole Word
  struct {               // Register as Bit Pattern
    ULONG ChanSel : 16;  // Channel select
  } ByBits;
} ADM2IF_CHAN2, *PADM2IF_CHAN2;

// Format register (+18)
typedef union _ADM2IF_FORMAT {
  U32 AsWhole;  // Register as a Whole Word
  struct {      // Register as Bit Pattern
    ULONG
    Pack : 8,       // 0 - 16 bit, 1 - 8 bit, 2 - 16 bit, 3 - 24 bit, 4 - 32 bit
        Code : 4,   // 0 - two's complement, 1 - floating point, 2 - straight
                    // binary, 7 - Gray code
        Align : 1,  // 0 - align to high-order bit, 1 - align to low-order bit
        Res : 3;    // reserved
  } ByBits;
} ADM2IF_FORMAT, *PADM2IF_FORMAT;

// Frequency Source register (+19)
typedef union _ADM2IF_FSRC {
  U32 AsWhole;      // Register as a Whole Word
  struct {          // Register as Bit Pattern
    ULONG Gen : 8,  // source clock: 0 - disabled, 1 - Gen1, 0x81 - external
        Inv : 1,    // inverting clock
        Res : 7;    // not use
  } ByBits;
} ADM2IF_FSRC, *PADM2IF_FSRC;

// Gain register (+21)
typedef union _ADM2IF_GAIN {
  U32 AsWhole;        // Register as a Whole Word
  struct {            // Register as Bit Pattern
    ULONG Chan0 : 4,  // gain of channel 0 (0 - largest range voltage)
        Chan1 : 4,    // gain of channel 1 (0 - largest range voltage)
        Chan2 : 4,    // gain of channel 2 (0 - largest range voltage)
        Chan3 : 4;    // gain of channel 3 (0 - largest range voltage)
  } ByBits;
} ADM2IF_GAIN, *PADM2IF_GAIN;

// Board Status Direct Register
typedef union _ADM2IF_STATUS {
  U32 AsWhole;            // Board Status Register as a Whole Word
  struct {                // Status Register as Bit Pattern
    ULONG CmdRdy : 1,     // Ready to do command
        FifoRdy : 1,      // Ready FIFO
        Empty : 1,        // Empty FIFO
        AlmostEmpty : 1,  // Almost Empty FIFO
        HalfFull : 1,     // Half Full FIFO
        AlmostFull : 1,   // Almost Full FIFO
        Full : 1,         // Full FIFO
        Overflow : 1,     // Overflow FIFO
        Underflow : 1,    // Underflow FIFO
        Reserved : 7;     // Reserved
  } ByBits;
} ADM2IF_STATUS, *PADM2IF_STATUS;

// Resources structure
typedef union _TETRAD_IDRES {
  USHORT AsWhole;         // Board Status Register as a Whole Word
  struct {                // Status Register as Bit Pattern
    USHORT Res0 : 4,      // Reserved
        InFifo : 1,       // 1 - input FIFO is presence
        OutFifo : 1,      // 1 - output FIFO is presence
        LvlFlagFifo : 1,  // 1 - можно устанавливать уровень срабатывания флагов
                          // FIFO
        Res1 : 9;         // Reserved
  } ByBits;
} TETRAD_IDRES, *PTETRAD_IDRES;

// Tetrad Identification structure
typedef volatile struct _TETRAD_ID {
  USHORT Id;         // Identificator
  USHORT Mod;        // Modificator
  USHORT Ver;        // Version
  TETRAD_IDRES Res;  // Resources
  USHORT FifoSize;   // FIFO size
  UCHAR FifoType;    // FIFO type
  USHORT Path;       //
  USHORT Inst;       // Instance number
} TETRAD_ID, *PTETRAD_ID;

// Delay register (+0x2F0)
typedef union _STD_DELAY {
  U32 AsWhole;          // Register as a Whole Word
  struct {              // Register as Bit Pattern
    ULONG Delay : 6,    // delay value
        Reserved0 : 1,  // Reserved
        Used : 1,       // read 1 - used specify ID
        ID : 4,  // ID: 0 - ADC input 0, ..., 8 - external start, ..., 12 - SYNX
                 // inp0, ...
        ExtFlag : 1,  // 1 - writing/reading to high bits (6...11)
        ExtReg : 1,   // 1 - Extension delay register (12 bits)
                      // Reserved1	: 2,	// Reserved
        Write : 1,    // 1 - command write
        Reset : 1;    // 1 - command reset
  } ByBits;
} STD_DELAY, *PSTD_DELAY;

#pragma pack(pop)

#define MAX_TETRNUM 16  // max number of tetrades of the ADM-interface

// Numbers of Tetrads
typedef enum _ADM2IF_NUM_TETRAD {
  ADM2IFnt_MAIN = 0,
  ADM2IFnt_BASEDAC = 1,
  ADM2IFnt_PIOSTD = 3,
  ADM2IFnt_ADC = 4,
} ADM2IF_NUM_TETRAD;

// Numbers of Tetrad Registers
typedef enum _ADM2IF_NUM_TETR_REGS {
  ADM2IFnr_STATUS = 0,   // (0x00) Status register
  ADM2IFnr_DATA = 1,     // (0x02) Data register
  ADM2IFnr_CMDADR = 2,   // (0x04) Address command register
  ADM2IFnr_CMDDATA = 3,  // (0x06) Data command register
} ADM2IF_NUM_TETR_REGS;

// Numbers of Command Registers
typedef enum _ADM2IF_NUM_CMD_REGS {
  ADM2IFnr_MODE0 = 0,
  ADM2IFnr_IRQMASK = 1,
  ADM2IFnr_IRQINV = 2,
  ADM2IFnr_FMODE = 3,
  ADM2IFnr_FDIV = 4,
  ADM2IFnr_STMODE = 5,
  ADM2IFnr_CNT0 = 6,
  ADM2IFnr_CNT1 = 7,
  ADM2IFnr_CNT2 = 8,
  ADM2IFnr_MODE1 = 9,
  ADM2IFnr_MODE2 = 10,    // 0x0A
  ADM2IFnr_MODE3 = 11,    // 0x0B
  ADM2IFnr_CNTAE = 12,    // 0x0C
  ADM2IFnr_CNTAF = 13,    // 0x0D
  ADM2IFnr_PRTMODE = 14,  // 0x0E
  ADM2IFnr_TLMODE = 15,   // 0x0F
  ADM2IFnr_CHAN1 = 16,    // 0x10
  ADM2IFnr_CHAN2 = 17,    // 0x11
  ADM2IFnr_FORMAT = 18,   // 0x12
  ADM2IFnr_FSRC = 19,     // 0x13
  ADM2IFnr_FDVR = 20,     // 0x14
  ADM2IFnr_GAIN = 21,     // 0x15
  ADM2IFnr_VCODAC = 24,   // 0x18
  ADM2IFnr_ECNT0 = 28,    // 0x1C
  ADM2IFnr_ECNT1 = 29,    // 0x1D
  ADM2IFnr_ECNT2 = 30,    // 0x1E
} ADM2IF_NUM_CMD_REGS;

// Numbers of Constant Registers
typedef enum _ADM2IF_NUM_CONST_REGS {
  ADM2IFnr_ID = 0x100,
  ADM2IFnr_IDMOD = 0x101,
  ADM2IFnr_VER = 0x102,
  ADM2IFnr_TRES = 0x103,
  ADM2IFnr_FSIZE = 0x104,
  ADM2IFnr_FTYPE = 0x105,
  ADM2IFnr_PATH = 0x106,
  ADM2IFnr_IDNUM = 0x107,
  ADM2IFnr_PFSIZE = 0x108,
} ADM2IF_NUM_CONST_REGS;

// Numbers of Direct Registers
typedef enum _ADM2IF_NUM_DIRECT_REGS {
  ADM2IFnr_FLAGCLR = 0x200,
  ADM2IFnr_SPDDEVICE = 0x203,
  ADM2IFnr_SPDCTRL = 0x204,
  ADM2IFnr_SPDADDR = 0x205,
  ADM2IFnr_SPDDATAL = 0x206,
  ADM2IFnr_SPDDATAH = 0x207,
  ADM2IFnr_STDDELAY = 0x2F0,  // write/read/reset delay for ADC/DAC channels /
                              // external start / SYNX
} ADM2IF_NUM_DIRECT_REGS;

#include "module.h"

inline ULONG RegRead(CModule* pModule, ULONG srvMainIdx, ULONG tetr, ULONG reg,
                     ULONG cmd) {
  DEVS_CMD_Reg RegParam;
  RegParam.idxMain = srvMainIdx;
  RegParam.tetr = tetr;
  RegParam.reg = reg;
  ULONG status = pModule->RegCtrl(cmd, &RegParam);
  return status;
}

inline ULONG WaitCmdReady(CModule* pModule, ULONG srvMainIdx, ULONG main_tetr,
                          long msTimeout) {
  DEVS_CMD_Reg param;
  param.idxMain = srvMainIdx;
  param.tetr = main_tetr;
  param.reg = ADM2IFnr_STATUS;
  PADM2IF_STATUS pStatus;
  if (msTimeout == -1) {
    do {
      pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
      pStatus = (PADM2IF_STATUS)&param.val;
    } while (!pStatus->ByBits.CmdRdy);
  } else {
    auto num = msTimeout * 100;
    for (int i = 0; i < num; i++) {
      pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
      pStatus = (PADM2IF_STATUS)&param.val;
      if (pStatus->ByBits.CmdRdy) return 0;  // OK
      #if defined(__IPC_WIN__)
             IPC_pause(10);
      #else
		  std::this_thread::sleep_for(std::chrono::microseconds(10));
      #endif
    }
    return 1;  // timeout event
  }
  return 0;  // OK
}

long GetTetrNum(PVOID module, ULONG srvMainIdx, ULONG tetrId);
long GetTetrNumEx(PVOID module, ULONG srvMainIdx, ULONG tetrId,
                  ULONG instantNum);

#endif  //_ADM2IF_H_
