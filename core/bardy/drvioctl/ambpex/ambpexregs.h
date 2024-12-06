//****************** File AmbpexRegs.h *********************************
// AMBPEX card definitions
//
//	Copyright (c) 2007, Instrumental Systems,Corp.
//  Written by Dorokhin Andrey
//
//  History:
//  19-03-07 - builded
//
//*******************************************************************

#ifndef _AMBPEXREGS_H_
#define _AMBPEXREGS_H_

    // the maximum number of these devices that can be connected at one time
#define AMB_DEVICES_SUPPORTED           32

#define INSYS_VENDOR_ID		0x4953

#define AMBPEX8_DEVID		0x5503 // DeviceID for AMBPEX8
#define ADP201X1AMB_DEVID	0x5504 // DeviceID for ADP201X1AMB
#define ADP201X1DSP_DEVID	0x5505 // DeviceID for ADP201X1DSP
#define AMBPEX5_DEVID		0x5507 // DeviceID for AMBPEX5
#define UADCPEX_DEVID		0x5510 // DeviceID for УАЦП-PCI Express(Кулик)
#define AC_ADC_DEVID		0x5512 // DeviceID for AcDSP_ADC
#define AC_DSP_DEVID		0x5513 // DeviceID for AcDSP_DSP
#define AC_SYNC_DEVID		0x5514 // DeviceID for Ac_SYNC

#define XM416X250M_DEVID	0x5516 // DeviceID for XM416X250M

#define PS_DSP_DEVID		0x5518 // DeviceID for PS_DSP
#define PS_ADC_DEVID		0x5519 // DeviceID for PS_ADC
#define PS_SYNC_DEVID		0x551A // DeviceID for PS_SYNC

#define FMC105P_DEVID		0x5509 // DeviceID for FMC105P
#define FMC106P_DEVID		0x550A // DeviceID for FMC106P
#define FMC103E_DEVID		0x550B // DeviceID for FMC103E2
#define FMC114V_DEVID		0x550C // DeviceID for FMC114V
#define FMC110P_DEVID		0x550D // DeviceID for FMC110P
#define FMC113E_DEVID		0x550E // DeviceID for FMC113E
#define FMC108V_DEVID		0x550F // DeviceID for FMC108V
#define FMC107P_DEVID		0x5511 // DeviceID for FMC107P
#define FMC115CP_DEVID		0x53B1 // DeviceID for FMC115CP
#define FMC112CP_DEVID		0x53B2 // DeviceID for FMC112CP
#define FMC117CP_DEVID		0x53B3 // DeviceID for FMC117CP
#define FMC121CP_DEVID		0x53B5 // DeviceID for FMC121CP
#define FMC122P_DEVID		0x551C // DeviceID for FMC122P
#define FMC123E_DEVID		0x551D // DeviceID for FMC123E
#define FMC124P_DEVID		0x551E // DeviceID for FMC124P
#define FMC125CP_DEVID		0x53B6 // DeviceID for FMC125CP
#define FMC127P_DEVID		0x551F // DeviceID for FMC127P
#define FMC122PSL_DEVID		0x5520 // DeviceID for FMC122P-Slave
#define FMC111P_DEVID		0x5521 // DeviceID for FMC111P
#define FMC126P_DEVID		0x5522 // DeviceID for FMC126P
#define FMC132P_DEVID		0x5523 // DeviceID for FMC132P
#define FMC121P_DEVID		0x5524 // DeviceID for FMC121cP(PCIe)
#define FMC133V_DEVID		0x5525 // DeviceID for FMC133V
#define DSP134V_DEVID		0x5526 // DeviceID for DSP134V
#define RFG2_DEVID		0x5527 // DeviceID for RFG2
#define FMC131VZQ_DEVID		0x5528 // DeviceID for FMC131V Zynq
#define FMC131VKU_DEVID		0x5529 // DeviceID for FMC131V KU (FPGA)
#define FMC139P_DEVID		0x552A // DeviceID for FMC139P
#define FMC141VZQ_DEVID		0x5530 // DeviceID for FMC141V Zynq
#define FMC141VKU1_DEVID	0x5531 // DeviceID for FMC141V KU (FPGA 1)
#define FMC141VKU2_DEVID	0x5532 // DeviceID for FMC141V KU (FPGA 2)
#define FMC143VZQ_DEVID		0x552D // DeviceID for FMC143V Zynq
#define FMC143VKU_DEVID		0x552E // DeviceID for FMC143V KU (FPGA)
#define FMC146VZQ_DEVID		0x5533 // DeviceID for FMC146V Zynq
#define FMC146VKU_DEVID		0x5534 // DeviceID for FMC146V KU (FPGA)

