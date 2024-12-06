#pragma once

#pragma pack(push, 1)

namespace InSys
{

  constexpr uint32_t ADM_VERSION = 0x200; // ADM version

  // Command Registers Layout
  struct TETR_CMD_REGS
  {
    uint32_t MODE0;   // (0x00) Control register
    uint32_t IRQMASK; // (0x04) Interrupt mask register
    uint32_t IRQINV;  // (0x08) Interrupt inversion register
    uint32_t FMODE;   // (0x0C) Clock source register
    uint32_t FDIV;    // (0x10) Clock divider register
    uint32_t STMODE;  // (0x14) Start mode register
    uint32_t CNT0;    // (0x18) Delay counter register
    uint32_t CNT1;    // (0x1C) Take counter register
    uint32_t CNT2;    // (0x20) Skip counter register
    uint32_t MODE1;   // (0x24) Control register
    uint32_t MODE2;   // (0x28) Control register
    uint32_t MODE3;   // (0x2C) Control register
    uint32_t Res[18]; // (0x30) Reserved space
  };

  // Constant Registers Layout
  struct TETR_CONST_REGS
  {
    uint32_t ID;      // (0x00) Control register
    uint32_t IDMOD;   // (0x04) Interrupt mask register
    uint32_t VER;     // (0x08) Interrupt inversion register
    uint32_t TRES;    // (0x0C) Clock source register
    uint32_t FSIZE;   // (0x10) Clock divider register
    uint32_t FTYPE;   // (0x14) Start mode register
    uint32_t PATH;    // (0x18) Delay counter register
    uint32_t IDNUM;   // (0x1C) Take counter register
    uint32_t Res[24]; // (0x30) Reserved space
  };

  // Board Mode Register (MODE0 +0)
  union ADM2IF_MODE0
  {
    uint32_t AsWhole; // Board Mode Register as a Whole Word
    struct
    {                     // Mode Register as Bit Pattern
      uint32_t Reset : 1, // PLD or tetrad reset
          FifoRes : 1,    // FIFO Reset
          IrqEnbl : 1,    // Interrupt request from tetrad enable
          DrqEnbl : 1,    // DMA request from tetrad enable
          Master : 1,     // Master(Single)/Slave mode
          Start : 1,      // Program start
          AdmClk : 1,     // Clock source on submodule
          Cycle : 1,      // Cycling enable
          Cnt0En : 1,     // Counter0 enable
          Cnt1En : 1,     // Counter1 enable
          Cnt2En : 1,     // Counter2 enable
          Res1 : 1,       // Reserved
          DrqSrc : 2,     // DMA request sources
          Res2 : 1,       // Reserved
          extFifo : 1;    // External FIFO used flag
    } ByBits;
  };

  // Start Mode Register (STMODE +5)
  union ADM2IF_STMODE
  {
    uint32_t AsWhole; // Start Mode Register as a Whole Word
    struct
    {                        // Start Mode Register as Bit Pattern
      uint32_t SrcStart : 5, // select of start signal
          Res1 : 1,          // Reserved
          InvStart : 1,      // 1 - inverting start signal
          TrigStart : 1,     // 1 - trigger start mode
          SrcStop : 5,       // select of stop signal
          Res2 : 1,          // Reserved
          InvStop : 1,       // 1 - inverting stop signal
          Restart : 1;       // 1 - Restart mode
    } ByBits;
  };

  // Mode1 Register (MODE1 +9)
  union ADM2IF_MODE1
  {
    uint32_t AsWhole; // Mode1 Register as a Whole Word
    struct
    {                   // Mode1 Register as Bit Pattern
      uint32_t Out : 4, // output data flow
          MsSync : 1,   // sync pulse for MASTER
          Res : 2,      // Reserved
          Test : 1,     // 1 - enabled test mode
          InHalf : 1,   // разделение потоков с АЦП пополам
          Res0 : 7;     // Reserved
    } ByBits;
  };

