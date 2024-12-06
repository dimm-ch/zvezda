/********************************************************************
 *  (C) Copyright 1992-2012 InSys Corp.  All rights reserved. 
 *
 *  Autor	- DOR
 *
 *  filename	- ti6678_pcie.h
 *  version 	- 1.0
 *  description:
 *		TI6678 PCIe Library
 *
 *
 *  Modify:
 ********************************************************************/

#ifndef	__TI6678HW__
#define	__TI6678HW__

//#define	VENDORID	0x104C
//#define	DEVICEID	0xB005
#define	VENDOR_ID		0x4953
#define	DEVICE_ID_EVM	0x6678
#define	DEVICE_ID_110	0x6610
#define	DEVICE_ID_114	0x6614

#define MAGIC_ADDR          0x0087FFFC

#define	MAX_TI6678_DEVICES	32

#define	TI6678_CORES	8
#define	TI6674_CORES	4
#define	TI6672_CORES	2
#define	TI6671_CORES	1

typedef struct TI6678_DEVICE {
	size_t	mem_pciereg;
	size_t	mem_bar1;
	size_t	mem_bar2;
	size_t	mem_bar3;
	
	uintptr_t	mem_pciereg_va;
	uintptr_t	mem_bar1_va;
	uintptr_t	mem_bar2_va;
	uintptr_t	mem_bar3_va;

	ULONG	bar1_size;
	ULONG	bar2_size;
	ULONG	bar3_size;

	ULONG	irq;
	ULONG	deviceId;
	ULONG	pcibus;
	ULONG	pcidev;
	ULONG	pcislot;
	ULONG	revision;

	UCHAR	*loadBufByte;
	ULONG	*loadBuf;
	ULONG	*checkBuf;

}TI6678_DEVICE;

// Addresses
#define L2_START	                   0x00800000
#define MSM_START		               0x0C000000  
#define DDR3_START			           0x80000000
#define PCIE_DATA				       0x60000000  

/* Data size in bytes when r/w data bewteen GPP and DSP via EDMA:
   GPP----PCIE link----PCIE data space----EDMA----DSP device memory (L2, DDR, ...) */
#define DMA_TRANSFER_SIZE            0x400000   /* 4MB */

/* For 1MB outbound translation window size */
#define PCIE_ADLEN_1MB               0x00100000
#define PCIE_1MB_BITMASK             0xFFF00000

/* Payload size in bytes over PCIE link. PCIe module supports 
   outbound payload size of 128 bytes and inbound payload size of 256 bytes */
#define PCIE_TRANSFER_SIZE           0x80               

/* Power domains definitions */
#define PD0         0     // Power Domain-0
#define PD1         1     // Power Domain-1
#define PD2         2     // Power Domain-2
#define PD3         3     // Power Domain-3		- PCIe
#define PD4         4     // Power Domain-4		- SRIO
#define PD5         5     // Power Domain-5		- HYPER
#define PD6         6     // Power Domain-6
#define PD7         7     // Power Domain-7		- MSMCRAM

#define PD8         8     // Power Domain-8		- core 0
#define PD9         9     // Power Domain-9		- core 1
#define PD10        10    // Power Domain-10	- core 2
#define PD11        11    // Power Domain-11	- core 3	
#define PD12        12    // Power Domain-12	- core 4
#define PD13        13    // Power Domain-13	- core 5
#define PD14        14    // Power Domain-14	- core 6
#define PD15        15    // Power Domain-15	- core 7
#define PD16        16    // Power Domain-16	- core 8
#define PD17        17    // Power Domain-17	- core 0

/* Modules on power domain 0 */
#define LPSC_EMIF16_SPI  3  
#define LPSC_TSIP        4

/* Modules on power domain 1 */
#define LPSC_DEBUG       5
#define LPSC_TETB_TRC    6

/* Modules on power domain 2 */
#define LPSC_PA          7  
#define LPSC_SGMII       8  
#define LPSC_SA          9  

/* Modules on power domain 3 */
#define LPSC_PCIE        10

/* Modules on power domain 4 */
#define LPSC_SRIO        11

/* Modules on power domain 5 */
#define LPSC_HYPER       12

/* Modules on power domain 6 */
#define LPSC_RESERV      13

/* Modules on power domain 7 */
#define LPSC_MSMCRAM     14

/* Modules on power domain 8 */
#define LPSC_C0_TIM0     15

/* Modules on power domain 9 */
#define LPSC_C1_TIM1     16