#define D2XT006_DEVID		0x5702 // DeviceID for D2XT006
#define ABC_DEVID		0x5804 // DeviceID for ABC
#define PANORAMA_DEVID		0x53B8 // DeviceID for PANORAMA
#define FMC130E_DEVID		0x546A // DeviceID for FMC130E


#define PCI_EXPROM_SIZE 512

#define PE_MAIN_ADDR 0x000
//#define PE_FIFO_ADDR 0x100
#define PE_BLK_SIZE 0x100
#define PE_EXT_FIFO_ADDR 0x400

#define PE_MAIN_ID		0x0013
#define PE_FIFO_ID		0x0014
#define PE_EXT_FIFO_ID	0x0018
#define PE_FID_ID		0x0019

#if !defined(DZYTOOLS_C_6_X)
#pragma pack(push,1)
#endif

// PCI-Express Main block Registers Layout
typedef volatile struct _PE_MAIN_REGISTERS {
	ULONG   BlockId;	// 0 (0x00) Сontrol MAIN block identification register (only read)
	ULONG   BlockVer;	// 1 (0x04) Сontrol MAIN block version register (only read)
	ULONG   DeviceId;	// 2 (0x08) Device identification register (only read)
	ULONG   DeviceRev;	// 3 (0x0C) Device revision register (only read)
	ULONG   PldVersion;	// 4 (0x10) PLD version register (only read)
	ULONG   BlockNum;	// 5 (0x14) Number of control blocks (only read)
	ULONG   DpramOffset;// 6 (0x18) (only read)
	ULONG   DpramSize;	// 7 (0x1C) (only read)
	ULONG   BrdMode;	// 8 (0x20) Board control register
	ULONG   IrqMask;	// 9 (0x24) Interrupt mask register
	ULONG   IrqInv;		// 10(0x0A) (0x28) Interrupt inversion register
	ULONG   Leds;		// 11(0x0B) (0x2C) LEDs control register
	ULONG   Reg12;		// 12(0x0С) (0x30) REG12 - Reserved space
	ULONG   PSynx;		// 13(0x0D) (0x34) not use
	ULONG   Reg14;		// 14(0x0E) (0x38) REG14 - Reserved space
	ULONG   Reg15;		// 15(0x0F) (0x3C) REG15 - Reserved space
	ULONG   BrdStatus;	// 16(0x10) (0x40) Board status register
	ULONG   Reg17;		// 17(0x11) (0x44) REG17 - Reserved space
	ULONG   Reg18;		// 18(0x12) (0x48) REG18 - Reserved space
	ULONG   Sem0;		// 19(0x13) (0x4C) not use
	ULONG   PldConf;	// 20(0x14) (0x50) not use
	ULONG   Reg21;		// 21(0x15) (0x54) REG21 - Reserved space
	ULONG   SpdCtrl;	// 22(0x16) (0x58) SPD&ADM_ROM Access Control register
	ULONG   SpdAddr;	// 23(0x17) (0x5C) Memory address register
	ULONG   SpdDataLo;	// 24(0x18) (0x60) Memory low data register
	ULONG   SpdDataHi;	// 25(0x19) (0x64) Memory high address register
	ULONG   LbData;		// 26(0x1A) (0x68) Loopback mode data register
	ULONG   Reg27;		// 27(0x1B) (0x6C) REG27 - Reserved space
	ULONG   JtagCnt;	// 28(0x1C) (0x70) number of shift bit register
	ULONG   JtagTms;	// 29(0x1D) (0x74) signal TMS register
	ULONG   JtagTdi;	// 30(0x1E) (0x78) signal TDI register
	ULONG   JtagTdo;	// 31(0x1F) (0x7C) signal TDO register
} PE_MAIN_REGISTERS, *PPE_MAIN_REGISTERS;

