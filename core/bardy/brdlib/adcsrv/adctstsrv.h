/*
 ****************** File adctstsrv.h *******************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : ADC section
 *
 * (C) InSys by Dorokhin Andrey Aug 2010-2016
 *
 ************************************************************
*/

#ifndef _ADCTSTSRV_H
 #define _ADCTSTSRV_H

#include "ctrlcmpsc.h"
#include "service.h"
#include "adm2if.h"
#include "adcsrv.h"
#include "mainregs.h"

// ADC Clock sources
enum {
	BRDclks_ADC_DISABLED = 0,	// 
	BRDclks_ADC_SUBGEN = 1,	// SubModule Internal Generator 
	BRDclks_ADC_EXTCLK = 0x81,	// External SubModule Clock
};

// tetrad id
const int ADC212X200M_TETR_ID	= 0x14;
const int ADC414X65M_TETR_ID	= 0x19;
const int ADC818X800_TETR_ID	= 0x29;
const int ADC10X2G_TETR_ID		= 0x2B;
const int ADC28X1G_TETR_ID		= 0x2F;
const int ADC1624X192_TETR_ID	= 0x3D;
const int ADC214X200M_TETR_ID	= 0x3F;
const int ADC216X100M_TETR_ID	= 0x40;
const int ADC212X500M_TETR_ID	= 0x5D;
const int ADC210X1G_TETR_ID		= 0x69;
const int ADMTEST2_TETR_ID		= 0x76;
const int ADC212X1G_TETR_ID		= 0x78;
const int FMC814x125M_TETR_ID	= 0x82;
const int FM214x250M_TETR_ID	= 0x88;
const int FM412x500M_TETR_ID	= 0x89;
const int FM212x1G_TETR_ID		= 0x92;
const int FM816x250M_TETR_ID	= 0x8B;
const int FM416x250M_TETR_ID	= 0x93;
const int FMADC216x250MDA_TETR_ID	= 0x9C;
const int FM814x250M_TETR_ID	= 0xAC;
const int FMCTEST_TETR_ID	= 0xAA;


#pragma pack(push,1)

typedef struct _ADCTSTSRV_CFG {
	ADCSRV_CFG DacCfg;
	ULONG		ClkSrc;					// источник такта
	double	dSamplingRate;	// Частота дискретизации, после выплнения SetRate()
	double	dClkValue;			// частота тактирования (Гц)
} ADCTSTSRV_CFG, *PADCTSTSRV_CFG;

#pragma pack(pop)

class CAdcTstSrv: public CAdcSrv
{

protected:

	virtual int CtrlRelease(void* pDev, void* pServData, ULONG cmd, void* args);
	virtual void GetAdcTetrNum(CModule* pModule);

	virtual int SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName);
	virtual int SetClkSource(CModule* pModule, ULONG ClkSrc);
	virtual int GetClkSource(CModule* pModule, ULONG& ClkSrc);
	virtual int SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue);
	virtual int SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue);
	virtual int GetRate(CModule* pModule, double& Rate, double ClkValue);

	virtual int SetInpRange(CModule* pModule, double& InpRange, ULONG Chan);
	virtual int GetInpRange(CModule* pModule, double& InpRange, ULONG Chan);

public:

	CAdcTstSrv(int idx, int srv_num, CModule* pModule, PADCSRV_CFG pCfg); // constructor

};

#endif // _ADCTSTSRV_H

//
// End of file
//