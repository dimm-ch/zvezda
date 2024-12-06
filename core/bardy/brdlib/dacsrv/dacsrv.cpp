/*
 ***************** File DacSrv.cpp ************
 *
 * BRD Driver for DAРЎ service on DAРЎ submodule
 *
 * (C) InSys by Dorokhin A. Oct 2005
 *
 ******************************************************
*/
#ifndef __linux__
#include <windows.h>
#include <winioctl.h>
#endif

#include "module.h"
#include "dacsrv.h"

#define	CURRFILE "DACSRV"

//***************************************************************************************
ULONG CDacSrv::GetParamForStream(CModule* pModule)
{
	ULONG m_AdmNum;
	BRDCHAR buf[SERVNAME_SIZE];
	BRDC_strcpy(buf, m_name);
	BRDCHAR *pBuf = buf + (BRDC_strlen(buf) - 2);
	if(BRDC_strchr(pBuf, '1'))
		m_AdmNum = 1;
	else
		m_AdmNum = 0;
	return ((m_AdmNum << 16) | m_DacTetrNum);
}

//***************************************************************************************
void CDacSrv::GetDacTetrNum(CModule* pModule)
{
	m_DacTetrNum = 0;
}

//***************************************************************************************
int CDacSrv::GetTraceText(CModule* pModule, U32 traceId, U32 sizeb, BRDCHAR *pText)
{
	pText[0] = '\0';

	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetTraceParam(CModule* pModule, U32 traceId, U32 sizeb, U32 *pRealSizeb, void *pParam )
{
	*pRealSizeb = 0;

	return BRDerr_OK;
}

//***************************************************************************************
void CDacSrv::FreeInfoForDialog(PVOID pInfo)
{
	PDACSRV_INFO pSrvInfo = (PDACSRV_INFO)pInfo;
	delete pSrvInfo;
}

//***************************************************************************************
PVOID CDacSrv::GetInfoForDialog(CModule* pDev)
{
	PDACSRV_CFG pSrvCfg = (PDACSRV_CFG)m_pConfig;
	PDACSRV_INFO pSrvInfo = new DACSRV_INFO;
	pSrvInfo->Size = sizeof(DACSRV_INFO);
	pSrvInfo->Bits = pSrvCfg->Bits;
	pSrvInfo->Encoding = pSrvCfg->Encoding;
	pSrvInfo->FifoSize = pSrvCfg->FifoSize;
	pSrvInfo->ChanCnt = pSrvCfg->MaxChan;
	pSrvInfo->MinRate = pSrvCfg->MinRate;
	pSrvInfo->MaxRate = pSrvCfg->MaxRate;
	pSrvInfo->BaseRefGen[0] = pSrvCfg->BaseRefGen[0];
	pSrvInfo->BaseRefGen[1] = pSrvCfg->BaseRefGen[1];
	pSrvInfo->SysRefGen = pSrvCfg->SysRefGen;
	CDacSrv::GetChanMask(pDev, pSrvInfo->ChanMask);
//	ULONG chan_mask;
//	CtrlChanMask(pDev, NULL, BRDctrl_DAC_GETCHANMASK, &chan_mask);
//	pSrvInfo->ChanMask = chan_mask;
	ULONG master;
	CtrlMaster(pDev, NULL, BRDctrl_DAC_GETMASTER, &master);
	pSrvInfo->SyncMode = master;
	CDacSrv::GetClkSource(pDev, pSrvInfo->ClockSrc);
	CDacSrv::GetClkValue(pDev, pSrvInfo->ClockSrc, pSrvInfo->ClockValue);
	CDacSrv::GetRate(pDev, pSrvInfo->SamplingRate, pSrvInfo->ClockValue);
	if((pSrvInfo->ClockSrc != BRDclks_BASEGEN0) || (pSrvInfo->ClockSrc != BRDclks_BASEGEN1))
		pSrvInfo->BaseExtClk = pSrvInfo->ClockValue;
//	BRD_SyncMode sync_mode;
//	CtrlSyncMode(pDev, NULL, BRDctrl_DAC_GETSYNCMODE, &sync_mode);
//	pSrvInfo->ClockSrc = sync_mode.clkSrc;
//	pSrvInfo->ClockValue = sync_mode.clkValue;
//	pSrvInfo->SamplingRate = sync_mode.rate;
//	if((sync_mode.clkSrc != BRDclks_BASEGEN0) || (sync_mode.clkSrc != BRDclks_BASEGEN1))
//		pSrvInfo->BaseExtClk = sync_mode.clkValue;
	ULONG format;
	CtrlFormat(pDev, NULL, BRDctrl_DAC_GETFORMAT, &format);
	pSrvInfo->Format = (UCHAR)format;
	ULONG code;
	CtrlCode(pDev, NULL, BRDctrl_DAC_GETCODE, &code);
	pSrvInfo->Encoding = (UCHAR)code;

	//for(int i = 0; i < BRD_CHANCNT; i++)
	//{
	//	CDacSrv::GetOutRange(pDev, pSrvInfo->Range[i], i);
	//	CDacSrv::GetBias(pDev, pSrvInfo->Bias[i], i);
	//}

	BRD_StartMode start_mode;
	CDacSrv::GetStartMode(pDev, &start_mode);
	//CtrlStartMode(pDev, NULL, BRDctrl_DAC_GETSTARTMODE, &start_mode);
	pSrvInfo->StartSrc		= start_mode.startSrc;
	pSrvInfo->StartInv		= start_mode.startInv;
	pSrvInfo->StartStopMode = start_mode.trigOn;
	pSrvInfo->StopSrc		= start_mode.trigStopSrc;
	pSrvInfo->StopInv		= start_mode.stopInv;
	
	BRD_EnVal st_delay;
	CtrlStDelay(pDev, NULL, BRDctrl_DAC_GETSTDELAY, &st_delay);
	pSrvInfo->Cnt0Enable = st_delay.enable;
	pSrvInfo->StartDelayCnt = st_delay.value;

	BRD_EnVal out_data;
	CtrlOutData(pDev, NULL, BRDctrl_DAC_GETOUTDATA, &out_data);
	pSrvInfo->Cnt1Enable = out_data.enable;
	pSrvInfo->OutCnt = out_data.value;

	BRD_EnVal skip_data;
	CtrlSkipData(pDev, NULL, BRDctrl_DAC_GETSKIPDATA, &skip_data);
	pSrvInfo->Cnt2Enable = skip_data.enable;
	pSrvInfo->SkipCnt = skip_data.value;

	BRDCHAR module_name[40];
//	sprintf(module_name, " (%s)", pDev->m_name);
	BRDC_sprintf(module_name, _BRDC(" (%s)"), pDev->GetName());
//	PBASE_Info binfo = pModule->GetBaseInfo();
//	sprintf(module_name, " (%s)", binfo->boardName);
	BRDC_strcpy(pSrvInfo->Name, m_name);
	BRDC_strcat(pSrvInfo->Name, module_name);

	return pSrvInfo;
//	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetPropertyFromDialog(void	*pDev, void	*args)
{
	CModule* pModule = (CModule*)pDev;
	PDACSRV_INFO pInfo = (PDACSRV_INFO)args;
	CDacSrv::SetChanMask(pModule, pInfo->ChanMask);
//	CtrlChanMask(pDev, NULL, BRDctrl_DAC_SETCHANMASK, &pInfo->ChanMask);
	ULONG master = pInfo->SyncMode;
	CtrlMaster(pDev, NULL, BRDctrl_DAC_SETMASTER, &master);
//	ULONG chan_mask = pInfo->ChanMask;
//	CtrlChanMask(pDev, NULL, BRDctrl_DAC_SETCHANMASK, &chan_mask);

//	CDacSrv::SetClkSource(pModule, pInfo->ClockSrc);
//	CDacSrv::SetClkValue(pModule, pInfo->ClockSrc, pInfo->ClockValue);
//	CDacSrv::SetRate(pModule, pInfo->SamplingRate, pInfo->ClockSrc, pInfo->ClockValue);

	//BRD_SyncMode sync_mode;
	//sync_mode.clkSrc	 = pInfo->ClockSrc;
	//sync_mode.clkValue	 = pInfo->ClockValue;
	//sync_mode.rate		 = pInfo->SamplingRate;
	//CtrlSyncMode(pDev, NULL, BRDctrl_DAC_SETSYNCMODE, &sync_mode);
	ULONG format = pInfo->Format;
	CtrlFormat(pDev, NULL, BRDctrl_DAC_SETFORMAT, &format);
	ULONG code = pInfo->Encoding;
	CtrlCode(pDev, NULL, BRDctrl_DAC_SETCODE, &code);

	//for(int i = 0; i < BRD_CHANCNT; i++)
	//{
	//	CDacSrv::SetOutRange(pModule, pInfo->Range[i], i);
	//	CDacSrv::SetBias(pModule, pInfo->Bias[i], i);
	//}

	BRD_StartMode start_mode;
//	CtrlStartMode(pDev, NULL, BRDctrl_DAC_GETSTARTMODE, &start_mode);
	start_mode.startSrc = pInfo->StartSrc;
	start_mode.startInv = pInfo->StartInv;		
	start_mode.trigOn = pInfo->StartStopMode;
	start_mode.trigStopSrc = pInfo->StopSrc;		
	start_mode.stopInv = pInfo->StopInv;
	CDacSrv::SetStartMode(pModule, &start_mode);
//	CtrlStartMode(pDev, NULL, BRDctrl_DAC_SETSTARTMODE, &start_mode);

	BRD_EnVal st_delay;
	st_delay.enable	= pInfo->Cnt0Enable;
	st_delay.value	= pInfo->StartDelayCnt;
	CtrlStDelay(pDev, NULL, BRDctrl_DAC_SETSTDELAY, &st_delay);
	BRD_EnVal out_data;
	out_data.enable	= pInfo->Cnt1Enable;
	out_data.value	= pInfo->OutCnt;
	CtrlOutData(pDev, NULL, BRDctrl_DAC_SETOUTDATA, &out_data);
	BRD_EnVal skip_data;
	skip_data.enable = pInfo->Cnt2Enable;
	skip_data.value	 = pInfo->SkipCnt;
	CtrlSkipData(pDev, NULL, BRDctrl_DAC_SETSKIPDATA, &skip_data);

	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	BRDCHAR Buffer[256];
	BRDCHAR ParamName[128];
	BRDCHAR* endptr;

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("MasterMode"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("MasterMode"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG master = BRDC_atoi(Buffer);
	CtrlMaster(pDev, NULL, BRDctrl_DAC_SETMASTER, &master);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("ChannelMask"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("ChannelMask"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
//	ULONG chanMask = atoi(Buffer);
	ULONG chanMask = BRDC_strtol(Buffer, &endptr, 0);
	CDacSrv::SetChanMask(pDev, chanMask);

    //IPC_getPrivateProfileString(SectionName, "ClockSource", "1", Buffer, sizeof(Buffer), iniFilePath);
	//ULONG clkSrc = atoi(Buffer);
	//CDacSrv::SetClkSource(pDev, clkSrc);
    //IPC_getPrivateProfileString(SectionName, "BaseExternalClockValue", "100000.0", Buffer, sizeof(Buffer), iniFilePath);
	//double clkValue = atof(Buffer);
	//CDacSrv::SetClkValue(pDev, clkSrc, clkValue);
    //IPC_getPrivateProfileString(SectionName, "SamplingRate", "100000.0", Buffer, sizeof(Buffer), iniFilePath);
	//double rate = atof(Buffer);
	//CDacSrv::SetRate(pDev, rate, clkSrc, clkValue);
	//Sleep(1000);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("DataFormat"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("DataFormat"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG format = BRDC_atoi(Buffer);
	CtrlFormat(pDev, NULL, BRDctrl_DAC_SETFORMAT, &format);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("CodeType"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("CodeType"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG code = BRDC_atoi(Buffer);
	CtrlCode(pDev, NULL, BRDctrl_DAC_SETCODE, &code);

	//double OutRange[BRD_CHANCNT], Bias[BRD_CHANCNT];
	//for(int i = 0; i < BRD_CHANCNT; i++)
	//{
	//	sprintf(ParamName, "OutputRange%d", i);
    //	IPC_getPrivateProfileString(SectionName, ParamName, "0.5", Buffer, sizeof(Buffer), iniFilePath);
	//	OutRange[i] = atof(Buffer);
	//	CDacSrv::SetOutRange(pDev, InpRange[i], i);
	//	sprintf(ParamName, "Bias%d", i);
    //	IPC_getPrivateProfileString(SectionName, ParamName, "0.0", Buffer, sizeof(Buffer), iniFilePath);
	//	Bias[i] = atof(Buffer);
	//	CDacSrv::SetBias(pDev, Bias[i], i);
	//}

	BRD_StartMode start_mode;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StartBaseSource"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StartBaseSource"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_mode.startSrc = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StartBaseInverting"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StartBaseInverting"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_mode.startInv = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StartMode"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StartMode"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_mode.trigOn = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StopSource"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StopSource"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_mode.trigStopSrc = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StopInverting"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StopInverting"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_mode.stopInv = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("ReStart"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("ReStart"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_mode.reStartMode = BRDC_atoi(Buffer);
	CDacSrv::SetStartMode(pDev, &start_mode);

	BRD_EnVal start_delay;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StartDelayEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StartDelayEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_delay.enable = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StartDelayCounter"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StartDelayCounter"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	start_delay.value = BRDC_atoi(Buffer);
	CtrlStDelay(pDev, NULL, BRDctrl_DAC_SETSTDELAY, &start_delay);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("ClockSource"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("ClockSource"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
//	ULONG clkSrc = atoi(Buffer);
	ULONG clkSrc = BRDC_strtol(Buffer, &endptr, 0);
//	CDacSrv::SetClkSource(pDev, clkSrc);
	SetClkSource( pDev, clkSrc );
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("BaseExternalClockValue"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("BaseExternalClockValue"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	double clkValue = BRDC_atof(Buffer);
	CDacSrv::SetClkValue(pDev, clkSrc, clkValue);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("SamplingRate"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("SamplingRate"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	double rate = BRDC_atof(Buffer);
	CDacSrv::SetRate(pDev, rate, clkSrc, clkValue);

	BRD_StdDelay dac_delay;
	for(int i = 0; i < 16; i++)
	{
		BRDC_sprintf(ParamName, _BRDC("StdDelay%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
#endif
		if(BRDC_strlen(Buffer))
		{
			dac_delay.nodeID = i;
			dac_delay.value = BRDC_atoi(Buffer);
			StdDelay(pDev, 1, &dac_delay);
		}
	}

	//BRD_EnVal out_data;
    //IPC_getPrivateProfileString(SectionName, "OutputSampleEnable", "0", Buffer, sizeof(Buffer), iniFilePath);
	//out_data.enable = atoi(Buffer);
    //IPC_getPrivateProfileString(SectionName, "OutputSampleCounter", "0", Buffer, sizeof(Buffer), iniFilePath);
	//out_data.value = atoi(Buffer);
	//CtrlOutData(pDev, NULL, BRDctrl_DAC_SETOUTDATA, &out_data);

	//BRD_EnVal skip_data;
    //IPC_getPrivateProfileString(SectionName, "SkipSampleEnable", "0", Buffer, sizeof(Buffer), iniFilePath);
	//skip_data.enable = atoi(Buffer);
    //IPC_getPrivateProfileString(SectionName, "SkipSampleCounter", "0", Buffer, sizeof(Buffer), iniFilePath);
	//skip_data.value = atoi(Buffer);
	//CtrlSkipData(pDev, NULL, BRDctrl_DAC_SETSKIPDATA, &skip_data);

	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	//BRDCHAR Buffer[128];

	ULONG master;
	CtrlMaster(pDev, NULL, BRDctrl_DAC_GETMASTER, &master);
	//BRDC_sprintf(Buffer, _BRDC("%u"), master);
    //IPC_writePrivateProfileString(SectionName, _BRDC("MasterMode"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("MasterMode"), master, 10, NULL);

	ULONG chanMask;
	CDacSrv::GetChanMask(pDev, chanMask);
	//BRDC_sprintf(Buffer, _BRDC("%u"), chanMask);
    //IPC_writePrivateProfileString(SectionName, _BRDC("ChannelMask"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ChannelMask"), chanMask, 16, NULL);

	ULONG clkSrc;
	CDacSrv::GetClkSource(pDev, clkSrc);
	//BRDC_sprintf(Buffer, _BRDC("%u"), clkSrc);
    //IPC_writePrivateProfileString(SectionName, _BRDC("ClockSource"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ClockSource"), clkSrc, 16, NULL);
	double clkValue;
	CDacSrv::GetClkValue(pDev, clkSrc, clkValue);
	if((clkSrc != BRDclks_BASEGEN0) && (clkSrc != BRDclks_BASEGEN1))
	{
		//BRDC_sprintf(Buffer, _BRDC("%.2f"), clkValue);
        //IPC_writePrivateProfileString(SectionName, _BRDC("BaseExternalClockValue"), Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, _BRDC("BaseExternalClockValue"), clkValue, 2, NULL);
	}
	double rate;
	CDacSrv::GetRate(pDev, rate, clkValue);
	//BRDC_sprintf(Buffer, _BRDC("%.2f"), rate);
    //IPC_writePrivateProfileString(SectionName, _BRDC("SamplingRate"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SamplingRate"), rate, 2, NULL);

	ULONG format;
	CtrlFormat(pDev, NULL, BRDctrl_DAC_GETFORMAT, &format);
	//BRDC_sprintf(Buffer, _BRDC("%u"), format);
    //IPC_writePrivateProfileString(SectionName, _BRDC("DataFormat"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("DataFormat"), format, 10, NULL);

	ULONG code;
	CtrlCode(pDev, NULL, BRDctrl_DAC_GETCODE, &code);
	//BRDC_sprintf(Buffer, _BRDC("%u"), code);
    //IPC_writePrivateProfileString(SectionName, _BRDC("CodeType"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("CodeType"), code, 10, NULL);

	//double OutRange[BRD_CHANCNT], Bias[BRD_CHANCNT];
	//for(int i = 0; i < BRD_CHANCNT; i++)
	//{
	//	CDacSrv::GetOutRange(pDev, OutRange[i], i);
	//	sprintf(Buffer, "%.2f", OutRange[i]);
	//	sprintf(ParamName, "OutputRange%d", i);
    //	IPC_writePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
	//	CDacSrv::GetBias(pDev, Bias[i], i);
	//	sprintf(Buffer, "%.4f", Bias[i]);
	//	sprintf(ParamName, "Bias%d", i);
    //	IPC_writePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
	//}

	BRD_StartMode start_mode;
	CDacSrv::GetStartMode(pDev, &start_mode);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_mode.startSrc);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StartBaseSource"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartBaseSource"), (ULONG)start_mode.startSrc, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_mode.startInv);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StartBaseInverting"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartBaseInverting"), (ULONG)start_mode.startInv, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_mode.trigOn);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StartMode"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartMode"), (ULONG)start_mode.trigOn, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_mode.trigStopSrc);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StopSource"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StopSource"), (ULONG)start_mode.trigStopSrc, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_mode.stopInv);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StopInverting"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StopInverting"), (ULONG)start_mode.stopInv, 10, NULL);

	BRD_EnVal start_delay;
	CtrlStDelay(pDev, NULL, BRDctrl_DAC_GETSTDELAY, &start_delay);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_delay.enable);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StartDelayEnable"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartDelayEnable"), (ULONG)start_delay.enable, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), start_delay.value);
    //IPC_writePrivateProfileString(SectionName, _BRDC("StartDelayCounter"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartDelayCounter"), (ULONG)start_delay.value, 10, NULL);

	BRD_EnVal out_data;
	CtrlOutData(pDev, NULL, BRDctrl_DAC_GETOUTDATA, &out_data);
	//BRDC_sprintf(Buffer, _BRDC("%u"), out_data.enable);
    //IPC_writePrivateProfileString(SectionName, _BRDC("OutputSampleEnable"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("OutputSampleEnable"), (ULONG)out_data.enable, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), out_data.value);
    //IPC_writePrivateProfileString(SectionName, _BRDC("OutputSampleCounter"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("OutputSampleCounter"), (ULONG)out_data.value, 10, NULL);

	BRD_EnVal skip_data;
	CtrlSkipData(pDev, NULL, BRDctrl_DAC_GETSKIPDATA, &skip_data);
	//BRDC_sprintf(Buffer, _BRDC("%u"), skip_data.enable);
    //IPC_writePrivateProfileString(SectionName, _BRDC("SkipSampleEnable"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SkipSampleEnable"), (ULONG)skip_data.enable, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u"), skip_data.value);
    //IPC_writePrivateProfileString(SectionName, _BRDC("SkipSampleCounter"), Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SkipSampleCounter"), (ULONG)skip_data.value, 10, NULL);

	// the function flushes the cache
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_writePrivateProfileString(NULL, NULL, NULL, iniFilePath);
#else
	WritePrivateProfileString(NULL, NULL, NULL, iniFilePath);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
//int CDacSrv::GetCfg(PBRD_DacCfg pCfg)
int CDacSrv::GetCfg(PVOID pCfg)
{
	PDACSRV_CFG pSrvCfg = (PDACSRV_CFG)m_pConfig;
	PBRD_DacCfg pDacCfg = (PBRD_DacCfg)pCfg;
	pDacCfg->Bits = pSrvCfg->Bits;
	pDacCfg->Encoding = pSrvCfg->Encoding;
	pDacCfg->FifoSize = pSrvCfg->FifoSize;
	pDacCfg->OutRange = pSrvCfg->OutRange;
	pDacCfg->MinRate = pSrvCfg->MinRate;
	pDacCfg->MaxRate = pSrvCfg->MaxRate;
	pDacCfg->MaxChan = pSrvCfg->MaxChan;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetChanMask(CModule* pModule, ULONG mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	pChan1->ByBits.ChanSel = mask;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetChanMask(CModule* pModule, ULONG& mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	mask = pChan1->ByBits.ChanSel;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetCode(CModule* pModule, ULONG type)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	pFormat->ByBits.Code = type;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetCode(CModule* pModule, ULONG& type)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	type = pFormat->ByBits.Code;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetClkSource(CModule* pModule, ULONG& ClkSrc)
{
	ULONG source;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		source = param.val;
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			source = param.val;
		}
		else
		{ // Slave
			source = BRDclks_EXTSYNX;
		}
	}
	ClkSrc = source;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	double clkval = 0.0;
	switch(ClkSrc)
	{
	case BRDclks_SYSGEN:		// System Generator
		clkval = pDacSrvCfg->SysRefGen;
		break;
	case BRDclks_BASEGEN0:		// Generator 0
		clkval = pDacSrvCfg->BaseRefGen[0];
		break;
	case BRDclks_BASEGEN1:		// Generator 1
		clkval = pDacSrvCfg->BaseRefGen[1];
		break;
	case BRDclks_EXTSDX:		// External clock
	case BRDclks_EXTSYNX:		// Master clock
	case BRDclks_INFREQ:		// 
	case BRDclks_SMOUTFREQ:	//
		clkval = pDacSrvCfg->BaseExtClk;
		break;
	case BRDclks_SMCLK:		// 
		// РїРѕР»СѓС‡РёС‚СЊ С‡Р°СЃС‚РѕС‚Сѓ РѕС‚ РґСЂСѓРіРёС… СЃР»СѓР¶Р±
//		clkval = pDacSrvCfg->RefGen[1];
		break;
	case BRDclks_BASEDDS:
		clkval = pDacSrvCfg->BaseExtClk;
		break;
	//case BRDclks_DAC_DISABLED:		// disabled clock
	//	clkval = 0.0;
	//	break;
	//case BRDclks_DAC_SUBGEN:		// Submodule generator
	//	clkval = pDacSrvCfg->SubRefGen;
	//	break;
	//case BRDclks_DAC_EXTCLK:		// External clock
	//	clkval = pDacSrvCfg->SubExtClk;
	//	break;
	default:
		clkval = 0.0;
		break;
	}
	ClkValue = clkval;
	return BRDerr_OK;
}
	
//***************************************************************************************
int CDacSrv::SetClkSource(CModule* pModule, ULONG ClkSrc)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FMODE;
		param.val = ClkSrc;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FMODE;
			param.val = ClkSrc;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		else
			return BRDerr_NOT_ACTION; // С„СѓРЅРєС†РёСЏ РІ СЂРµР¶РёРјРµ Slave РЅРµ РІС‹РїРѕР»РЅРёРјР°
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PDACSRV_CFG pDacSrvCfg = (PDACSRV_CFG)m_pConfig;
	switch(ClkSrc)
	{
	case BRDclks_SYSGEN:		// System Generator
		ClkValue = pDacSrvCfg->SysRefGen;
		break;
	case BRDclks_BASEGEN0:		// Generator 0
		ClkValue = pDacSrvCfg->BaseRefGen[0];
		break;
	case BRDclks_BASEGEN1:		// Generator 1
		ClkValue = pDacSrvCfg->BaseRefGen[1];
		break;
	case BRDclks_EXTSDX:		// External clock from SDX
	case BRDclks_EXTSYNX:	// External clock from SYNX
//	case BRDclks_SMCLK:		// 
	case BRDclks_BASEDDS: // External for tetrad, but internal for board
	case BRDclks_INFREQ:		// Sinchro with input
	case BRDclks_SMOUTFREQ:	// Sinchro with output
		pDacSrvCfg->BaseExtClk = ROUND(ClkValue);
		break;

	//case BRDclks_DAC_DISABLED:		// disabled clock
	//	ClkValue = 0.0;
	//	break;
	//case BRDclks_DAC_SUBGEN:		// Submodule generator
	//	ClkValue = pDacSrvCfg->SubRefGen;
	//	break;
	//case BRDclks_DAC_EXTCLK:		// External clock
	//	pDacSrvCfg->SubExtClk = ROUND(ClkValue);
	//	break;
	default:
//		ClkValue = 0.0;
		break;
	}
	return BRDerr_OK;
}
	
//***************************************************************************************
int CDacSrv::SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue)
{
	ULONG status = BRDerr_OK;
	ULONG DacRateDivider = ROUND(ClkValue / Rate);
	DacRateDivider = DacRateDivider ? DacRateDivider : 1;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FDIV;
		param.val = DacRateDivider;
		status = pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		status = pModule->RegCtrl(DEVScmd_REGREADIND, &param);
        if(status == BRDerr_OK) {
			if(pMode0->ByBits.Master)
			{ // Master
				param.reg = ADM2IFnr_FDIV;
				param.val = DacRateDivider;
				status = pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			}
        }
		else
		{
			//param.tetr = m_DacTetrNum;
			//param.reg = ADM2IFnr_FDIV;
			//param.val = DacRateDivider;
			//status = pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			return BRDerr_NOT_ACTION; // С„СѓРЅРєС†РёСЏ РІ СЂРµР¶РёРјРµ Slave РЅРµ РІС‹РїРѕР»РЅРёРјР°
		}
	}
	if(status == BRDerr_OK)
		Rate = ClkValue / DacRateDivider;
	return status;
//	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetRate(CModule* pModule, double& Rate, double ClkValue)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FDIV;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FDIV;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		}
		else
			return BRDerr_NOT_ACTION; // С„СѓРЅРєС†РёСЏ РІ СЂРµР¶РёРјРµ Slave РЅРµ РІС‹РїРѕР»РЅРёРјР°
	}
	Rate = ClkValue / param.val;
	return BRDerr_OK;
}
//***************************************************************************************
int CDacSrv::SetDivClk(CModule* pModule, ULONG& DivClk, double& rate)
{

	return BRDerr_OK;

}
//***************************************************************************************
int CDacSrv::GetDivClk(CModule* pModule, ULONG& DivClk, double& rate)
{

	return BRDerr_OK;
}
//***************************************************************************************

//***************************************************************************************
int CDacSrv::SetBias(CModule* pModule, double& Bias, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	return Status;
}

//***************************************************************************************
int CDacSrv::GetBias(CModule* pModule, double& Bias, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Bias = 0.0;
	return Status;
}

//***************************************************************************************
int CDacSrv::SetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStMode->ByBits.SrcStart  = pStartMode->startSrc;
		pStMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
		pStMode->ByBits.InvStart  = pStartMode->startInv;
		pStMode->ByBits.InvStop   = pStartMode->stopInv;
		pStMode->ByBits.TrigStart = pStartMode->trigOn;
		pStMode->ByBits.Restart =   pStartMode->reStartMode;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.tetr = m_MainTetrNum;
		param.reg = ADM2IFnr_FMODE;
		param.val = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_STMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
			pStMode->ByBits.SrcStart  = pStartMode->startSrc;
			pStMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
			pStMode->ByBits.InvStart  = pStartMode->startInv;
			pStMode->ByBits.InvStop   = pStartMode->stopInv;
			pStMode->ByBits.TrigStart = pStartMode->trigOn;
			pStMode->ByBits.Restart =   pStartMode->reStartMode;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

			param.reg = ADM2IFnr_FMODE;
			param.val = 7;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		else
			return BRDerr_NOT_ACTION; // С„СѓРЅРєС†РёСЏ РІ СЂРµР¶РёРјРµ Slave РЅРµ РІС‹РїРѕР»РЅРёРјР°
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStartMode->startSrc	= pStMode->ByBits.SrcStart;
		pStartMode->trigStopSrc	= pStMode->ByBits.SrcStop;
		pStartMode->startInv	= pStMode->ByBits.InvStart;
		pStartMode->stopInv		= pStMode->ByBits.InvStop;
		pStartMode->trigOn		= pStMode->ByBits.TrigStart;
		pStartMode->reStartMode	= pStMode->ByBits.Restart;
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_STMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
			pStartMode->startSrc	= pStMode->ByBits.SrcStart;
			pStartMode->trigStopSrc	= pStMode->ByBits.SrcStop;
			pStartMode->startInv	= pStMode->ByBits.InvStart;
			pStartMode->stopInv		= pStMode->ByBits.InvStop;
			pStartMode->trigOn		= pStMode->ByBits.TrigStart;
			pStartMode->reStartMode	= pStMode->ByBits.Restart;
		}
		else
			return BRDerr_NOT_ACTION; // С„СѓРЅРєС†РёСЏ РІ СЂРµР¶РёРјРµ Slave РЅРµ РІС‹РїРѕР»РЅРёРјР°