// Numbers of PCI-Express Main block Registers
typedef enum _PE_MAIN_ADDR_REG {
    PEMAINadr_BLOCK_ID		= 0x00, // 0x00
    PEMAINadr_BLOCK_VER		= 0x08, // 0x01
    PEMAINadr_DEVICE_ID		= 0x10, // 0x02
    PEMAINadr_DEVICE_REV	= 0x18, // 0x03
    PEMAINadr_PLD_VER		= 0x20, // 0x04
    PEMAINadr_BLOCK_CNT		= 0x28, // 0x05
    PEMAINadr_DPRAM_OFFSET	= 0x30, // 0x06
    PEMAINadr_DPRAM_SIZE	= 0x38, // 0x07
    PEMAINadr_BRD_MODE		= 0x40, // 0x08
    PEMAINadr_IRQ_MASK		= 0x48, // 0x09
    PEMAINadr_IRQ_INV		= 0x50, // 0x0A
    PEMAINadr_LEDS			= 0x58, // 0x0B
    PEMAINadr_BRD_STATUS	= 0x80, // 0x10
    PEMAINadr_SPD_CTRL		= 0xB0, // 0x16
    PEMAINadr_SPD_ADDR		= 0xB8, // 0x17
    PEMAINadr_SPD_DATAL		= 0xC0, // 0x18
    PEMAINadr_SPD_DATAH		= 0xC8, // 0x19
    PEMAINadr_LB_DATA		= 0xD0, // 0x1A
    PEMAINadr_JTAG_CNT		= 0xE0, // 0x1C
    //PEMAINadr_JTAG_TMS		= 0xE8, // 0x1D
    //PEMAINadr_JTAG_TDI		= 0xF0, // 0x1E
    //PEMAINadr_JTAG_TDO		= 0xF8, // 0x1F
	PEMAINadr_BOARD_NUM		= 0xF8, // 0x1F
} PE_MAIN_ADDR_REG;  

// PCI-Express FIFO block Registers Layout
typedef volatile struct _PE_FIFO_REGISTERS {
	ULONG   BlockId;	// 0 (0x00) Сontrol FIFO block identification register (only read)
	ULONG   BlockVer;	// 1 (0x04) Сontrol FIFO block version register (only read)
	ULONG   FifoId;		// 2 (0x08) FIFO identification register (only read)
	ULONG   FifoNumber;	// 3 (0x0C) FIFO number register (only read)
	ULONG   DmaSize;	// 4 (0x10) DMA size register (only read) - NOT use by EXT
	ULONG   Reg5;		// 5 (0x14) REG5 - Reserved space
	ULONG   Reg6;		// 6 (0x18) REG6 - Reserved space
	ULONG   Reg7;		// 7 (0x1С) REG7 - Reserved space
	ULONG   FifoCtlr;	// 8 (0x20) FIFO control register - NOT use by EXT
	ULONG   DmaCtlr;	// 9 (0x24) DMA control register
	ULONG   Reg10;		// 10(0x0A) (0x28) REG10 - Reserved space
	ULONG   Reg11;		// 11(0x0B) (0x2C) REG11 - Reserved space
	ULONG   Reg12;		// 12(0x0С) (0x30) REG12 - Reserved space
	ULONG   Reg13;		// 13(0x0D) (0x34) REG13 - Reserved space
	ULONG   Reg14;		// 14(0x0E) (0x38) REG14 - Reserved space
	ULONG   Reg15;		// 15(0x0F) (0x3C) REG15 - Reserved space
	ULONG   FifoStatus;	// 16(0x10) (0x40) FIFO status register
	ULONG   FlagClr;	// 17(0x11) (0x44) Flags clear register
	ULONG   Reg18;		// 18(0x12) (0x48) REG18 - Reserved space
	ULONG   Reg19;		// 19(0x13) (0x4C) REG19 - Reserved space
	ULONG   PciAddrLo;	// 20(0x14) (0x50) PCI address (low part) register
	ULONG   PciAddrHi;	// 21(0x15) (0x54) PCI address (high part) register
	ULONG   PciSize;	// 22(0x16) (0x58) block size register - NOT use by EXT
	ULONG   LocalAddr;	// 23(0x17) (0x5C) Local address register
	ULONG   Reg24;		// 24(0x18) (0x60) REG24 - Reserved space
	ULONG   Reg25;		// 25(0x19) (0x64) REG25 - Reserved space
	ULONG   Reg26;		// 26(0x1A) (0x68) REG26 - Reserved space
	ULONG   Reg27;		// 27(0x1B) (0x6C) REG27 - Reserved space
	ULONG   Reg28;		// 28(0x1C) (0x70) REG28 - Reserved space
	ULONG   Reg29;		// 29(0x1D) (0x74) REG29 - Reserved space
	ULONG   Reg30;		// 30(0x1E) (0x78) REG30 - Reserved space
	ULONG   Reg31;		// 31(0x1F) (0x7C) REG31 - Reserved space
} PE_FIFO_REGISTERS, *PPE_FIFO_REGISTERS;

