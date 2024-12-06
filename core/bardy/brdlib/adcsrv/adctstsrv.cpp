/*
 ***************** File adctstsrv.cpp ************
 *
 * BRD Driver for ADС service
 *
 * (C) InSys by Dorokhin A. Aug 2010-2016
 *
 ******************************************************
*/

#include "module.h"
#include "adctstsrv.h"

#define	CURRFILE "ADCTSTSRV"

//***************************************************************************************
CAdcTstSrv::CAdcTstSrv(int idx, int srv_num, CModule* pModule, PADCSRV_CFG pCfg) :
	CAdcSrv(idx, _BRDC("ADCTST"), srv_num, pModule, pCfg, sizeof(ADCSRV_CFG))
{
}

//***************************************************************************************
int CAdcTstSrv::CtrlRelease(
								void			*pDev,		// InfoSM or InfoBM
								void			*pServData,	// Specific Service Data
								ULONG			cmd,		// Command Code (from BRD_ctrl())
								void			*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	CAdcSrv::SetChanMask(pModule, 0);  // ?
	return BRDerr_CMD_UNSUPPORTED; // для освобождения подслужб
}

const double BRD_ADC_MAXGAINTUN	= 2.; // max gain tuning = 2%

//***************************************************************************************
void CAdcTstSrv::GetAdcTetrNum(CModule* pModule)
{
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC212X200M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC414X65M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC818X800_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC10X2G_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC1624X192_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC214X200M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC216X100M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC212X500M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADMTEST2_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC210X1G_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, ADC212X1G_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FMC814x125M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FM214x250M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FM412x500M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FM212x1G_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FM816x250M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FM416x250M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FMADC216x250MDA_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FM814x250M_TETR_ID);
	if(m_AdcTetrNum != -1)
		return;
	m_AdcTetrNum = GetTetrNum(pModule, m_index, FMCTEST_TETR_ID);
}

//***************************************************************************************
int CAdcTstSrv::SetProperties(CModule* pModule, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	ADCTSTSRV_CFG		*pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	BRDCHAR			Buffer[128];
	BRDCHAR			*endptr;

	//
	// Извлечение параметров работы субмодуля из INI-файла
	// и инициализация субмодуля в соответствии с этими параметрами
	//

	// Извлечение стандартных параметров
	CAdcSrv::SetProperties(pModule, iniFilePath, SectionName);

	// Извлечение источника тактовой частоты
	U32			clkSrc;

	IPC_getPrivateProfileString(SectionName, _BRDC("ClockSource"), _BRDC("129"), Buffer, sizeof(Buffer), iniFilePath);
	clkSrc = BRDC_strtol(Buffer, &endptr, 0);
	SetClkSource(pModule, clkSrc);

	// Извлечение частоты внешнего генератора

	double				clkValue;

	IPC_getPrivateProfileString(SectionName, _BRDC("ExternalClockValue"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
	if (Buffer[0])
	{
		clkValue = BRDC_atof(Buffer);
		SetClkValue(pModule, BRDclks_ADC_EXTCLK, clkValue);
	}

	//GetPrivateProfileString(SectionName, _BRDC("BaseClockValue"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
	//if (Buffer[0])
	//{
	//	clkValue = BRDC_atof(Buffer);
	//	SetClkValue(pModule, BRDclks_ADC_BASEGEN, clkValue);
	//}

	IPC_getPrivateProfileString(SectionName, _BRDC("SubClockValue"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
	if (Buffer[0])
	{
		clkValue = BRDC_atof(Buffer);
		SetClkValue(pModule, BRDclks_ADC_SUBGEN, clkValue);
	}

	//
	// Извлечение требуемой частоты дискретизации
	//
	double			rate;

	IPC_getPrivateProfileString(SectionName, _BRDC("SamplingRate"), _BRDC("100000000.0"), Buffer, sizeof(Buffer), iniFilePath);
	rate = BRDC_atof(Buffer);
	SetRate(pModule, rate, 0, 0.0);

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::SetClkSource(CModule* pModule, ULONG nClkSrc)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	pSrvCfg->ClkSrc = nClkSrc;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::GetClkSource(CModule* pModule, ULONG& ClkSrc)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	ClkSrc = pSrvCfg->ClkSrc;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::SetClkValue(CModule* pModule, ULONG clkSrc, double& refClkValue)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	pSrvCfg->dClkValue = refClkValue;
	return BRDerr_OK;
}
//***************************************************************************************
int CAdcTstSrv::GetClkValue(CModule* pModule, ULONG clkSrc, double& refClkValue)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	refClkValue = pSrvCfg->dClkValue;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::SetRate(CModule* pModule, double& refRate, ULONG clkSrc, double clkValue)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	pSrvCfg->dSamplingRate = refRate;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::GetRate(CModule* pModule, double& refRate, double ClkValue)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	refRate = pSrvCfg->dSamplingRate;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::SetInpRange(CModule* pModule, double& InpRange, ULONG Chan)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcTstSrv::GetInpRange(CModule* pModule, double& InpRange, ULONG Chan)
{
	ADCTSTSRV_CFG *pSrvCfg = (ADCTSTSRV_CFG*)m_pConfig;
	InpRange = pSrvCfg->DacCfg.InpRange / 1000.;
	return BRDerr_OK;
}

// ***************** End of file adctstsrv.cpp ***********************
