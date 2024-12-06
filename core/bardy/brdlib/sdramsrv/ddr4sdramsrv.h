/*
 ****************** File ddr4sdramSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DDR4 Sdram section
 *
 * (C) InSys by Dorokhin Andrey Jan, 2017
 *
 ************************************************************
*/

#ifndef _DDR4SDRAMSRV_H
 #define _DDR4SDRAMSRV_H

#include "ctrlsdram.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "sdramregs.h"
#include "sdramsrv.h"

const int DDR4_TETR_ID = 0xE7;  // tetrad id SDRAM of FMC126P - autor Avdeev
const int DDR4_DAC_TETR_ID = 0xE8;  // tetrad id SDRAM for DAC

#pragma pack(push,1)

typedef struct _DDR4SDRAMSRV_CFG {
	BRDCHAR	DlgDllName[DLGDLLNAME_SIZE];	// название диалоговой dll
	UCHAR	isAlreadyInit;	// флаг инициализации (чтобы делать ее однократно)
	UCHAR	Mode;			// флаги возможностей ввода из памяти в хост (0x01)/ вывода в память из хоста (0x02)
	UCHAR	Res;			// резерв
	UCHAR	MemType;		// тип памяти
	UCHAR	SlotCnt;		// количество установленных слотов
	UCHAR	ModuleCnt;		// количество установленных DIMM-модулей (занятых слотов)
	UCHAR	RowAddrBits;	// количество разрядов адреса строк : 12-18 (Byte 5 - SDRAM Addressing: Bits 5~3 - Row Address Bits)
	UCHAR	ColAddrBits;	// количество разрядов адреса столбцов : 9-12 (Byte 5 - SDRAM Addressing: Bits 2~0 - Column Address Bits)
	UCHAR	ChipBanks;		// количество банков в микросхемах : 4,8 (Byte 4 - SDRAM Density and Banks: Bits 7~6 - Bank Group Bits, Bits 5~4 - Bank Address Bits)
	__int64	CapacityMbits;	// объём модуля в мегабитах : 256,512,1024,2048,4096,8192,16384, 32768 (Byte 4 - SDRAM Density and Banks: Bits 3~0 - Total SDRAM capacity, in megabits)
	UCHAR	PrimWidth;		// количество разрядов в модуле : 8,16,32,64 (Byte 13 - Module Memory Bus Width: Bits 2~0 - Primary bus width, in bits)
	UCHAR	ModuleBanks;	// количество банков в модуле : 1-8 (Byte 12 - Module Organization: Bits 5~3 - Number of Ranks)
	UCHAR	ChipWidth;		// количество разрядов в микросхемах : 4,8,16,32 (Byte 12 - Module Organization: Bits 2~0 - SDRAM Device Width)
	UCHAR	CfgSource;		// откуда брать конфигурацию: 0 и 1 - из ICR, 2 - из SPD 
	S08		TetrNum;		// номер тетрады (перебивает ID тетрады)
	U16		AdrMask;		// маска разрядов адреса памяти (биты 0..9 соответствуют адресным битам 20..29)
	U16		AdrConst;		// значение разрядов адреса памяти  (биты 0..9 соответствуют адресным битам 20..29)
} DDR4SDRAMSRV_CFG, *PDDR4SDRAMSRV_CFG;

#pragma pack(pop)

class CDdr4SdramSrv : public CSdramSrv
{

protected:

	U32 m_ControllerId;
	U32 m_CoupleNum[4];

	virtual void GetSdramTetrNum(CModule* pModule);
	virtual void GetSdramTetrNumEx(CModule* pModule, ULONG TetrInstNum);

	UCHAR ReadSpdByte(CModule* pModule, ULONG OffsetSPD, ULONG CtrlSPD);
	ULONG GetCfgFromSpd(CModule* pModule, PDDR4SDRAMSRV_CFG pCfgSPD);
	virtual ULONG CheckCfg(CModule* pModule);
	virtual void MemInit(CModule* pModule, ULONG init);
	virtual int GetCfgEx(PBRD_SdramCfgEx pCfg);
	virtual int ExtraInit(CModule* pModule);

	virtual int SetSel(CModule* pModule, ULONG& sel, ULONG cmd);
	virtual int SetTestSeq(CModule* pModule, ULONG mode);
	virtual int GetTestSeq(CModule* pModule, ULONG& mode);

public:

	CDdr4SdramSrv(int idx, int srv_num, CModule* pModule, PDDR4SDRAMSRV_CFG pCfg); // constructor
	
};

#endif // _DDR3SDRAMSRV_H

//
// End of file
//