// Numbers of PCI-Express FIFO block Registers
typedef enum _PE_FIFO_ADDR_REG {
    PEFIFOadr_BLOCK_ID		= 0x00, // 0x00
    PEFIFOadr_BLOCK_VER		= 0x08, // 0x01
    PEFIFOadr_FIFO_ID		= 0x10, // 0x02
    PEFIFOadr_FIFO_NUM		= 0x18, // 0x03
    PEFIFOadr_DMA_SIZE		= 0x20, // 0x04 - RESOURCE by EXT
    PEFIFOadr_FIFO_CTRL		= 0x40, // 0x08 - DMA_MODE by EXT
    PEFIFOadr_DMA_CTRL		= 0x48, // 0x09
    PEFIFOadr_BLOCK_CNT		= 0x50, // 0x0A
    PEFIFOadr_FIFO_STATUS	= 0x80, // 0x10
    PEFIFOadr_FLAG_CLR		= 0x88, // 0x11
    PEFIFOadr_ERROR_CNT		= 0x90, // 0x12
    PEFIFOadr_PCI_ADDRL		= 0xA0, // 0x14
    PEFIFOadr_PCI_ADDRH		= 0xA8, // 0x15
    PEFIFOadr_PCI_SIZE		= 0xB0, // 0x16 - NOT use by EXT
    PEFIFOadr_LOCAL_ADR		= 0xB8, // 0x17
	PEFIFOadr_IRQ_MODE		= 0xE0, // 0x1C
	PEFIFOadr_IRQ_TBLADR	= 0xE8, // 0x1D
	PEFIFOadr_IRQ_TBLDATA	= 0xF0, // 0x1E
	PEFIFOadr_IRQ_CNT		= 0xF8, // 0x1F
} PE_FIFO_ADDR_REG;  

// Numbers of PCI-Express FID (FMC identifier) block Registers
typedef enum _PE_FID_ADDR_REG {
    PEFIDadr_BLOCK_ID		= 0x00, // 0x00
    PEFIDadr_BLOCK_VER		= 0x08, // 0x01
    PEFIDadr_CONST_V0		= 0x10, // 0x02
    PEFIDadr_CONST_V1		= 0x18, // 0x03
    PEFIDadr_CONST_V2		= 0x20, // 0x04
    PEFIDadr_CONST_V3		= 0x28, // 0x05
//    PEMAINadr_DPRAM_OFFSET	= 0x30, // 0x06
//    PEMAINadr_DPRAM_SIZE	= 0x38, // 0x07
//    PEMAINadr_BRD_MODE		= 0x40, // 0x08
//    PEMAINadr_IRQ_MASK		= 0x48, // 0x09
//    PEMAINadr_IRQ_INV		= 0x50, // 0x0A
    PEFIDadr_POWER_CTRL		= 0x58, // 0x0B
    PEFIDadr_POWER_ROM		= 0x60, // 0x0C
    PEFIDadr_POWER_STATUS	= 0x80, // 0x10
    PEFIDadr_SPD_DEVICE	= 0xA8, // 0x15
    PEFIDadr_SPD_CTRL		= 0xB0, // 0x16
    PEFIDadr_SPD_ADDR		= 0xB8, // 0x17
    PEFIDadr_SPD_DATAL		= 0xC0, // 0x18
    PEFIDadr_SPD_DATAH		= 0xC8, // 0x19
} PE_FID_ADDR_REG;  

// FMC SPD Control register 0xB0 (PE_FID)
typedef union _FMC_SPD_CTRL {
	USHORT AsWhole; // SPD Control Register as a Whole Word
	struct { // SPD Control Register as Bit Pattern
		USHORT	ReadOp		: 1, // Read Operation
				WriteOp		: 1, // Write Operation
				Res			: 2, // Reserved
				Addr		: 7, // address on I2C bus
				Res1		: 4, // Reserved
				Ready		: 1; // Data Ready
	} ByBits;
} FMC_SPD_CTRL, *PFMC_SPD_CTRL;