//		{ // Slave
//			source = BRD_clksEXTSYNX;
//		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetGain(CModule* pModule, double& Gain, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDacSrv::GetGain(CModule* pModule, double& Gain, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Gain = 1.0;
	return Status;
//	double value = 1.0;
//	return value;
}

//***************************************************************************************
int CDacSrv::SetOutRange(CModule* pModule, double& OutRange, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDacSrv::GetOutRange(CModule* pModule, double& OutRange, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	PDACSRV_CFG pSrvCfg = (PDACSRV_CFG)m_pConfig;
	OutRange = pSrvCfg->OutRange / 1000.;
//	double value = (double)pSrvCfg->InpRange;
//	return value;
	return Status;
}

//***************************************************************************************
int CDacSrv::SetTestMode(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Test = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetTestMode(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	mode = pMode1->ByBits.Test;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::StdDelay(CModule* pModule, int cmd, PBRD_StdDelay delay)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_TRES;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	if(param.val & 0x800)
	{
		param.reg = ADM2IFnr_STDDELAY;
		STD_DELAY* pDacDelay = (STD_DELAY*)&param.val;
		pDacDelay->ByBits.Delay = 0x3f;
		pDacDelay->ByBits.ID = delay->nodeID;
		pDacDelay->ByBits.Reset = 0;
		pDacDelay->ByBits.Write = 0;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pDacDelay->ByBits.Used)
		{
			if(cmd == 1)
			{ // write
				pDacDelay->ByBits.Write = 1;
				pDacDelay->ByBits.Reset = 0;
				pDacDelay->ByBits.Delay = delay->value;
			}
			if(cmd == 2)
			{ // reset
				pDacDelay->ByBits.Write = 0;
				pDacDelay->ByBits.Reset = 1;
			}
			if(cmd > 0)
			{ // write || reset
				pDacDelay->ByBits.ID = delay->nodeID;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				pDacDelay->ByBits.Reset = 0;
				pDacDelay->ByBits.Write = 0;
				pDacDelay->ByBits.ID = delay->nodeID;
				pDacDelay->ByBits.Delay = 0x3f;
				pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			}
			delay->value = pDacDelay->ByBits.Delay;
		}
		else
			return BRDerr_BAD_NODEID;
	}
	else
		return BRDerr_CMD_UNSUPPORTED;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetSpecific(CModule* pModule, PBRD_DacSpec pSpec)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDacSrv::GetSpecific(CModule* pModule, PBRD_DacSpec pSpec)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	mode = 0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDacSrv::SetResist(CModule * pModule, ULONG nInpRes, ULONG nChan)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CDacSrv::GetResist(CModule * pModule, ULONG &nInpRes, ULONG nChan)
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CDacSrv::SetSource(CModule* pModule, ULONG source)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Out = source;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetSource(CModule* pModule, ULONG& target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DacTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	target = pMode1->ByBits.Out;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SelfClbr(CModule* pModule)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDacSrv::SetClkRefSource(CModule* pModule, ULONG ClkSrc,ULONG RefSrc)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetClkRefSource(CModule* pModule, ULONG& ClkSrc,ULONG& RefSrc)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::SetClkRefValue(CModule* pModule, ULONG ClkSrc,ULONG RefSrc, double& ClkValue,double& RefValue)
{
	return BRDerr_OK;

}
//************************************************************************************************
int CDacSrv::GetClkRefValue(CModule* pModule, ULONG ClkSrc,ULONG RefSrc, double& ClkValue,double& RefValue)
{
	return BRDerr_OK;

}
	


//***************************************************************************************
S32		CDacSrv::IndRegRead( CModule* pModule, S32 tetrNo, S32 regNo, U32 *pVal )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	*pVal = param.val;

	return BRDerr_OK;
}

//***************************************************************************************
/*
S32		CDacSrv::IndRegRead( CModule* pModule, S32 tetrNo, S32 regNo, ULONG *pVal )
{
	return IndRegRead( pModule, tetrNo, regNo, (U32*)pVal );
}
*/
//***************************************************************************************
S32		CDacSrv::IndRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, U32 val )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	param.val = val;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
/*
S32		CDacSrv::IndRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, ULONG val )
{
	return IndRegWrite( pModule, tetrNo, regNo, (U32)val );
}
*/

//***************************************************************************************
S32		CDacSrv::DirRegRead( CModule* pModule, S32 tetrNo, S32 regNo, U32 *pVal )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	*pVal = param.val;

	return BRDerr_OK;
}