  // Pretrigger Mode Register (PRTMODE +14)
  union ADM2IF_PRTMODE
  {
    uint32_t AsWhole; // Pretrigger Mode Register as a Whole Word
    struct
    {                      // Pretrigger Mode Register as Bit Pattern
      uint32_t Enable : 1, // 1 - enabled pretrigger mode
          External : 1,    // 1 - enabled external pretrigger mode
          Assur : 1,       // 1 - enabled garanting pretrigger mode
          Res : 1;         // Reserved
    } ByBits;
  };

  // Title Mode Register (TLMODE +15)
  union ADM2IF_TLMODE
  {
    uint32_t AsWhole; // Title Mode Register as a Whole Word
    struct
    {                    // Title Mode Register as Bit Pattern
      uint32_t Size : 7, // title size
          TitleOn : 1,   // 1 - enabled title mode
          Res : 8;       // Reserved
    } ByBits;
  };

  // Chan1 register (+16)
  union ADM2IF_CHAN1
  {
    uint32_t AsWhole; // Register as a Whole Word
    struct
    {                        // Register as Bit Pattern
      uint32_t ChanSel : 16; // Channel select
    } ByBits;
  };

  // Chan2 register (+17)
  union ADM2IF_CHAN2
  {
    uint32_t AsWhole; // Register as a Whole Word
    struct
    {                        // Register as Bit Pattern
      uint32_t ChanSel : 16; // Channel select
    } ByBits;
  };

  // Format register (+18)
  union ADM2IF_FORMAT
  {
    uint32_t AsWhole; // Register as a Whole Word
    struct
    { // Register as Bit Pattern
      uint32_t
          Pack : 8,  // 0 - 16 bit, 1 - 8 bit, 2 - 16 bit, 3 - 24 bit, 4 - 32 bit
          Code : 4,  // 0 - two's complement, 1 - floating point, 2 - straight
                     // binary, 7 - Gray code
          Align : 1, // 0 - align to high-order bit, 1 - align to low-order bit
          Res : 3;   // reserved
    } ByBits;
  };

  // Frequency Source register (+19)
  union ADM2IF_FSRC
  {
    uint32_t AsWhole; // Register as a Whole Word
    struct
    {                   // Register as Bit Pattern
      uint32_t Gen : 8, // source clock: 0 - disabled, 1 - Gen1, 0x81 - external
          Inv : 1,      // inverting clock
          Res : 7;      // not use
    } ByBits;
  };

  // Gain register (+21)
  union ADM2IF_GAIN
  {
    uint32_t AsWhole; // Register as a Whole Word
    struct
    {                     // Register as Bit Pattern
      uint32_t Chan0 : 4, // gain of channel 0 (0 - largest range voltage)
          Chan1 : 4,      // gain of channel 1 (0 - largest range voltage)
          Chan2 : 4,      // gain of channel 2 (0 - largest range voltage)
          Chan3 : 4;      // gain of channel 3 (0 - largest range voltage)
    } ByBits;
  };

  // Board Status Direct Register
  union ADM2IF_STATUS
  {
    uint32_t AsWhole; // Board Status Register as a Whole Word
    struct
    {                      // Status Register as Bit Pattern
      uint32_t CmdRdy : 1, // Ready to do command
          FifoRdy : 1,     // Ready FIFO
          Empty : 1,       // Empty FIFO
          AlmostEmpty : 1, // Almost Empty FIFO
          HalfFull : 1,    // Half Full FIFO
          AlmostFull : 1,  // Almost Full FIFO
          Full : 1,        // Full FIFO
          Overflow : 1,    // Overflow FIFO
          Underflow : 1,   // Underflow FIFO
          Reserved : 7;    // Reserved
    } ByBits;
  };

  // Resources structure
  union TETRAD_IDRES
  {
    uint16_t AsWhole; // Board Status Register as a Whole Word
    struct
    {                      // Status Register as Bit Pattern
      uint16_t Res0 : 4,   // Reserved
          InFifo : 1,      // 1 - input FIFO is presence
          OutFifo : 1,     // 1 - output FIFO is presence
          LvlFlagFifo : 1, // 1 - можно устанавливать уровень срабатывания флагов
                           // FIFO
          Res1 : 9;        // Reserved
    } ByBits;
  };