// Board Control register 0x40 (PE_MAIN)
typedef union _BRD_MODE {
	USHORT AsWhole; // Board Control Register as a Whole Word
	struct { // Board Control Register as Bit Pattern
		USHORT	RstClkOut	: 1, // Output Clock Reset for ADMPLD
				Sleep		: 1, // Sleep mode for ADMPLD
				RstClkIn	: 1, // Input Clock Reset for ADMPLD
				Reset		: 1, // Reset for ADMPLD
				RegLoop		: 1, // Register operation loopback mode
				Res			: 3, // Reserved
				OutFlags	: 8; // Output Flags
	} ByBits;
} BRD_MODE, *PBRD_MODE;

// Board Status register 0x80 (PE_MAIN)
typedef union _BRD_STATUS {
	USHORT AsWhole; // Board Status Register as a Whole Word
	struct { // Board Status Register as Bit Pattern
		USHORT	SlvDcm		: 1, // Capture Input Clock from ADMPLD
				NotUse		: 7, // 
				InFlags		: 8; // Input Flags
	} ByBits;
} BRD_STATUS, *PBRD_STATUS;

// SPD Control register 0xB0 (PE_MAIN)
typedef union _SPD_CTRL {
	USHORT AsWhole; // SPD Control Register as a Whole Word
	struct { // SPD Control Register as Bit Pattern
		USHORT	ReadOp		: 1, // Read Operation
				WriteOp		: 1, // Write Operation
				Res			: 2, // Reserved
				WriteEn		: 1, // Write Enable
				WriteDis	: 1, // Read Disable
				Res1		: 2, // Reserved
				SpdId		: 3, // SPD Identification
				Res2		: 3, // Reserved
				Sema		: 1, // Semaphor
				Ready		: 1; // Data Ready
	} ByBits;
} SPD_CTRL, *PSPD_CTRL;

// FIFO ID register 0x10 (PE_FIFO)
typedef union _FIFO_ID {
	USHORT AsWhole; // FIFO ID Register as a Whole Word
	struct { // FIFO ID Register as Bit Pattern
		USHORT	Size		: 12, // FIFO size (32-bit words)
				Dir			: 4; // DMA direction
	} ByBits;
} FIFO_ID, *PFIFO_ID;

// FIFO Control register 0x40 (PE_FIFO)
typedef union _FIFO_CTRL {
	USHORT AsWhole; // FIFO Control Register as a Whole Word
	struct { // FIFO Control Register as Bit Pattern
		USHORT	Reset		: 1, // FIFO Reset
				DrqEn		: 1, // DMA Request enable
				Loopback	: 1, // Not use
				TstCnt		: 1, // 32-bit Test Counter enable
				Res			: 12; // Reserved
	} ByBits;
} FIFO_CTRL, *PFIFO_CTRL;

// DMA Mode register 0x40 (PE_EXT_FIFO)
typedef union _DMA_MODE_EXT {
	USHORT AsWhole; // DMA Mode Register as a Whole Word
	struct { // FIFO Control Register as Bit Pattern
		USHORT	SGModeEnbl	: 1, // 1 - Scatter/Gather mode enable
				DemandMode	: 1, // 1 - always
				Dir		: 1, // DMA direction
				Res0		: 2, // Reserved
				IntEnbl		: 1, // Interrrupt enable (End of DMA)
				StepEnbl    	: 1, // 1 - STEP mode
				Res		: 9; // Reserved
	} ByBits;
} DMA_MODE_EXT, *PDMA_MODE_EXT;

// DMA Control register 0x48 (PE_FIFO)
typedef union _DMA_CTRL {
	USHORT AsWhole; // DMA Control Register as a Whole Word
	struct { // DMA Control Register as Bit Pattern
		USHORT	Start		: 1, // DMA start
				Stop		: 1, // DMA stop
				SGEnbl		: 1, // Scatter/Gather mode enable
				Res0		: 1, // Reserved
				DemandMode	: 1, // 1 - always
				IntEnbl		: 1, // Interrrupt enable (End of DMA)
				Res			: 10; // Reserved
	} ByBits;
} DMA_CTRL, *PDMA_CTRL;