//***************************************************************************************
/*
S32		CDacSrv::DirRegRead( CModule* pModule, S32 tetrNo, S32 regNo, ULONG *pVal )
{
	return DirRegRead( pModule, tetrNo, regNo, (U32*)pVal );
}
*/
//***************************************************************************************
S32		CDacSrv::DirRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, U32 val )
{
	DEVS_CMD_Reg	param;

	param.idxMain = m_index;
	param.tetr = tetrNo;
	param.reg = regNo;
	param.val = val;
	pModule->RegCtrl(DEVScmd_REGWRITEDIR, &param);

	return BRDerr_OK;
}

//***************************************************************************************
/*
S32		CDacSrv::DirRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, ULONG val )
{
	return DirRegWrite( pModule, tetrNo, regNo, (U32)val );
}
*/

//***************************************************************************************
int CDacSrv::SetInterpFactor(CModule* pModule, ULONG InterpFactor )
{
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::GetInterpFactor(CModule* pModule, ULONG &InterpFactor )
{
	InterpFactor = 1;
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::DacEnable(CModule* pModule, ULONG enable)
{
	//
	// Ekk добавил 10.08.2016 (переделал из CtrlEnable() )
	//
	U32			val;

	if(enable)
	{
		PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&val;

		for(int i = 0; i < 20; i++)
		{
			DirRegRead( pModule, m_DacTetrNum, ADM2IFnr_STATUS, &val );
			if(pStatus->ByBits.Empty)
				break;
			#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
			    IPC_delay(1);
			#else
				Sleep(1);
			#endif
		}
	}

	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&val;

	IndRegRead( pModule, m_DacTetrNum, ADM2IFnr_MODE0, &val );
	pMode0->ByBits.Start = enable;
	IndRegWrite( pModule, m_DacTetrNum, ADM2IFnr_MODE0, val );

	if(!pMode0->ByBits.Master)
	{ 
		// Master/Slave
		IndRegRead( pModule, m_MainTetrNum, ADM2IFnr_MODE0, &val );
		if(pMode0->ByBits.Master)
		{ 
			// Master
			if(enable)
			{
				IndRegRead( pModule, m_MainTetrNum, ADM2IFnr_MODE0, &val );
			}
			pMode0->ByBits.Start = enable;
			IndRegWrite( pModule, m_MainTetrNum, ADM2IFnr_MODE0, val );
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDacSrv::PrepareStart(CModule* pModule, void *arg)
{
	return BRDerr_OK;
}

// ***************** End of file DacSrv.cpp ***********************