  // Tetrad Identification structure
  struct TETRAD_ID
  {
    uint16_t Id;       // Identificator
    uint16_t Mod;      // Modificator
    uint16_t Ver;      // Version
    TETRAD_IDRES Res;  // Resources
    uint16_t FifoSize; // FIFO size
    uint8_t FifoType;  // FIFO type
    uint16_t Path;     //
    uint16_t Inst;     // Instance number
  };

  // Delay register (+0x2F0)
  union STD_DELAY
  {
    uint32_t AsWhole; // Register as a Whole Word
    struct
    {                     // Register as Bit Pattern
      uint32_t Delay : 6, // delay value
          Reserved0 : 1,  // Reserved
          Used : 1,       // read 1 - used specify ID
          ID : 4,         // ID: 0 - ADC input 0, ..., 8 - external start, ..., 12 - SYNX
                          // inp0, ...
          ExtFlag : 1,    // 1 - writing/reading to high bits (6...11)
          ExtReg : 1,     // 1 - Extension delay register (12 bits)
                          // Reserved1	: 2,	// Reserved
          Write : 1,      // 1 - command write
          Reset : 1;      // 1 - command reset
    } ByBits;
  };

#pragma pack(pop)

  constexpr uint32_t MAX_TETRNUM = 16; // max number of tetrades of the ADM-interface

  // Numbers of Tetrads
  enum ADM2IF_NUM_TETRAD
  {
    ADM2IFnt_MAIN = 0,
    ADM2IFnt_BASEDAC = 1,
    ADM2IFnt_PIOSTD = 3,
    ADM2IFnt_ADC = 4,
  };

  // Numbers of Tetrad Registers
  enum ADM2IF_NUM_TETR_REGS
  {
    ADM2IFnr_STATUS = 0,  // (0x00) Status register
    ADM2IFnr_DATA = 1,    // (0x02) Data register
    ADM2IFnr_CMDADR = 2,  // (0x04) Address command register
    ADM2IFnr_CMDDATA = 3, // (0x06) Data command register
  };

  // Numbers of Command Registers
  enum ADM2IF_NUM_CMD_REGS
  {
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
    ADM2IFnr_MODE2 = 10,   // 0x0A
    ADM2IFnr_MODE3 = 11,   // 0x0B
    ADM2IFnr_CNTAE = 12,   // 0x0C
    ADM2IFnr_CNTAF = 13,   // 0x0D
    ADM2IFnr_PRTMODE = 14, // 0x0E
    ADM2IFnr_TLMODE = 15,  // 0x0F
    ADM2IFnr_CHAN1 = 16,   // 0x10
    ADM2IFnr_CHAN2 = 17,   // 0x11
    ADM2IFnr_FORMAT = 18,  // 0x12
    ADM2IFnr_FSRC = 19,    // 0x13
    ADM2IFnr_FDVR = 20,    // 0x14
    ADM2IFnr_GAIN = 21,    // 0x15
    ADM2IFnr_VCODAC = 24,  // 0x18
    ADM2IFnr_ECNT0 = 28,   // 0x1C
    ADM2IFnr_ECNT1 = 29,   // 0x1D
    ADM2IFnr_ECNT2 = 30,   // 0x1E
  };

  // Numbers of Constant Registers
  enum ADM2IF_NUM_CONST_REGS
  {
    ADM2IFnr_ID = 0x100,
    ADM2IFnr_IDMOD = 0x101,
    ADM2IFnr_VER = 0x102,
    ADM2IFnr_TRES = 0x103,
    ADM2IFnr_FSIZE = 0x104,
    ADM2IFnr_FTYPE = 0x105,
    ADM2IFnr_PATH = 0x106,
    ADM2IFnr_IDNUM = 0x107,
    ADM2IFnr_PFSIZE = 0x108,
  };

  // Numbers of Direct Registers
  enum ADM2IF_NUM_DIRECT_REGS
  {
    ADM2IFnr_FLAGCLR = 0x200,
    ADM2IFnr_SPDDEVICE = 0x203,
    ADM2IFnr_SPDCTRL = 0x204,
    ADM2IFnr_SPDADDR = 0x205,
    ADM2IFnr_SPDDATAL = 0x206,
    ADM2IFnr_SPDDATAH = 0x207,
    ADM2IFnr_STDDELAY = 0x2F0,
  };

} // namespace InSys