// DMA Control register 0x48 (PE_EXT_FIFO)
typedef union _DMA_CTRL_EXT {
	USHORT AsWhole; // DMA Control Register as a Whole Word
	struct { // DMA Control Register as Bit Pattern
		USHORT	Start		: 1, // DMA start/stop
				Res0		: 2, // Reserved
				Pause		: 1, // DMA pause
				ResetFIFO	: 1, // FIFO Reset
				Res			: 11; // Reserved
	} ByBits;
} DMA_CTRL_EXT, *PDMA_CTRL_EXT;

// IRQ Mode register 0xE0 (PE_EXT_FIFO)
typedef union _IRQ_EXT_MODE {
	ULONG	AsWhole; // IRQ Mode Register as a Whole Word
	struct { // DMA Control Register as Bit Pattern
		ULONG	Cnt			: 8, // Table counter
				Mode		: 1, // 0 - INTA, 1 - IRQ table
				Res			: 15, // Reserved
				Sign		: 8; // Reserved
	} ByBits;
} IRQ_EXT_MODE, *PIRQ_EXT_MODE;

// FIFO Status register 0x80 (PE_FIFO)
typedef union _FIFO_STATUS {
	USHORT AsWhole; // FIFO Status Register as a Whole Word
	struct { // FIFO Status Register as Bit Pattern
		USHORT	DmaStat		: 4, // DMA Status
				DmaEot		: 1, // DMA block Complete (End of Transfer)
				SGEot		: 1, // Scatter/Gather End of Transfer (all blocks complete)
				IntErr		: 1, // not serviced of interrrupt - ERROR!!! block leave out!!!
				IntRql		: 1, // Interrrupt request
//				DmaErr		: 1, // DMA channel error
				DscrErr		: 1, // Descriptor error
				NotUse		: 3, // Not Use
				Sign		: 6; // Signature (0x0A)
	} ByBits;
} FIFO_STATUS, *PFIFO_STATUS;

/*
// Tetrad Registers Layout
typedef volatile struct _ADM2IF_TETRAD {
	ULONG	STATUS;		// (0x00) Status register
	ULONG	DATA;		// (0x04) Data register
	ULONG	CMD_ADR;	// (0x08) Address command register
	ULONG	CMD_DATA;	// (0x0С) Data command register
} ADM2IF_TETRAD, *PADM2IF_TETRAD;
*/

// Numbers of Tetrad Registers
typedef enum _TETRAD_REG {
    TRDadr_STATUS,
    TRDadr_DATA,
    TRDadr_CMD_ADR,
    TRDadr_CMD_DATA
} TETRAD_REG, *PTETRAD_REG;

typedef enum _AmbStatusRegBits {
	AMB_statCMDRDY = 1
} AmbStatusRegBits;

// Main Select of Interrupts & DMA channels Register (MODE0 +16)
typedef union _MAIN_SELX {
	ULONG AsWhole; // Board Mode Register as a Whole Word
	struct { // Mode Register as Bit Pattern
		ULONG	IrqNum	: 4, // Interrupt number
				Res1	: 4, // Reserved
				DmaTetr	: 4, // Tetrad number for DMA channel X
				DrqEnbl	: 1, // DMA request enable
				DmaMode	: 3; // DMA mode
	} ByBits;
} MAIN_SELX, *PMAIN_SELX;

#if !defined(DZYTOOLS_C_6_X)
#pragma pack(pop)
#endif

//#define REG_SIZE	sizeof(ULONG)			// register size
//#define TETRAD_SIZE	sizeof(ADM2IF_TETRAD)	// tetrad size
#define REG_SIZE	0x00001000			// register size
#define TETRAD_SIZE	0x00004000		// tetrad size
#define ADM_SIZE	0x00020000		// ADM interface size

// Numbers of Registers
typedef enum _AMB_AUX_NUM_REG {  
    AUXnr_ADM_PLD_DATA			= 0,
    AUXnr_ADM_PLD_MODE_STATUS	= 1,
    AUXnr_SUBMOD_ID_ROM			= 2,
    AUXnr_DSP_PLD_DATA			= 3,
    AUXnr_DSP_PLD_MODE_STATUS	= 4,
} AMB_AUX_NUM_REG;  

// Numbers of Direct Registers
typedef enum _MAIN_NUM_DIRECT_REGS {
	MAINnr_IRQENST	= 0x200,
	MAINnr_IRQENCL	= 0x201,
} MAIN_NUM_DIRECT_REGS;

#endif //_AMBPEXREGS_H_
