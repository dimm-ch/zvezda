/*
 ****************** File Ddc4x16Srv.h *******************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : DDC section
 *
 * (C) InSys by Dorokhin Andrey Jan 2006
 *
 * 12.10.2006 - submodule ver 3.0 - synchronous mode
 *
 ************************************************************
*/

#ifndef _DDC4x16SRV_H
 #define _DDC4x16SRV_H

#include "ctrlddc4x16.h"
#include "ddcsrv.h"
#include "ddc4x16srvinfo.h"

const int DDC4x16S_TETR_ID = 0x07; // DDC synchronous tetrad id
const int DDC4x16_TETR_ID = 0x1A; // DDC asynchronous & combined tetrad id

//#define BASE_MAXREFS 2
//const int THRDACCNT = 8; // Number of threshold DACs
// ADC Threshold DAC number
//enum {
//	BRDtdn_ADC_BIAS0		= 3,	// bias of channel 0
//	BRDtdn_ADC_BIAS1		= 4,	// bias of channel 1
//	BRDtdn_ADC_GAINTUN0		= 5,	// gain tuning of channel 0
//	BRDtdn_ADC_GAINTUN1		= 6,	// gain tuning of channel 1
//	BRDtdn_ADC_CLKPHASE0	= 7,	// clock phase control of channel 0
//	BRDtdn_ADC_CLKPHASE1	= 8		// clock phase control of channel 1
//};

#pragma pack(push,1)

typedef struct _DDC4X16SRV_CFG {
	DDCSRV_CFG DdcCfg;
	U32		SubRefGen;				// frequency of submodule reference generator (значение субмодульного опорного генератора (√ц))
	U32		SubExtClk;				// frequency of external clock (субмодульна€ внешн€€ частота тактировани€ (√ц))
	REAL64	Decimation[BRD_DDC_CHANCNT];
} DDC4X16SRV_CFG, *PDDC4X16SRV_CFG;

// for Program command
typedef struct _BRD_DdcRec
{
	U08		data;			// data
	U08		reg;			// register address of DDC
	U08		page;			// page number of DDC
	U08		res;			// not use
} BRD_DdcRec, *PBRD_DdcRec;

typedef struct _PRGFILE_PARAM {
	U32		RefClk;				// значение опорной частоты (√ц))
	U32		ASyncMode;			// not used for ver 3.0
	U32		DdcMask;			// not used for ver 3.0
//	U32		Decim[4];
	REAL64	Decim[4];
	U32		ModeOfDdc;			// not used for ver 3.0
} PRGFILE_PARAM, *PPRGFILE_PARAM;

#pragma pack(pop)

class CDdc4x16Srv: public CDdcSrv
{

protected:

	inline void REG_POKE_IND(CModule* pModule, U32 trd, U32 reg, U32 data)
	{
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = trd;
		param.reg = reg;
		param.val = data;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}

	inline U32 REG_PEEK_IND(CModule* pModule, U32 trd, U32 reg)
	{
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = trd;
		param.reg = reg;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		return param.val;
	}

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	virtual void GetDdcTetrNum(CModule* pModule);
	virtual void* GetInfoForDialog(CModule* pModule);
	virtual void FreeInfoForDialog(PVOID pInfo);
	virtual int SetPropertyFromDialog(void	*pDev, void	*args);

	virtual int SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SetClkSource(CModule* pModule, ULONG ClkSrc);
	virtual int GetClkSource(CModule* pModule, ULONG& ClkSrc);
	virtual int SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue);
	virtual int GetRate(CModule* pModule, double& Rate, double ClkValue);

	virtual int SetInpSrc(CModule* pModule, ULONG& AdcNum, ULONG DdcChan);
	virtual int GetInpSrc(CModule* pModule, ULONG& AdcNum, ULONG DdcChan);
	virtual int SetFC(CModule* pModule, double& Freq, ULONG DdcChan);
	virtual int GetFC(CModule* pModule, double& Freq, ULONG DdcChan);
	virtual int SetFcCode(CModule* pModule, U32& NcoFreq, ULONG DdcChan);
	virtual int GetFcCode(CModule* pModule, U32& NcoFreq, ULONG DdcChan);
	virtual int SetDecim(CModule* pModule, double& Decim, ULONG DdcChan);
	virtual int GetDecim(CModule* pModule, double& Decim, ULONG DdcChan);
	virtual int SetFrame(CModule* pModule, ULONG Len);
	virtual int GetFrame(CModule* pModule, ULONG& Len);
	virtual int SetProgram(CModule* pModule, PVOID args);
	virtual int SetDDCSync(CModule* pModule, ULONG mode, ULONG prog, ULONG async);
	virtual int GetDDCSync(CModule* pModule, ULONG& mode, ULONG& async);

	virtual int SetFormat(CModule* pModule, ULONG format);
//	virtual int GetFormat(CModule* pModule, ULONG& format);

	virtual int StartEnable(CModule* pModule, ULONG Enbl);

	void ProgDdcReg(CModule* pModule, ULONG cmd, ULONG chipMask, ULONG addr, ULONG& data);
	int VerifyProgram(CModule* pModule, ULONG chipMask, PBRD_DdcRec pRec, ULONG num_rec);
	void WriteDdcReg(CModule* pModule, ULONG chipMask, ULONG page, ULONG reg, ULONG data);
	void ReadDdcReg(CModule* pModule, ULONG chipMask, ULONG page, ULONG reg, ULONG& data);
	int SetAdcMode(CModule* pModule, PBRD_Adc4x16Mode pMode);
	int GetAdcMode(CModule* pModule, PBRD_Adc4x16Mode pMode);
	int CtrlAlign(CModule* pModule, U32 cmd, PVOID arg);

	virtual int SetSpecific(CModule* pModule, PBRD_DdcSpec pSpec);
	
public:

	CDdc4x16Srv(int idx, int srv_num, CModule* pModule, PDDC4X16SRV_CFG pCfg); // constructor

};

#endif // _DDC4x16SRV_H

//
// End of file
//