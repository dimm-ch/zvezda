#ifndef _FM415SREGS_H
	#define _FM415SREGS_H

#pragma pack(push,1)
typedef union _FM415S_STATUS {
	ULONG AsWhole;
	struct {
		ULONG	CmdRdy : 1,
			FifoRdy : 1,
			Empty : 1,
			AlmostEmpty : 1,
			HalfFull : 1,
			AlmostFull : 1,
			Full : 1,
			Overflow : 1,
			Underflow : 1,
			Reserved : 6,			
			Init : 1;
	} ByBits;
} FM415S_STATUS, *PFM415S_STATUS;

//typedef union _FM415S_CONTROL {//0x15
//	ULONG AsWhole;
//	struct {
//		ULONG PhyEnable		: 1,
//			LinkEnable		: 1,
//			Reserved		: 14;
//	} ByBits;
//} FM415S_CONTROL, *PFM415S_CONTROL;

typedef union _FM415S_TEST_SEQUENCE {//0x0C
	ULONG AsWhole;
	struct {
		ULONG Reserved1 : 9,
			Gen_Enable	: 1,
			Cnt_Enable	: 1,
			Reserved2	: 5;
	} ByBits;
} FM415S_TEST_SEQUENCE, *PFM415S_TEST_SEQUENCE;

//typedef union _FM415S_MODE1 {//0x9
//	ULONG AsWhole;
//	struct {
//		ULONG Rx0En : 1,
//			Rx1En : 1,
//			Rx2En : 1,
//			Rx3En : 1,
//			Rx4En : 1,
//			Rx5En : 1,
//			Rx6En : 1,
//			Rx7En : 1,
//			Reserved : 8;
//	} ByBits;
//} FM415S_MODE1, *PFM415S_MODE1;

typedef union _FM415S_SFB_TXDIS {//0x8
	ULONG AsWhole;
	struct {
		ULONG SFB_TXDIS0 : 1,
			SFB_TXDIS1 : 1,
			SFB_TXDIS2 : 1,
			SFB_TXDIS3 : 1,
			Reserved : 12;
	} ByBits;
} FM415S_SFB_TXDIS, * PFM415S_SFB_TXDIS;

typedef union _FM415S_AURORA_MODE1 {//0x9
	ULONG AsWhole;
	struct {
		ULONG StartTx : 1,
			Reserved : 15;
	} ByBits;
} FM415S_AURORA_MODE1, * PFM415S_AURORA_MODE1;

typedef union _FM415S_AURORA_STATUS {
	ULONG AsWhole;
	struct {
		ULONG	CmdRdy : 1,			
			Reserved : 9,
			LaneUp	: 1,
			Reserved2 : 4,
			GT_PLL_Lock : 1;
	} ByBits;
} FM415S_AURORA_STATUS, * PFM415S_AURORA_STATUS;

#pragma pack(pop)

const ULONG FM415S_REG_CHAN1 = 0x10;
const ULONG FM415S_REG_LC_FLAG = 0x16;
const ULONG FM415S_REG_RM_FLAG = 0x208;
const ULONG FM415S_REG_FLAG_CLR = 0x200;
const ULONG FM415S_REG_LINE_STATUS = 0x209;
const ULONG FM415S_REG_LINK_STATUS = 0x20A;
const ULONG FM415S_REG_DATA_STATUS = 0x20B;
const ULONG FM415S_REG_LINK0_ERR_CORR = 0x20C;
const ULONG FM415S_REG_LINK1_ERR_CORR = 0x20D;
const ULONG FM415S_REG_LINK2_ERR_CORR = 0x210;
const ULONG FM415S_REG_LINK3_ERR_CORR = 0x211;

const ULONG FM415S_REG_TEST_SEQUENCE = 0x0C;
const ULONG FM415S_REG_DECIM = 0x0D;
const ULONG FM415S_REG_ERR_ADR = 0x10;
//const ULONG FM415S_REG_CONTROL = 0x17;

const ULONG FM415S_REG_SLKB_SEL = 0x17;
const ULONG FM415S_REG_SFB_TXDIS = 0x18;

const ULONG FM415S_SPD_DEVICE = 0x203;
const ULONG FM415S_SPD_CTRL = 0x204;
const ULONG FM415S_SPD_ADDR = 0x205;
const ULONG FM415S_SPD_DATA = 0x206;
const ULONG FM415S_SPD_DATAH = 0x207;

//const ULONG SPDdev_FM415S_TCA62425 = 0x01;
const ULONG SPDdev_FM415S_GEN = 0x05;
const ULONG SPDdev_FM415S_SFP = 0x01;

const ULONG FM415S_SPD_REG_CNT_TX_L = 0x200;
const ULONG FM415S_SPD_REG_CNT_TX_H = 0x201;
const ULONG FM415S_SPD_REG_CNT_RX_L = 0x202;
const ULONG FM415S_SPD_REG_CNT_RX_H = 0x203;

const ULONG FM415S_SPD_REG_ERR_DATA = 0x204;

const ULONG FM415S_ERR_ADDR_CH0_DATA = 0x0;
const ULONG FM415S_ERR_ADDR_CH1_DATA = 0x1000;
//const ULONG FM415S_ERR_ADDR_CH2_DATA = 0x2000;
//const ULONG FM415S_ERR_ADDR_CH3_DATA = 0x3000;
const ULONG FM415S_ERR_ADDR_BLOCK_RD_L = 0x2000;
const ULONG FM415S_ERR_ADDR_BLOCK_RD_H = 0x2001;
const ULONG FM415S_ERR_ADDR_BLOCK_OK_L = 0x2002;
const ULONG FM415S_ERR_ADDR_BLOCK_OK_H = 0x2003;
const ULONG FM415S_ERR_ADDR_BLOCK_ERR_L = 0x2004;
const ULONG FM415S_ERR_ADDR_BLOCK_ERR_H = 0x2005;
const ULONG FM415S_ERR_ADDR_BLOCK_WR_L = 0x2006;
const ULONG FM415S_ERR_ADDR_BLOCK_WR_H = 0x2007;

const ULONG FM415S_ERR_DATA_READ_D0 = 0x0;
const ULONG FM415S_ERR_DATA_READ_D1 = 0x1;
const ULONG FM415S_ERR_DATA_READ_D2 = 0x2;
const ULONG FM415S_ERR_DATA_READ_D3 = 0x3;
const ULONG FM415S_ERR_DATA_EXPECT_D0 = 0x4;
const ULONG FM415S_ERR_DATA_EXPECT_D1 = 0x5;
const ULONG FM415S_ERR_DATA_EXPECT_D2 = 0x6;
const ULONG FM415S_ERR_DATA_EXPECT_D3 = 0x7;
const ULONG FM415S_ERR_DATA_INDEX = 0x8;
const ULONG FM415S_ERR_DATA_BLOCK_D0 = 0x9;
const ULONG FM415S_ERR_DATA_BLOCK_D1 = 0xA;

const ULONG FM415S_ERR_DATA_TOTAL_ERR_L = 0x100;
const ULONG FM415S_ERR_DATA_TOTAL_ERR_H = 0x101;

#endif //_FM415SREGS_H