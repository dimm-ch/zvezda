/****************** File AdcSrv.cpp ************
 *
 * BRD Driver for ADС service on ADC submodule
 *
 * (C) InSys by Dorokhin A. Apr 2004
 *
 ***********************************************/

#include "module.h"
#include "adcsrv.h"

#define	CURRFILE "ADCSRV"

//***************************************************************************************
//int CAdcSrv::GetFifoParam(CModule* pModule, USHORT& FifoDeep, USHORT& FifoWidth)
//{
//	DEVS_CMD_Reg param;
//	param.idxMain = m_index;
//	param.tetr = m_AdcTetrNum;
//	param.reg = ADM2IFnr_FTYPE;
//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//	FifoWidth = param.val >> 3; // ширина FIFO (в байтах)
//	param.reg = ADM2IFnr_FSIZE;
//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//	FifoDeep = param.val;
//	return BRDerr_OK;
//}

//***************************************************************************************
void CAdcSrv::GetAdcTetrNum(CModule* pModule)
{
	m_AdcTetrNum = 0;
}

//***************************************************************************************
void CAdcSrv::GetAdcTetrNumEx(CModule* pModule, ULONG TetrInstNum)
{
	if(TetrInstNum == 1)
		GetAdcTetrNum(pModule);
	else
		m_AdcTetrNum = -1;
}