/* Modules on power domain 10 */
#define LPSC_C2_TIM2     17

/* Modules on power domain 11 */
#define LPSC_C3_TIM3     18

/* Modules on power domain 12 */
#define LPSC_C4_TIM4     19

/* Modules on power domain 13 */
#define LPSC_C5_TIM5     20

/* Modules on power domain 14 */
#define LPSC_C6_TIM6     21

/* Modules on power domain 15 */
#define LPSC_C7_TIM7     22

#define PSC_SWRSTDISABLE             0x0
#define PSC_ENABLE                   0x3

#define LOC_RST_ASSERT               0x0
#define LOC_RST_DEASSERT             0x1

/* CIC registers */
#define CIC0_BASE_ADDRESS            0x02600000
#define CIC1_BASE_ADDRESS            0x02604000
#define CIC2_BASE_ADDRESS            0x02608000
#define CIC3_BASE_ADDRESS            0x0260C000
#define CIC_REVISION		         0x00
#define CIC_CONTROL			         0x04
#define CIC_GLOBAL_ENABLE	         0x10
#define CIC_SYS_STAT_IDX_SET         0x20
#define CIC_SYS_STAT_IDX_CLR         0x24
#define CIC_SYS_ENB_IDX_SET          0x28
#define CIC_SYS_ENB_IDX_CLR          0x2C
#define CIC_HOST_ENB_IDX_SET         0x34
#define CIC_HOST_ENB_IDX_CLR         0x38
#define CIC_GLOBAL_PRIO_IDX          0x80

#define PCIEXpress_Legacy_INTA                 50
#define PCIEXpress_Legacy_INTB                 51

/* PCIE registers */
#define PCIE_BASE_ADDRESS            0x21800000
#define OB_SIZE                      0x30
#define PRIORITY                     0x3C
#define EP_IRQ_SET                   0x64
#define EP_IRQ_CLR                   0x68
#define EP_IRQ_STATUS                0x6C
#define GPR(n)				 (0x70 + (0x4 * (n)))	
#define LEGACY_A_IRQ_STATUS_RAW      0x180
#define LEGACY_A_IRQ_ENABLE_SET      0x188
#define LEGACY_A_IRQ_ENABLE_CLR      0x18C
#define OB_OFFSET_INDEX(n)           (0x200 + (8 * (n)))
#define OB_OFFSET_HI(n)              (0x204 + (8 * (n)))
#define IB_BAR(n)                    (0x300 + (0x10 * (n)))
#define IB_START_LO(n)               (0x304 + (0x10 * (n)))
#define IB_START_HI(n)               (0x308 + (0x10 * (n)))
#define IB_OFFSET(n)                 (0x30C + (0x10 * (n)))

/* PSC registers */
#define PSC_BASE_ADDRESS             0x02350000
#define PTCMD                        0x120
#define PTSTAT                       0x128
#define PDSTAT(n)                    (0x200 + (4 * (n)))
#define PDCTL(n)                     (0x300 + (4 * (n)))
#define MDSTAT(n)                    (0x800 + (4 * (n)))
#define MDCTL(n)                     (0xA00 + (4 * (n))) 

/* EDMA registers */
#define EDMA_TPCC0_BASE_ADDRESS      0x02700000
#define DMAQNUM0                     0x0240  
#define ESR                          0x1010 
#define EESR                         0x1030                 
#define IESR                         0x1060
#define IPR                          0x1068 
#define ICR                          0x1070 
#define PARAM_0_OPT                  0x4000
#define PARAM_0_SRC                  0x4004
#define PARAM_0_A_B_CNT              0x4008
#define PARAM_0_DST                  0x400C
#define PARAM_0_SRC_DST_BIDX         0x4010
#define PARAM_0_LINK_BCNTRLD         0x4014
#define PARAM_0_SRC_DST_CIDX         0x4018
#define PARAM_0_CCNT                 0x401C

/* Chip level registers */
#define CHIP_LEVEL_BASE_ADDRESS      0x02620000
#define KICK0                        0x38    
#define KICK1                        0x3C
#define KICK0_UNLOCK                 0x83E70B13
#define KICK1_UNLOCK                 0x95A4F1E0 
#define KICK_LOCK                    0x0
#define DSP_BOOT_ADDR(n)             (0x040 + (4 * (n)))
#define IPCGR(n)                     (0x240 + (4 * (n)))

#endif	//__TI6678HW__