//***************************************************************************************
int CAdcSrv::GetTraceText(CModule* pModule, U32 traceId, U32 sizeb, BRDCHAR *pText)
{
	pText[0] = '\0';

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetTraceParam(CModule* pModule, U32 traceId, U32 sizeb, U32 *pRealSizeb, void *pParam )
{
	*pRealSizeb = 0;

	return BRDerr_OK;
}

//***************************************************************************************
void CAdcSrv::FreeInfoForDialog(PVOID pInfo)
{
	PADCSRV_INFO pSrvInfo = (PADCSRV_INFO)pInfo;
	delete pSrvInfo;
}

//***************************************************************************************
PVOID CAdcSrv::GetInfoForDialog(CModule* pDev)
{
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	PADCSRV_INFO pSrvInfo = new ADCSRV_INFO;
	pSrvInfo->Size = sizeof(ADCSRV_INFO);
	pSrvInfo->Bits = pSrvCfg->Bits;
	pSrvInfo->Encoding = pSrvCfg->Encoding;
	pSrvInfo->FifoSize = pSrvCfg->FifoSize;
//	GetFifoParam(pDev, pSrvInfo->FifoDeep, pSrvInfo->FifoWidth);
	pSrvInfo->ChanCnt = pSrvCfg->MaxChan;
	pSrvInfo->MinRate = pSrvCfg->MinRate;
	pSrvInfo->MaxRate = pSrvCfg->MaxRate;
	pSrvInfo->RefPVS = pSrvCfg->RefPVS;
	pSrvInfo->SysRefGen = pSrvCfg->SysRefGen;
	pSrvInfo->BaseRefGen[0] = pSrvCfg->BaseRefGen[0];
	pSrvInfo->BaseRefGen[1] = pSrvCfg->BaseRefGen[1];
	pSrvInfo->BaseExtClk = pSrvCfg->BaseExtClk;
	CAdcSrv::GetChanMask(pDev, pSrvInfo->ChanMask);
//	ULONG chan_mask;
//	CtrlChanMask(pDev, NULL, BRDctrl_ADC_GETCHANMASK, &chan_mask);
//	pSrvInfo->ChanMask = chan_mask;
	ULONG master;
	GetMaster(pDev, master);
	pSrvInfo->SyncMode = master;
//	CtrlMaster(pDev, NULL, BRDctrl_ADC_GETMASTER, &master);
	CAdcSrv::GetClkSource(pDev, pSrvInfo->ClockSrc);
	CAdcSrv::GetClkValue(pDev, pSrvInfo->ClockSrc, pSrvInfo->ClockValue);
	CAdcSrv::GetRate(pDev, pSrvInfo->SamplingRate, pSrvInfo->ClockValue);
	if((pSrvInfo->ClockSrc != BRDclks_BASEGEN0) && 
		(pSrvInfo->ClockSrc != BRDclks_BASEGEN1) && 
		(pSrvInfo->ClockSrc != BRDclks_SYSGEN))
			pSrvInfo->BaseExtClk = pSrvInfo->ClockValue;
//	BRD_SyncMode sync_mode;
//	CtrlSyncMode(pDev, NULL, BRDctrl_ADC_GETSYNCMODE, &sync_mode);
//	pSrvInfo->ClockSrc = sync_mode.clkSrc;
//	pSrvInfo->ClockValue = sync_mode.clkValue;
//	pSrvInfo->SamplingRate = sync_mode.rate;
//	if((sync_mode.clkSrc != BRDclks_BASEGEN0) || (sync_mode.clkSrc != BRDclks_BASEGEN1))
//		pSrvInfo->BaseExtClk = sync_mode.clkValue;
	ULONG format;
	CtrlFormat(pDev, NULL, BRDctrl_ADC_GETFORMAT, &format);
	pSrvInfo->Format = (UCHAR)format;
	ULONG code;
	CtrlCode(pDev, NULL, BRDctrl_ADC_GETCODE, &code);
	pSrvInfo->Encoding = (UCHAR)code;
//	BRD_ValChan range;
//	BRD_ValChan bias;
//	BRD_ValChan resist;
//	BRD_ValChan dccouple;
//	for(int i = 0; i < BRD_CHANCNT; i++)
//	for(int i = 0; i < pSrvCfg->MaxChan; i++)
	for(int i = 0; i < MAX_CHAN; i++)
	{
		CAdcSrv::GetInpRange(pDev, pSrvInfo->Range[i], i);
		CAdcSrv::GetBias(pDev, pSrvInfo->Bias[i], i);
		ULONG resist;
		CAdcSrv::GetInpResist(pDev, resist, i);
		pSrvInfo->Resist[i] = (UCHAR)resist;
		ULONG dc_coupling;
		CAdcSrv::GetDcCoupling(pDev, dc_coupling, i);
		pSrvInfo->DcCoupling[i] = (UCHAR)dc_coupling;
//		range.chan = bias.chan = resist.chan = dccouple.chan = i;
//		CtrlInpRange(pDev, NULL, BRDctrl_ADC_GETINPRANGE, &range);
//		CtrlBias(pDev, NULL, BRDctrl_ADC_GETBIAS, &bias);
//		CtrlInpResist(pDev, NULL, BRDctrl_ADC_GETINPRESIST, &resist);
//		CtrlDcCoupling(pDev, NULL, BRDctrl_ADC_GETDCCOUPLING, &dccouple);
//		pSrvInfo->Range[i] = range.value;
//		pSrvInfo->Bias[i] = bias.value;
//		pSrvInfo->Resist[i] = ROUND(resist.value);
//		pSrvInfo->DcCoupling[i] = ROUND(dccouple.value);
	}

	BRD_StartMode start_mode;
	CAdcSrv::GetStartMode(pDev, &start_mode);
	//CtrlStartMode(pDev, NULL, BRDctrl_ADC_GETSTARTMODE, &start_mode);
	pSrvInfo->StartSrc		= start_mode.startSrc;
	pSrvInfo->StartInv		= start_mode.startInv;
	pSrvInfo->StartStopMode = start_mode.trigOn;
	pSrvInfo->StopSrc		= start_mode.trigStopSrc;
	pSrvInfo->StopInv		= start_mode.stopInv;
	pSrvInfo->ReStart		= start_mode.reStartMode;
	
	//BRD_PretrigMode pretrig_mode;
	//CtrlGetPretrigMode(pDev, NULL, NULL, &pretrig_mode);
	//pSrvInfo->PretrigEnable = pretrig_mode.enable;
	//pSrvInfo->PretrigExernal = pretrig_mode.external;
	//pSrvInfo->PretrigAssur = pretrig_mode.assur;

	BRD_EnVal st_delay;
	GetCnt(pDev, 0, &st_delay);
	pSrvInfo->Cnt0Enable = st_delay.enable;
	pSrvInfo->StartDelayCnt = st_delay.value;

	BRD_EnVal acq_data;
	GetCnt(pDev, 1, &acq_data);
	pSrvInfo->Cnt1Enable = acq_data.enable;
	pSrvInfo->DaqCnt = acq_data.value;

	BRD_EnVal skip_data;
	GetCnt(pDev, 2, &skip_data);
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
int CAdcSrv::SetPropertyFromDialog(void	*pDev, void	*args)
{
	CModule* pModule = (CModule*)pDev;
	PADCSRV_INFO pInfo = (PADCSRV_INFO)args;
	//CAdcSrv::SetChanMask(pModule, pInfo->ChanMask);
	SetChanMask(pModule, pInfo->ChanMask);
//	CtrlChanMask(pDev, NULL, BRDctrl_ADC_SETCHANMASK, &pInfo->ChanMask);
	ULONG master = pInfo->SyncMode;
	CAdcSrv::SetMaster(pModule, master);
//	CtrlMaster(pDev, NULL, BRDctrl_ADC_SETMASTER, &master);
//	ULONG chan_mask = pInfo->ChanMask;
//	CtrlChanMask(pDev, NULL, BRDctrl_ADC_SETCHANMASK, &chan_mask);
	CAdcSrv::SetClkSource(pModule, pInfo->ClockSrc);
	CAdcSrv::SetClkValue(pModule, pInfo->ClockSrc, pInfo->ClockValue);
	CAdcSrv::SetRate(pModule, pInfo->SamplingRate, pInfo->ClockSrc, pInfo->ClockValue);
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	pSrvCfg->BaseExtClk = ROUND(pInfo->BaseExtClk);
	//BRD_SyncMode sync_mode;
	//sync_mode.clkSrc	 = pInfo->ClockSrc;
	//sync_mode.clkValue	 = pInfo->ClockValue;
	//sync_mode.rate		 = pInfo->SamplingRate;
	//CtrlSyncMode(pDev, NULL, BRDctrl_ADC_SETSYNCMODE, &sync_mode);
	ULONG format = pInfo->Format;
	CtrlFormat(pDev, NULL, BRDctrl_ADC_SETFORMAT, &format);
	ULONG code = pInfo->Encoding;
	CtrlCode(pDev, NULL, BRDctrl_ADC_SETCODE, &code);
//	for(int i = 0; i < BRD_CHANCNT; i++)
//	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
//	for(int i = 0; i < pSrvCfg->MaxChan; i++)
	for(int i = 0; i < MAX_CHAN; i++)
	{
		CAdcSrv::SetInpRange(pModule, pInfo->Range[i], i);
		CAdcSrv::SetBias(pModule, pInfo->Bias[i], i);
		CAdcSrv::SetInpResist(pModule, pInfo->Resist[i], i);
		CAdcSrv::SetDcCoupling(pModule, pInfo->DcCoupling[i], i);
	}
	//BRD_ValChan range;
	//BRD_ValChan bias;
	//BRD_ValChan resist;
	//BRD_ValChan dccouple;
	//for(int i = 0; i < BRD_CHANCNT; i++)
	//{
	//	range.chan = bias.chan = resist.chan = dccouple.chan = i;
	//	range.value = pInfo->Range[i];
	//	bias.value = pInfo->Bias[i];
	//	resist.value = pInfo->Resist[i];
	//	dccouple.value = pInfo->DcCoupling[i];
	//	CtrlInpRange(pDev, NULL, BRDctrl_ADC_SETINPRANGE, &range);
	//	CtrlBias(pDev, NULL, BRDctrl_ADC_SETBIAS, &bias);
	//	CtrlInpResist(pDev, NULL, BRDctrl_ADC_SETINPRESIST, &resist);
	//	CtrlDcCoupling(pDev, NULL, BRDctrl_ADC_SETDCCOUPLING, &dccouple);
	//}
	BRD_StartMode start_mode;
//	CtrlStartMode(pDev, NULL, BRDctrl_ADC_GETSTARTMODE, &start_mode);
	start_mode.startSrc = pInfo->StartSrc;
	start_mode.startInv = pInfo->StartInv;		
	start_mode.trigOn = pInfo->StartStopMode;
	start_mode.trigStopSrc = pInfo->StopSrc;		
	start_mode.stopInv = pInfo->StopInv;
	start_mode.reStartMode = pInfo->ReStart;
	CAdcSrv::SetStartMode(pModule, &start_mode);
//	CtrlStartMode(pDev, NULL, BRDctrl_ADC_SETSTARTMODE, &start_mode);

	//BRD_PretrigMode pretrig_mode;
	//pretrig_mode.enable = pInfo->PretrigEnable;
	//pretrig_mode.external = pInfo->PretrigExernal;
	//pretrig_mode.assur = pInfo->PretrigAssur;
	//CtrlSetPretrigMode(pDev, NULL, NULL, &pretrig_mode);

	BRD_EnVal st_delay;
	st_delay.enable	= pInfo->Cnt0Enable;
	st_delay.value	= pInfo->StartDelayCnt;
	SetCnt(pModule, 0, &st_delay);
	pInfo->StartDelayCnt = st_delay.value;
	BRD_EnVal acq_data;
	acq_data.enable	= pInfo->Cnt1Enable;
	acq_data.value	= pInfo->DaqCnt;
	SetCnt(pModule, 1, &st_delay);
	pInfo->DaqCnt = acq_data.value;
	BRD_EnVal skip_data;
	skip_data.enable = pInfo->Cnt2Enable;
	skip_data.value	 = pInfo->SkipCnt;
	SetCnt(pModule, 2, &st_delay);
	pInfo->SkipCnt = skip_data.value;

	return BRDerr_OK;
}

//***************************************************************************************
// read parameters from ini-file & set it
int CAdcSrv::SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
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
	SetMaster(pDev, master);
//	CtrlMaster(pDev, NULL, BRDctrl_ADC_SETMASTER, &master);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("ChannelMask"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("ChannelMask"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG chanMask = BRDC_strtol(Buffer, &endptr, 0);//strtol
	CAdcSrv::SetChanMask(pDev, chanMask);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("ClockSource"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("ClockSource"), _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
//	ULONG clkSrc = atoi(Buffer);
	ULONG clkSrc = BRDC_strtol(Buffer, &endptr, 0);//strtol
//	CAdcSrv::SetClkSource(pDev, clkSrc);
	SetClkSource(pDev, clkSrc);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("BaseExternalClockValue"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("BaseExternalClockValue"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	double clkValue = BRDC_atof(Buffer); // atof
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	pSrvCfg->BaseExtClk = ROUND(clkValue);
	CAdcSrv::SetClkValue(pDev, clkSrc, clkValue);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("SamplingRate"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("SamplingRate"), _BRDC("100000.0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	double rate = BRDC_atof(Buffer);
	CAdcSrv::SetRate(pDev, rate, clkSrc, clkValue);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("DataFormat"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("DataFormat"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG format = BRDC_strtol(Buffer, &endptr, 0);
	CtrlFormat(pDev, NULL, BRDctrl_ADC_SETFORMAT, &format);

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("CodeType"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("CodeType"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG code = BRDC_atoi(Buffer);
	CtrlCode(pDev, NULL, BRDctrl_ADC_SETCODE, &code);

	double InpRange[MAX_CHAN], Bias[MAX_CHAN];
	ULONG InpResist[MAX_CHAN], DcCoupling[MAX_CHAN], InpFilter[MAX_CHAN];
	for(int i = 0; i < MAX_CHAN; i++)
	{
		BRDC_sprintf(ParamName, _BRDC("InputRange%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("0.5"), Buffer, sizeof(Buffer), iniFilePath);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("0.5"), Buffer, sizeof(Buffer), iniFilePath);
#endif
		InpRange[i] = BRDC_atof(Buffer); //atof
		CAdcSrv::SetInpRange(pDev, InpRange[i], i);
		BRDC_sprintf(ParamName, _BRDC("Bias%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("0.0"), Buffer, sizeof(Buffer), iniFilePath);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("0.0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
		Bias[i] = BRDC_atof(Buffer); //atof
		CAdcSrv::SetBias(pDev, Bias[i], i);
		BRDC_sprintf(ParamName, _BRDC("InputResistance%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
		InpResist[i] = BRDC_atoi(Buffer);
		CAdcSrv::SetInpResist(pDev, InpResist[i], i);
		BRDC_sprintf(ParamName, _BRDC("DcCoupling%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
		DcCoupling[i] = BRDC_atoi(Buffer);
		CAdcSrv::SetDcCoupling(pDev, DcCoupling[i], i);
		BRDC_sprintf(ParamName, _BRDC("InpFilter%d"), i);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
		IPC_getPrivateProfileString(SectionName, ParamName, _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#else
		GetPrivateProfileString(SectionName, ParamName, _BRDC("1"), Buffer, sizeof(Buffer), iniFilePath);
#endif
		InpFilter[i] = BRDC_atoi(Buffer);
		CAdcSrv::SetInpFilter(pDev, InpFilter[i], i);
	}

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
	CAdcSrv::SetStartMode(pDev, &start_mode);

	ULONG	startSlave;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("StartSlave"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("StartSlave"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
#endif
	if( Buffer[0] )
	{
		startSlave = BRDC_atoi(Buffer);
		SetStartSlave( pDev, startSlave );
	}

	ULONG sample_size = format ? format : 2;
	if(format == 0x80) // упакованные 12-разрядные данные
		sample_size = 2;
	int chans = 0;
	for(ULONG i = 0; i < 16; i++)
		chans += (chanMask >> i) & 0x1;

#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("IsPreTriggerMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("IsPreTriggerMode"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	int PretrigMode = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("PreTriggerSamples"), _BRDC("16"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("PreTriggerSamples"), _BRDC("16"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	int nPreTrigSamples = BRDC_atoi(Buffer);

	BRD_PretrigMode pretrigger;
	pretrigger.enable = 0;
	pretrigger.assur = 0;
	pretrigger.external = 0;
	if(PretrigMode)
	{
		pretrigger.enable = 1;
		pretrigger.assur = (PretrigMode == 2) ? 1 : 0;
		pretrigger.external = (PretrigMode == 3) ? 1 : 0;
	}
	pretrigger.size = ULONG((nPreTrigSamples * chans * sample_size) / sizeof(ULONG));
	SetPretrigMode(pDev, &pretrigger);

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
	start_delay.value = start_delay.value * sample_size * chans / sizeof(ULONG); // было в отсчетах на канал, стало в 32-битных словах
	SetCnt(pDev, 0, &start_delay);

	BRD_EnVal acq_data;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("AcquiredSampleEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("AcquiredSampleEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	acq_data.enable = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("AcquiredSampleCounter"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("AcquiredSampleCounter"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	acq_data.value = BRDC_atoi(Buffer);
	acq_data.value = acq_data.value * sample_size * chans / sizeof(ULONG); // было в отсчетах на канал, стало в 32-битных словах
	SetCnt(pDev, 1, &acq_data);

	BRD_EnVal skip_data;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("SkipSampleEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("SkipSampleEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	skip_data.enable = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("SkipSampleCounter"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("SkipSampleCounter"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	skip_data.value = BRDC_atoi(Buffer);
	skip_data.value = skip_data.value * sample_size * chans / sizeof(ULONG); // было в отсчетах на канал, стало в 32-битных словах
	SetCnt(pDev, 2, &skip_data);

	BRD_EnVal title_mode;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("TitleEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("TitleEnable"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	title_mode.enable = BRDC_atoi(Buffer);
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("TitleSize"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("TitleSize"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	title_mode.value = BRDC_atoi(Buffer);//atoi
	SetTitleMode(pDev, &title_mode);

	ULONG title_data;
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("TitleData"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("TitleData"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	title_data = BRDC_strtol(Buffer, &endptr, 0); //strtol
//	title_data = atoi(Buffer);
	SetTitleData(pDev, &title_data);

	BRD_StdDelay adc_delay;
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
			adc_delay.nodeID = i;
			adc_delay.value = BRDC_atoi(Buffer);
			StdDelay(pDev, 1, &adc_delay);
		}
	}
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("DelayExtStart"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("DelayExtStart"), _BRDC(""), Buffer, sizeof(Buffer), iniFilePath);
#endif
	if(BRDC_strlen(Buffer))
	{
		adc_delay.nodeID = 8;
		adc_delay.value = BRDC_atoi(Buffer);
		StdDelay(pDev, 1, &adc_delay);
	}
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_getPrivateProfileString(SectionName, _BRDC("DecimFlow"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#else
	GetPrivateProfileString(SectionName, _BRDC("DecimFlow"), _BRDC("0"), Buffer, sizeof(Buffer), iniFilePath);
#endif
	ULONG mode = BRDC_atoi(Buffer);
	InHalf(pDev, mode);

	return BRDerr_OK;
}

//***************************************************************************************
// get parameters & write it into ini-file
int CAdcSrv::SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	//BRDCHAR Buffer[128];
	BRDCHAR ParamName[128];

	ULONG master;
	GetMaster(pDev, master);
//	CtrlMaster(pDev, NULL, BRDctrl_ADC_GETMASTER, &master);
	//sprintf_s(Buffer, "%u", master);
	//WritePrivateProfileString(SectionName, "MasterMode", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("MasterMode"), master, 10, NULL);

	ULONG chanMask;
	CAdcSrv::GetChanMask(pDev, chanMask);
	//sprintf_s(Buffer, "%u", chanMask);
	//WritePrivateProfileString(SectionName, "ChannelMask", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ChannelMask"), chanMask, 16, NULL);

	ULONG clkSrc;
	CAdcSrv::GetClkSource(pDev, clkSrc);
	//sprintf_s(Buffer, "%u", clkSrc);
	//WritePrivateProfileString(SectionName, "ClockSource", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ClockSource"), clkSrc, 16, NULL);
	double clkValue;
	CAdcSrv::GetClkValue(pDev, clkSrc, clkValue);
	if((clkSrc != BRDclks_BASEGEN0) && (clkSrc != BRDclks_BASEGEN1))
	{
		//sprintf_s(Buffer, "%.2f", clkValue);
		//WritePrivateProfileString(SectionName, "BaseExternalClockValue", Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, _BRDC("BaseExternalClockValue"), clkValue, 2, NULL);
	}
	double rate;
	CAdcSrv::GetRate(pDev, rate, clkValue);
	//sprintf_s(Buffer, "%.2f", rate);
	//WritePrivateProfileString(SectionName, "SamplingRate", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SamplingRate"), rate, 2, NULL);

	ULONG format;
	CtrlFormat(pDev, NULL, BRDctrl_ADC_GETFORMAT, &format);
	//sprintf_s(Buffer, "%u", format);
	//WritePrivateProfileString(SectionName, "DataFormat", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("DataFormat"), format, 10, NULL);

	ULONG code;
	CtrlCode(pDev, NULL, BRDctrl_ADC_GETCODE, &code);
	//sprintf_s(Buffer, "%u", code);
	//WritePrivateProfileString(SectionName, "CodeType", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("CodeType"), code, 10, NULL);

	double InpRange[MAX_CHAN], Bias[MAX_CHAN];
	ULONG InpResist[MAX_CHAN], DcCoupling[MAX_CHAN];
	ULONG InpFilter[MAX_CHAN];
	for(int i = 0; i < MAX_CHAN; i++)
	{
		CAdcSrv::GetInpRange(pDev, InpRange[i], i);
		BRDC_sprintf(ParamName, _BRDC("InputRange%d"), i);
		//sprintf_s(Buffer, "%.2f", InpRange[i]);
		//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, ParamName, InpRange[i], 2, NULL);
		CAdcSrv::GetBias(pDev, Bias[i], i);
		BRDC_sprintf(ParamName, _BRDC("Bias%d"), i);
		//sprintf_s(Buffer, "%.4f", Bias[i]);
		//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, ParamName, Bias[i], 4, NULL);
		CAdcSrv::GetInpResist(pDev, InpResist[i], i);
		BRDC_sprintf(ParamName, _BRDC("InputResistance%d"), i);
		//sprintf_s(Buffer, "%u", InpResist[i]);
		//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, ParamName, InpResist[i], 10, NULL);
		CAdcSrv::GetDcCoupling(pDev, DcCoupling[i], i);
		BRDC_sprintf(ParamName, _BRDC("DcCoupling%d"), i);
		//sprintf_s(Buffer, "%u", DcCoupling[i]);
		//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, ParamName, DcCoupling[i], 10, NULL);

		CAdcSrv::GetInpFilter(pDev, InpFilter[i], i);
		BRDC_sprintf(ParamName, _BRDC("InpFilter%d"), i);
		WriteInifileParam(iniFilePath, SectionName, ParamName, InpFilter[i], 10, NULL);
	}

	BRD_StartMode start_mode;
	CAdcSrv::GetStartMode(pDev, &start_mode);
	//sprintf_s(Buffer, "%u", start_mode.startSrc);
	//WritePrivateProfileString(SectionName, "StartBaseSource", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartBaseSource"), (ULONG)start_mode.startSrc, 10, NULL);
	//sprintf_s(Buffer, "%u", start_mode.startInv);
	//WritePrivateProfileString(SectionName, "StartBaseInverting", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartBaseInverting"), (ULONG)start_mode.startInv, 10, NULL);
	//sprintf_s(Buffer, "%u", start_mode.trigOn);
	//WritePrivateProfileString(SectionName, "StartMode", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartMode"), (ULONG)start_mode.trigOn, 10, NULL);
	//sprintf_s(Buffer, "%u", start_mode.trigStopSrc);
	//WritePrivateProfileString(SectionName, "StopSource", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StopSource"), (ULONG)start_mode.trigStopSrc, 10, NULL);
	//sprintf_s(Buffer, "%u", start_mode.stopInv);
	//WritePrivateProfileString(SectionName, "StopInverting", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StopInverting"), (ULONG)start_mode.stopInv, 10, NULL);
	//sprintf_s(Buffer, "%u", start_mode.reStartMode);
	//WritePrivateProfileString(SectionName, "ReStart", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ReStart"), (ULONG)start_mode.reStartMode, 10, NULL);

	ULONG	startSlave;
	GetStartSlave( pDev, startSlave );
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartSlave"), (ULONG)startSlave, 10, NULL);

	ULONG sample_size = format ? format : 2;
	if(format == 0x80) // упакованные 12-разрядные данные
		sample_size = 2;
	int chans = 0;
	for(ULONG i = 0; i < 16; i++)
		chans += (chanMask >> i) & 0x1;

	BRD_PretrigMode pretrigger;
	GetPretrigMode(pDev, &pretrigger);
	int PretrigMode = 0;
	if(pretrigger.enable && !pretrigger.assur && !pretrigger.external)
		PretrigMode = 1;
	if(pretrigger.enable && pretrigger.assur && !pretrigger.external)
		PretrigMode = 2;
	if(pretrigger.enable && !pretrigger.assur && pretrigger.external)
		PretrigMode = 3;
	//sprintf_s(Buffer, "%u", PretrigMode);
	//WritePrivateProfileString(SectionName, "IsPreTriggerMode", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("IsPreTriggerMode"), (ULONG)PretrigMode, 10, NULL);
	int nPreTrigSamples = pretrigger.size * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал 
	//sprintf_s(Buffer, "%u", nPreTrigSamples);
	//WritePrivateProfileString(SectionName, "PreTriggerSamples", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("PreTriggerSamples"), (ULONG)nPreTrigSamples, 10, NULL);

	BRD_EnVal start_delay;
	GetCnt(pDev, 0, &start_delay);
	start_delay.value = start_delay.value * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал
	//sprintf_s(Buffer, "%u", start_delay.enable);
	//WritePrivateProfileString(SectionName, "StartDelayEnable", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartDelayEnable"), (ULONG)start_delay.enable, 10, NULL);
	//sprintf_s(Buffer, "%u", start_delay.value);
	//WritePrivateProfileString(SectionName, "StartDelayCounter", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartDelayCounter"), (ULONG)start_delay.value, 10, NULL);

	BRD_EnVal acq_data;
	GetCnt(pDev, 1, &acq_data);
	acq_data.value = acq_data.value * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал
	//sprintf_s(Buffer, "%u", acq_data.enable);
	//WritePrivateProfileString(SectionName, "AcquiredSampleEnable", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("AcquiredSampleEnable"), (ULONG)acq_data.enable, 10, NULL);
	//sprintf_s(Buffer, "%u", acq_data.value);
	//WritePrivateProfileString(SectionName, "AcquiredSampleCounter", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("AcquiredSampleCounter"), (ULONG)acq_data.value, 10, NULL);

	BRD_EnVal skip_data;
	GetCnt(pDev, 2, &skip_data);
	skip_data.value = skip_data.value * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал
	//sprintf_s(Buffer, "%u", skip_data.enable);
	//WritePrivateProfileString(SectionName, "SkipSampleEnable", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SkipSampleEnable"), (ULONG)skip_data.enable, 10, NULL);
	//sprintf_s(Buffer, "%u", skip_data.value);
	//WritePrivateProfileString(SectionName, "SkipSampleCounter", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SkipSampleCounter"), (ULONG)skip_data.value, 10, NULL);

	BRD_EnVal title_mode;
	GetTitleMode(pDev, &title_mode);
	//sprintf_s(Buffer, "%u", title_mode.enable);
	//WritePrivateProfileString(SectionName, "TitleEnable", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("TitleEnable"), (ULONG)title_mode.enable, 10, NULL);
	//sprintf_s(Buffer, "%u", title_mode.value);
	//WritePrivateProfileString(SectionName, "TitleSize", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("TitleSize"), (ULONG)title_mode.value, 10, NULL);

	ULONG title_data;
	GetTitleData(pDev, &title_data);
	//sprintf_s(Buffer, "%u", title_data);
	//WritePrivateProfileString(SectionName, "TitleData", Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("TitleData"), title_data, 10, NULL);

	// the function flushes the cache
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
	IPC_writePrivateProfileString(NULL, NULL, NULL, iniFilePath);
#else
	WritePrivateProfileString(NULL, NULL, NULL, iniFilePath);
#endif
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetCfg(PBRD_AdcCfg pCfg)
{
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	pCfg->Bits = pSrvCfg->Bits;
	pCfg->Encoding = pSrvCfg->Encoding;
	pCfg->FifoSize = pSrvCfg->FifoSize;
	pCfg->InpRange = pSrvCfg->InpRange;
	pCfg->MinRate = pSrvCfg->MinRate;
	pCfg->MaxRate = pSrvCfg->MaxRate;
	pCfg->NumChans = pSrvCfg->MaxChan;
	pCfg->ChanType = 0;

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetChanMask(CModule* pModule, ULONG mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	pChan1->ByBits.ChanSel = mask;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetChanMask(CModule* pModule, ULONG& mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	mask = pChan1->ByBits.ChanSel;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetCode(CModule* pModule, ULONG type)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	pFormat->ByBits.Code = type;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetCode(CModule* pModule, ULONG& type)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	type = pFormat->ByBits.Code;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetClkSource(CModule* pModule, ULONG& ClkSrc)
{
	ULONG source;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
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
int CAdcSrv::GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	double ClkVal;
	switch(ClkSrc)
	{
	case BRDclks_SYSGEN:		// System Generator
		ClkVal = pAdcSrvCfg->SysRefGen;
		break;
	case BRDclks_BASEGEN0:		// Generator 0
		ClkVal = pAdcSrvCfg->BaseRefGen[0];
		break;
	case BRDclks_BASEGEN1:		// Generator 1
		ClkVal = pAdcSrvCfg->BaseRefGen[1];
		break;
	case BRDclks_EXTSDX:		// External clock
		ClkVal = pAdcSrvCfg->BaseExtClk;
		break;
	case BRDclks_EXTSYNX:	// External clock
	case BRDclks_SMCLK:		// 
	case BRDclks_INFREQ:		// 
	case BRDclks_SMOUTFREQ:	//
		ClkVal = pAdcSrvCfg->BaseExtClk;
		// получить частоту от других служб
//		Clk = pDacSrvCfg->RefGen[1];
		break;
	case BRDclks_BASEDDS:
		ClkVal = pAdcSrvCfg->BaseExtClk;
		break;
	//case BRDclks_ADC_DISABLED:		// disabled clock
	//	Clk = 0.0;
	//	break;
	//case BRDclks_ADC_SUBGEN:		// Submodule generator
	//	Clk = pAdcSrvCfg->SubRefGen;
	//	break;
	//case BRDclks_ADC_EXTCLK:		// External clock
	//	Clk = pAdcSrvCfg->SubExtClk;
	//	break;
	default:
		ClkVal = 0.0;
		break;
	}
	ClkValue = ClkVal;
	return BRDerr_OK;
}
	
//***************************************************************************************
int CAdcSrv::SetClkSource(CModule* pModule, ULONG ClkSrc)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
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
			return BRDerr_NOT_ACTION; // функция в режиме Slave не выполнима
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	switch(ClkSrc)
	{
	case BRDclks_SYSGEN:		// System Generator
		ClkValue = pAdcSrvCfg->SysRefGen;
		break;
	case BRDclks_BASEGEN0:		// Generator 0
		ClkValue = pAdcSrvCfg->BaseRefGen[0];
		break;
	case BRDclks_BASEGEN1:		// Generator 1
		ClkValue = pAdcSrvCfg->BaseRefGen[1];
		break;
	case BRDclks_EXTSDX:		// External clock from SDX
	case BRDclks_EXTSYNX:	// External clock from SYNX
//	case BRDclks_SMCLK:		// 
	case BRDclks_BASEDDS: // External for tetrad, but internal for board
	case BRDclks_INFREQ:		// Sinchro with input
	case BRDclks_SMOUTFREQ:	// Sinchro with output
		pAdcSrvCfg->BaseExtClk = ROUND(ClkValue);
		break;

	//case BRDclks_ADC_DISABLED:		// disabled clock
	//	ClkValue = 0.0;
	//	break;
	//case BRDclks_ADC_SUBGEN:		// Submodule generator
	//	ClkValue = pAdcSrvCfg->SubRefGen;
	//	break;
	//case BRDclks_ADC_EXTCLK:		// External clock
	//	pAdcSrvCfg->SubExtClk = ROUND(ClkValue);
	//	break;
	default:
//		ClkValue = 0.0;
		break;
	}
	return BRDerr_OK;
}
	
//***************************************************************************************
int CAdcSrv::SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue)
{
	ULONG AdcRateDivider = ROUND(ClkValue / Rate);
	AdcRateDivider = AdcRateDivider ? AdcRateDivider : 1;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_FDIV;
		param.val = AdcRateDivider;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FDIV;
			param.val = AdcRateDivider;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		else
			return BRDerr_NOT_ACTION; // функция в режиме Slave не выполнима
	}
	Rate = ClkValue / AdcRateDivider;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetRate(CModule* pModule, double& Rate, double ClkValue)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
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
		{ // Slave
			param.val = 1;
		}
	}
	Rate = ClkValue / param.val;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetBias(CModule* pModule, double& Bias, ULONG Chan)
{
	int Status = BRDerr_OK;
	double inp_range;
	Status = GetInpRange(pModule, inp_range, Chan); 
	if(Status != BRDerr_OK)
		return Status;

	int max_dac_value = 255;
	int min_dac_value = 0;
	double half_dac_value = 128.;

	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_MainTetrNum;
	////param.reg = ADM2IFnr_STATUS;
	////PADM2IF_STATUS pStatus;
	////do {
	////	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	////	pStatus = (PADM2IF_STATUS)&param.val;
	////} while(!pStatus->ByBits.CmdRdy);
	//if(WaitCmdReady(pModule, m_index, m_MainTetrNum, 1000))
	//	return BRDerr_WAIT_TIMEOUT;

	USHORT dac_data;
	if(fabs(Bias) > inp_range)
		dac_data = Bias > 0.0 ? max_dac_value : min_dac_value;
	else
		dac_data = (USHORT)floor((Bias / inp_range + 1.) * half_dac_value + 0.5);
	if(dac_data > max_dac_value)
		dac_data = max_dac_value;
	Bias = inp_range * (dac_data / half_dac_value - 1.);

	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
//	pSrvCfg->Bias[Chan] = Bias;
	pSrvCfg->ThrDac[Chan + BRDtdn_ADC_BIAS0 - 1] = (UCHAR)dac_data;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;
	param.reg = MAINnr_THDAC;
	param.val = 0;
	PMAIN_THDAC pThDac = (PMAIN_THDAC)&param.val;
	pThDac->ByBits.Data = dac_data;
	pThDac->ByBits.Num = Chan + BRDtdn_ADC_BIAS0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return Status;
}

//***************************************************************************************
int CAdcSrv::GetBias(CModule* pModule, double& Bias, ULONG Chan)
{
	int Status = BRDerr_OK;
	double inp_range;
	Status = GetInpRange(pModule, inp_range, Chan); 
	if(Status != BRDerr_OK)
		return Status;

	double half_dac_value = 128.;
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	UCHAR dac_data = (UCHAR)pSrvCfg->ThrDac[Chan + BRDtdn_ADC_BIAS0 - 1];
	Bias = inp_range * (dac_data / half_dac_value - 1.);
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetGainTuning(CModule* pModule, double& GainTuning, ULONG Chan)
{
	double max_thr = BRD_ADC_MAXGAINTUN; // %

	if(Chan > 1)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;

	USHORT dac_data;
	if(fabs(GainTuning) > max_thr)
		dac_data = GainTuning > 0.0 ? 255 : 0;
	else
		dac_data = (USHORT)floor((GainTuning / max_thr + 1.) * 128. + 0.5);
//		dac_data = (USHORT)floor((1. - GainTuning / max_thr) * 128. + 0.5);
	if(dac_data > 255)
		dac_data = 255;
	GainTuning = max_thr * (dac_data / 128. - 1.);
//	GainTuning = max_thr * (1. - dac_data / 128.);

	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
//	pSrvCfg->GainTun[Chan] = GainTuning;
	pSrvCfg->ThrDac[Chan + BRDtdn_ADC_GAINTUN0 - 1] = (UCHAR)dac_data;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;
	param.reg = MAINnr_THDAC;
	param.val = 0;
	PMAIN_THDAC pThDac = (PMAIN_THDAC)&param.val;
	pThDac->ByBits.Data = dac_data;
	pThDac->ByBits.Num = Chan + BRDtdn_ADC_GAINTUN0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetGainTuning(CModule* pModule, double& GainTuning, ULONG Chan)
{
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	double max_thr = BRD_ADC_MAXGAINTUN; // %
	if(Chan > 1)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	UCHAR dac_data = (UCHAR)pSrvCfg->ThrDac[Chan + BRDtdn_ADC_GAINTUN0 - 1];
	GainTuning = max_thr * (dac_data / 128. - 1.);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetClkPhase(CModule* pModule, double& Phase, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetClkPhase(CModule* pModule, double& Phase, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetDcCoupling(CModule* pModule, ULONG InpType, ULONG Chan)
{
	if(Chan > 3)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_INP;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADC_INP pInp = (PADC_INP)&param.val;
	switch(Chan)
	{
	case 0:
		pInp->ByBits.InpType0 = InpType;
		break;
	case 1:
		pInp->ByBits.InpType1 = InpType;
		break;
	case 2:
		pInp->ByBits.InpType2 = InpType;
		break;
	case 3:
		pInp->ByBits.InpType3 = InpType;
		break;
	}
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetDcCoupling(CModule* pModule, ULONG& InpType, ULONG Chan)
{
	if(Chan > 3)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_INP;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADC_INP pInp = (PADC_INP)&param.val;
	ULONG value;
	switch(Chan)
	{
	case 0:
		value = pInp->ByBits.InpType0;
		break;
	case 1:
		value = pInp->ByBits.InpType1;
		break;
	case 2:
		value = pInp->ByBits.InpType2;
		break;
	case 3:
		value = pInp->ByBits.InpType3;
		break;
	}
	InpType = value;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetInpFilter(CModule* pModule, ULONG isLpfOn, ULONG Chan)
{
	ADC_INP		inp;

	if(Chan > 3)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_INP, &inp.AsWhole );
	switch(Chan)
	{
		case 0:
			inp.ByBits.LpfOn0 = isLpfOn;
			break;
		case 1:
			inp.ByBits.LpfOn1 = isLpfOn;
			break;
		case 2:
			inp.ByBits.LpfOn2 = isLpfOn;
			break;
		case 3:
			inp.ByBits.LpfOn3 = isLpfOn;
			break;
	}
	IndRegWrite( pModule, m_AdcTetrNum, ADCnr_INP, inp.AsWhole );

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetInpFilter(CModule* pModule, ULONG& refIsLpfOn, ULONG Chan)
{
	ADC_INP		inp;
	ULONG		value;

	if(Chan > 3)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_INP, &inp.AsWhole );
	switch(Chan)
	{
		case 0:
			value = inp.ByBits.LpfOn0;
			break;
		case 1:
			value = inp.ByBits.LpfOn1;
			break;
		case 2:
			value = inp.ByBits.LpfOn2;
			break;
		case 3:
			value = inp.ByBits.LpfOn3;
			break;
	}
	refIsLpfOn = value;

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetInpResist(CModule* pModule, ULONG InpRes, ULONG Chan)
{
	if(Chan > 3)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_INP;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADC_INP pInp = (PADC_INP)&param.val;
	switch(Chan)
	{
	case 0:
		pInp->ByBits.InpR0 = InpRes;
		break;
	case 1:
		pInp->ByBits.InpR1 = InpRes;
		break;
	case 2:
		pInp->ByBits.InpR2 = InpRes;
		break;
	case 3:
		pInp->ByBits.InpR3 = InpRes;
		break;
	}
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetInpResist(CModule* pModule, ULONG& InpRes, ULONG Chan)
{
	if(Chan > 3)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_INP;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADC_INP pInp = (PADC_INP)&param.val;
	ULONG value;
	switch(Chan)
	{
	case 0:
		value = pInp->ByBits.InpR0;
		break;
	case 1:
		value = pInp->ByBits.InpR1;
		break;
	case 2:
		value = pInp->ByBits.InpR2;
		break;
	case 3:
		value = pInp->ByBits.InpR3;
		break;
	}
	InpRes = value;
	return BRDerr_OK;
}

////***************************************************************************************
//int CAdcSrv::SetClkMode(CModule* pModule, PVOID pMode)
//{
//	int Status = BRDerr_CMD_UNSUPPORTED;
//	PBRD_ClkMode pAdcClk = (PBRD_ClkMode)pMode;
//	Status = SetClkSource(pModule, pAdcClk->src);
//	Status = SetClkValue(pModule, pAdcClk->src, pAdcClk->value);
//	return Status;
//}
//
////***************************************************************************************
//int CAdcSrv::GetClkMode(CModule* pModule, PVOID pMode)
//{
//	int Status = BRDerr_CMD_UNSUPPORTED;
//	PBRD_ClkMode pAdcClk = (PBRD_ClkMode)pMode;
//	ULONG src = pAdcClk->src;
//	Status = GetClkSource(pModule, src);
//	pAdcClk->src = src;
//	Status = GetClkValue(pModule, pAdcClk->src, pAdcClk->value);
//	return Status;
//}
//
////***************************************************************************************
//int CAdcSrv::SetSyncMode(CModule* pModule, PVOID pMode)
//{
//	int Status = BRDerr_CMD_UNSUPPORTED;
//	PBRD_SyncMode pSyncMode = (PBRD_SyncMode)pMode;
//	Status = SetClkSource(pModule, pSyncMode->clkSrc);
//	Status = SetClkValue(pModule, pSyncMode->clkSrc, pSyncMode->clkValue);
//	Status = SetRate(pModule, pSyncMode->rate, pSyncMode->clkSrc, pSyncMode->clkValue);
//	return Status;
//}
//
////***************************************************************************************
//int CAdcSrv::GetSyncMode(CModule* pModule, PVOID pMode)
//{
//	int Status = BRDerr_CMD_UNSUPPORTED;
//	PBRD_SyncMode pSyncMode = (PBRD_SyncMode)pMode;
//	ULONG src = pSyncMode->clkSrc;
//	Status = GetClkSource(pModule, src);
//	pSyncMode->clkSrc = src;
//	Status = GetClkValue(pModule, pSyncMode->clkSrc, pSyncMode->clkValue);
//	Status = GetRate(pModule, pSyncMode->rate, pSyncMode->clkValue);
//	return Status;
//}
//
//***************************************************************************************
int CAdcSrv::SetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
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
		pStMode->ByBits.Restart   = pStartMode->reStartMode;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		ULONG master = pMode0->ByBits.Master;
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStMode->ByBits.SrcStart = pStartMode->startSrc;
		if(!master)
			pStMode->ByBits.SrcStart = BRDsts_EXTSYNX; // SLAVE
		pStMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
		pStMode->ByBits.InvStart  = pStartMode->startInv;
		pStMode->ByBits.InvStop   = pStartMode->stopInv;
		pStMode->ByBits.TrigStart = pStartMode->trigOn;
		pStMode->ByBits.Restart   = pStartMode->reStartMode;
		if(master && pStMode->ByBits.Restart == 1)
			pStMode->ByBits.SrcStop   = BRDsts_TRDADC;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.tetr = m_AdcTetrNum;
		pStMode->ByBits.SrcStart  = BRDsts_EXTSYNX; // SLAVE
		pStMode->ByBits.SrcStop   = BRDsts_PRG;
		pStMode->ByBits.InvStart  = 0;
		pStMode->ByBits.InvStop   = 0;
		pStMode->ByBits.TrigStart = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;//ADM2IF_ntBASEDAC;
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

		//param.reg = ADM2IFnr_PRTMODE;
		//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		//PADM2IF_PRTMODE pPreMode = (PADM2IF_PRTMODE)&param.val;
		//pStartMode->pretrig	 = pPreMode->ByBits.Enable;

	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;//ADM2IF_ntMAIN;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		ULONG master = pMode0->ByBits.Master;
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStMode = (PADM2IF_STMODE)&param.val;
		pStartMode->startSrc	= pStMode->ByBits.SrcStart;
//		if(!master)
//			pStartMode->startSrc = BRDsts_EXTSYNX;
		pStartMode->trigStopSrc	= pStMode->ByBits.SrcStop;
		pStartMode->startInv	= pStMode->ByBits.InvStart;
		pStartMode->stopInv		= pStMode->ByBits.InvStop;
		pStartMode->trigOn		= pStMode->ByBits.TrigStart;
		pStartMode->reStartMode	= pStMode->ByBits.Restart;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_FTYPE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int widthFifo = param.val >> 3; // ширина FIFO (в байтах)

	param.reg = ADM2IFnr_TLMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_TLMODE pTlMode = (PADM2IF_TLMODE)&param.val;
	pTlMode->ByBits.TitleOn = pTitleMode->enable;
	if(pTitleMode->value && pTitleMode->value < 8)
		pTitleMode->value = 0;
//	if(pTitleMode->value > 8)
//		pTitleMode->value = (pTitleMode->value >> 1) << 1;
	pTlMode->ByBits.Size = pTitleMode->value * sizeof(ULONG) / widthFifo; // pTitleMode->value - размер заголовка в 32-разрядных словах
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pTitleMode->value = pTlMode->ByBits.Size * widthFifo / sizeof(ULONG); // pTitleMode->value - размер заголовка в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;

	param.reg = ADM2IFnr_FTYPE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int widthFifo = param.val >> 3; // ширина FIFO (в байтах)

	param.reg = ADM2IFnr_TLMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_TLMODE pTlMode = (PADM2IF_TLMODE)&param.val;
	pTitleMode->enable = pTlMode->ByBits.TitleOn;
	pTitleMode->value = pTlMode->ByBits.Size * widthFifo / sizeof(ULONG); // pTitleMode->value - размер заголовка в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetPretrigMode(CModule* pModule, PBRD_PretrigMode pPreMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_PRTMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_PRTMODE pPrtMode = (PADM2IF_PRTMODE)&param.val;
	pPrtMode->ByBits.Enable = pPreMode->enable;
	pPrtMode->ByBits.External = pPreMode->external;
	pPrtMode->ByBits.Assur = pPreMode->assur;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	//param.reg = ADM2IFnr_CNTAF;
	//int pre_size = pAdcSrvCfg->FifoSize / sizeof(ULONG) - pPreMode->size; // в 32-битных словах
	//param.val = (pre_size < 16) ? 16 : pre_size;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = ADM2IFnr_FTYPE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int widthFifo = param.val >> 3; // ширина FIFO (в том числе вспомогательного претриггерного) в байтах

	//PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	//int deepFifo = (pAdcSrvCfg->FifoSize / widthFifo) >> 1; // глубина вспомогательного претриггерного FIFO

	param.reg = ADM2IFnr_PFSIZE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int deepFifo = param.val; // глубина вспомогательного претриггерного FIFO
	if(!deepFifo)
	{
		param.reg = ADM2IFnr_FSIZE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		deepFifo = param.val; // глубина основного FIFO
	}

	int pre_size = deepFifo - pPreMode->size * sizeof(ULONG) / widthFifo; // pPreMode->size - в 32-битных словах
	param.reg = ADM2IFnr_CNTAF;
	param.val = (pre_size == deepFifo) ? deepFifo - 2 : pre_size ; // в словах ширины FIFO
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pre_size = deepFifo - param.val; // в словах ширины FIFO
	pPreMode->size = pre_size * widthFifo / sizeof(ULONG); // в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetPretrigMode(CModule* pModule, PBRD_PretrigMode pPreMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_PRTMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_PRTMODE pPrtMode = (PADM2IF_PRTMODE)&param.val;
	pPreMode->enable = pPrtMode->ByBits.Enable;
	pPreMode->external = pPrtMode->ByBits.External;
	pPreMode->assur = pPrtMode->ByBits.Assur;

	//PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	//param.reg = ADM2IFnr_CNTAF;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//pPreMode->size = pAdcSrvCfg->FifoSize / sizeof(ULONG) - param.val; // в 32-битных словах

	param.reg = ADM2IFnr_FTYPE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int widthFifo = param.val >> 3; // ширина FIFO (в том числе вспомогательного претриггерного) в байтах

	//PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	//int deepFifo = (pAdcSrvCfg->FifoSize / widthFifo) >> 1; // глубина вспомогательного претриггерного FIFO

	param.reg = ADM2IFnr_PFSIZE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int deepFifo = param.val; // глубина вспомогательного претриггерного FIFO
	if(!deepFifo)
	{
		param.reg = ADM2IFnr_FSIZE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		deepFifo = param.val; // глубина основного FIFO
	}

	param.reg = ADM2IFnr_CNTAF;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG pre_size = deepFifo - param.val; // в словах ширины FIFO
	pPreMode->size = pre_size * widthFifo / sizeof(ULONG); // в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetPretrigEvent(CModule* pModule, ULONG& EventStart)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_PRTEVENTLO;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG sync_event = param.val & 0xffff;
	param.reg = ADCnr_PRTEVENTHI;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	sync_event |= (param.val << 16); // получаем в 64-разрядных словах
//	ULONG start_addr = GetStartAddr(pModule);
//	*(ULONG*)args = sync_event - start_addr;
	EventStart = sync_event << 1; // возвращаем в 32-разрядных словах
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetClkLocation(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.AdmClk = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetClkLocation(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	mode = pMode0->ByBits.AdmClk;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetDblClk(CModule* pModule, ULONG& mode)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	mode = 0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetDblClk(CModule* pModule, ULONG& mode)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	mode = 0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetClkThr(CModule* pModule, double& ClkThr)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetClkThr(CModule* pModule, double& ClkThr)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetExtClkThr(CModule* pModule, double& ClkThr)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetExtClkThr(CModule* pModule, double& ClkThr)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SelfClbr(CModule* pModule)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetClkInv(CModule* pModule, ULONG ClkInv)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetClkInv(CModule* pModule, ULONG& ClkInv)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	ClkInv = 0;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetMu(CModule* pModule, void *args )
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CAdcSrv::GetMu(CModule* pModule, void *args )
{
	return BRDerr_CMD_UNSUPPORTED;
}

//***************************************************************************************
int CAdcSrv::SetStartSlave(CModule* pModule, ULONG StartSlave)
{
	//
	// Установить/обнулить бит CONTROL1[START_SL(8)]
	//
	U32			value;

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_CTRL1, &value );
	value &= ~(1<<8);
	if( StartSlave )
		value |= 1<<8;
	IndRegWrite( pModule, m_AdcTetrNum, ADCnr_CTRL1, value );
	//printf( ">>> SetStartSlave(%d) code=0x%X\n", StartSlave, value );

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetStartSlave(CModule* pModule, ULONG& refStartSlave)
{
	//
	// Считать бит CONTROL1[START_SL(8)]
	//
	U32			value;

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_CTRL1, &value );
	refStartSlave = 0x1 & (value>>8);

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetClockSlave(CModule* pModule, ULONG ClockSlave)
{
	//
	// Установить/обнулить бит CONTROL1[CLK_SL(9)]
	//
	U32			value;

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_CTRL1, &value );
	value &= ~(1<<9);
	if( ClockSlave )
		value |= 1<<9;
	IndRegWrite( pModule, m_AdcTetrNum, ADCnr_CTRL1, value );

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetClockSlave(CModule* pModule, ULONG& refClockSlave)
{
	//
	// Считать бит CONTROL1[CLK_SL(9)]
	//
	U32			value;

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_CTRL1, &value );
	refClockSlave = 0x1 & (value>>9);

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetChanOrder(CModule* pModule, BRD_ItemArray *pItemArray )
{
	//
	// Ничего не возвращать
	//
	pItemArray->itemReal = 0;

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetInpSrc(CModule* pModule, PVOID pCtrl)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetInpSrc(CModule* pModule, PVOID pCtrl)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetGain(CModule* pModule, double& Gain, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetGain(CModule* pModule, double& Gain, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Gain = 1.0;
	return Status;
//	double value = 1.0;
//	return value;
}

//***************************************************************************************
int CAdcSrv::SetInpRange(CModule* pModule, double& InpRange, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetInpRange(CModule* pModule, double& InpRange, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
	InpRange = pSrvCfg->InpRange / 1000.;
//	double value = (double)pSrvCfg->InpRange;
//	return value;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetTestMode(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Test = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetTestMode(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	mode = pMode1->ByBits.Test;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetTestSeq(CModule* pModule, ULONG seq)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_TESTSEQ;
	param.val = seq;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetTestSeq(CModule* pModule, ULONG& seq)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_TESTSEQ;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	seq = param.val;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::StdDelay(CModule* pModule, int cmd, PBRD_StdDelay delay)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_TRES;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	if(param.val & 0x800)
	{
		param.reg = ADM2IFnr_STDDELAY;
		STD_DELAY* pAdcDelay = (STD_DELAY*)&param.val;
		pAdcDelay->ByBits.Delay = 0x3f;
		pAdcDelay->ByBits.ID = delay->nodeID;
		pAdcDelay->ByBits.Reset = 0;
		pAdcDelay->ByBits.Write = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pAdcDelay->ByBits.Used)
		{
			//pAdcDelay->ByBits.ID = delay->nodeID;
			//pAdcDelay->ByBits.Delay = delay->value;
			//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			if(cmd == 1)
			{ // write
				pAdcDelay->ByBits.Write = 1;
				pAdcDelay->ByBits.Reset = 0;
				pAdcDelay->ByBits.Delay = delay->value;
			}
			if(cmd == 2)
			{ // reset
				pAdcDelay->ByBits.Write = 0;
				pAdcDelay->ByBits.Reset = 1;
			}
			if(cmd > 0)
			{ // write || reset
				pAdcDelay->ByBits.ID = delay->nodeID;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				pAdcDelay->ByBits.Reset = 0;
				pAdcDelay->ByBits.Write = 0;
				pAdcDelay->ByBits.ID = delay->nodeID;
				pAdcDelay->ByBits.Delay = 0x3f;
				pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			}
			delay->value = pAdcDelay->ByBits.Delay;
		}
		else
			return BRDerr_BAD_NODEID;
	}
	else
		return BRDerr_CMD_UNSUPPORTED;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::InHalf(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.InHalf = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetSpecific(CModule* pModule, PBRD_AdcSpec pSpec)
{
//	int Status = BRDerr_CMD_UNSUPPORTED;
	int Status = BRDerr_OK;
	switch(pSpec->command)
	{
	case ADCcmd_ADJUST:
		{
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = m_AdcTetrNum;
		param.reg = ADCnr_ADJUST;
		param.val = *(PULONG)pSpec->arg;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		break;
	case ADCcmd_INHALF:
		{
		ULONG mode = *(PULONG)pSpec->arg;
		InHalf(pModule, mode);
		}
		break;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::GetSpecific(CModule* pModule, PBRD_AdcSpec pSpec)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	mode = 0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CAdcSrv::SetMaster(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.reg = ADM2IFnr_MODE0;
	param.tetr = m_MainTetrNum;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Master = mode >> 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.tetr = m_AdcTetrNum;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pMode0->ByBits.Master = mode & 0x1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetMaster(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.reg = ADM2IFnr_MODE0;
	param.tetr = m_AdcTetrNum;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	mode = pMode0->ByBits.Master;

	param.tetr = m_MainTetrNum;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	mode |= (pMode0->ByBits.Master << 1);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetTarget(CModule* pModule, ULONG target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Out = target;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetTarget(CModule* pModule, ULONG& target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	target = pMode1->ByBits.Out;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	switch(numreg)
	{
	case 0:
		pMode0->ByBits.Cnt0En = pEnVal->enable;
		break;
	case 1:
		pMode0->ByBits.Cnt1En = pEnVal->enable;
		break;
	case 2:
		pMode0->ByBits.Cnt2En = pEnVal->enable;
		break;
	}
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = ADM2IFnr_FTYPE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int widthFifo = param.val >> 3; // ширина FIFO (в байтах)

	param.val = pEnVal->value * sizeof(ULONG) / widthFifo; // pEnVal->value - в 32-битных словах
	param.reg = ADM2IFnr_CNT0 + numreg;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//	if(pEnVal->enable)
//		BRDC_printf(_BRDC("CNT%d = %d\n"), numreg, param.val & 0xffff);

	pEnVal->value = param.val * widthFifo / sizeof(ULONG); // pEnVal->value - в 32-битных словах

	param.reg = ADM2IFnr_TRES;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	if(param.val & 0x400) // бит 10 регистра TRES - признак поддержки 32-разрядных счетчиков
	{
		param.val = (pEnVal->value * sizeof(ULONG) / widthFifo) >> 16;
		param.reg = ADM2IFnr_ECNT0 + numreg;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//		if(pEnVal->enable)
//			BRDC_printf(_BRDC("ECNT%d = %d\n"), numreg, param.val);
	}

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	switch(numreg)
	{
	case 0:
		pEnVal->enable = pMode0->ByBits.Cnt0En;
		break;
	case 1:
		pEnVal->enable = pMode0->ByBits.Cnt1En;
		break;
	case 2:
		pEnVal->enable = pMode0->ByBits.Cnt2En;
		break;
	}

	param.reg = ADM2IFnr_FTYPE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	int widthFifo = param.val >> 3; // ширина FIFO (в байтах)

	param.reg = ADM2IFnr_CNT0 + numreg;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//pEnVal->value = param.val * widthFifo / sizeof(ULONG); // pEnVal->value - в 32-битных словах
	ULONG val = param.val;

	param.reg = ADM2IFnr_TRES;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	if(param.val & 0x400) // бит 10 регистра TRES - признак поддержки 32-разрядных счетчиков
	{
		param.reg = ADM2IFnr_ECNT0 + numreg;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		val = val + (param.val << 16);
	}
	pEnVal->value = val * widthFifo / sizeof(ULONG); // pEnVal->value - в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetTitleData(CModule* pModule, PULONG pTitleData)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_TLDATAL;
	param.val = (*pTitleData) & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = ADCnr_TLDATAH;
	param.val = ((*pTitleData) >> 16) & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetTitleData(CModule* pModule, PULONG pTitleData)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_TLDATAL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG tldata = param.val & 0xffff;
	param.reg = ADCnr_TLDATAH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	tldata |= param.val << 16;
	*pTitleData = tldata;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::ClrBitsOverflow(CModule* pModule, ULONG flags)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_OVERFLOW;
	param.val = flags;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = ADCnr_FLAGCLR;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADC_FLAGCLR pFlClr = (PADC_FLAGCLR)&param.val;
	pFlClr->ByBits.ClrOvrAdc = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::IsBitsOverflow(CModule* pModule, ULONG& OverBits)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_OVERFLOW;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	OverBits = param.val & 0xFFFF;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::FifoReset(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::AdcEnable(CModule* pModule, ULONG enable)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = enable;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	if(!pMode0->ByBits.Master)
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		pMode0->ByBits.Start = enable;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::PrepareStart(CModule* pModule, void *arg)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::IsComplex(CModule *pModule, U32 *pIsComplex )
{
	*pIsComplex = 0;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetFc(CModule *pModule, double *pFc, U32 nChan)
{
	*pFc = 0.0;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetFc(CModule *pModule, double *pFc, U32 nChan)
{
	*pFc = 0.0;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::SetStartResist(CModule* pModule, ULONG InpRes)
{
	U32			val;

	//
	// Установить вх.сопротивление для сигнала внешнего старта: 0 - 2.5 кОм, 1 - 50 Ом
	//

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_CTRL1, &val );
	val &= 0xFFFE;
	if( InpRes )
		val |= 0x1;
	IndRegWrite( pModule, m_AdcTetrNum, ADCnr_CTRL1, val );

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetStartResist(CModule* pModule, ULONG& InpRes)
{
	U32			val;

	//
	// Считать вх.сопротивление для сигнала внешнего старта: 0 - 2.5 кОм, 1 - 50 Ом
	//

	IndRegRead( pModule, m_AdcTetrNum, ADCnr_CTRL1, &val );
	InpRes = val & 0x1;

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetAddrData(CModule* pModule, U32 *pAddrData )
{
	ULONG m_AdmNum;

	m_AdmNum = 0;
	*pAddrData = (m_AdmNum << 16) | m_AdcTetrNum;

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetFifoStatus(CModule* pModule, U32 *pFifoStatus )
{
	DirRegRead( pModule, m_AdcTetrNum, ADM2IFnr_STATUS, pFifoStatus );
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::GetFormat(CModule* pModule, U32 *pFormat )
{
	IndRegRead( pModule, m_AdcTetrNum, ADM2IFnr_FORMAT, pFormat );
	return BRDerr_OK;
}

//***************************************************************************************
S32		CAdcSrv::IndRegRead( CModule* pModule, S32 tetrNo, S32 regNo, U32 *pVal )
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
//S32		CAdcSrv::IndRegRead( CModule* pModule, S32 tetrNo, S32 regNo, ULONG *pVal )
//{
//	return IndRegRead( pModule, tetrNo, regNo, (U32*)pVal );
//}

//***************************************************************************************
S32		CAdcSrv::IndRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, U32 val )
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
//S32		CAdcSrv::IndRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, ULONG val )
//{
//	return IndRegWrite( pModule, tetrNo, regNo, (U32)val );
//}

//***************************************************************************************
S32		CAdcSrv::DirRegRead( CModule* pModule, S32 tetrNo, S32 regNo, U32 *pVal )
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
//S32		CAdcSrv::DirRegRead( CModule* pModule, S32 tetrNo, S32 regNo, ULONG *pVal )
//{
//	return DirRegRead( pModule, tetrNo, regNo, (U32*)pVal );
//}

//***************************************************************************************
S32		CAdcSrv::DirRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, U32 val )
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
//S32		CAdcSrv::DirRegWrite( CModule* pModule, S32 tetrNo, S32 regNo, ULONG val )
//{
//	return DirRegWrite( pModule, tetrNo, regNo, (U32)val );
//}

// ***************** End of file AdcSrv.cpp ***********************
