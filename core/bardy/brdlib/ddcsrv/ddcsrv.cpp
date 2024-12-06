/****************** File DdcSrv.cpp ************
 *
 * BRD Driver for DD— service on DD— submodule
 *
 * (C) InSys by Dorokhin A. Jan 2006
 *
 ***********************************************/

#include "module.h"
#include "ddcsrv.h"

#define	CURRFILE "DDCSRV"

//***************************************************************************************
void CDdcSrv::GetDdcTetrNum(CModule* pModule)
{
	m_DdcTetrNum = 0;
}

//***************************************************************************************
void CDdcSrv::FreeInfoForDialog(PVOID pInfo)
{
	PDDCSRV_INFO pSrvInfo = (PDDCSRV_INFO)pInfo;
	delete pSrvInfo;
}

//***************************************************************************************
PVOID CDdcSrv::GetInfoForDialog(CModule* pDev)
{
	PDDCSRV_CFG pSrvCfg = (PDDCSRV_CFG)m_pConfig;
	PDDCSRV_INFO pSrvInfo = new DDCSRV_INFO;
	pSrvInfo->Size = sizeof(DDCSRV_INFO);
//	pSrvInfo->Bits = pSrvCfg->Bits;
//	pSrvInfo->Encoding = pSrvCfg->Encoding;
	pSrvInfo->FifoSize = pSrvCfg->FifoSize;
	pSrvInfo->ChanCnt = pSrvCfg->MaxChan;
//	pSrvInfo->MinRate = pSrvCfg->MinRate;
//	pSrvInfo->MaxRate = pSrvCfg->MaxRate;
//	pSrvInfo->RefPVS = pSrvCfg->RefPVS;
//	pSrvInfo->BaseRefGen[0] = pSrvCfg->BaseRefGen[0];
//	pSrvInfo->BaseRefGen[1] = pSrvCfg->BaseRefGen[1];
	CDdcSrv::GetChanMask(pDev, pSrvInfo->ChanMask);
//	ULONG chan_mask;
//	CtrlChanMask(pDev, NULL, BRDctrl_DDC_GETCHANMASK, &chan_mask);
//	pSrvInfo->ChanMask = chan_mask;
	ULONG master;
	CtrlMaster(pDev, NULL, BRDctrl_DDC_GETMASTER, &master);
	pSrvInfo->SyncMode = master;
	CDdcSrv::GetClkSource(pDev, pSrvInfo->ClockSrc);
	CDdcSrv::GetClkValue(pDev, pSrvInfo->ClockSrc, pSrvInfo->ClockValue);
	CDdcSrv::GetRate(pDev, pSrvInfo->SamplingRate, pSrvInfo->ClockValue);
//	if((pSrvInfo->ClockSrc == BRDclks_EXTCLK))
//		pSrvInfo->ExtClk = pSrvInfo->ClockValue;

	BRD_StartMode start_mode;
	CDdcSrv::GetStartMode(pDev, &start_mode);
	//CtrlStartMode(pDev, NULL, BRDctrl_DDC_GETSTARTMODE, &start_mode);
	pSrvInfo->StartSrc		= start_mode.startSrc;
	pSrvInfo->StartInv		= start_mode.startInv;
	pSrvInfo->StartStopMode = start_mode.trigOn;
	pSrvInfo->StopSrc		= start_mode.trigStopSrc;
	pSrvInfo->StopInv		= start_mode.stopInv;
	
	BRDCHAR module_name[40];
//	sprintf(module_name, _BRDC(" (%s)") ,pDev->m_name);
	BRDC_sprintf(module_name, _BRDC(" (%s)") ,pDev->GetName());
//	PBASE_Info binfo = pModule->GetBaseInfo();
//	sprintf(module_name, _BRDC(" (%s)") ,binfo->boardName);
	BRDC_strcpy(pSrvInfo->Name, m_name);
	BRDC_strcat(pSrvInfo->Name, module_name);

	return pSrvInfo;
//	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetPropertyFromDialog(void	*pDev, void	*args)
{
	CModule* pModule = (CModule*)pDev;
	PDDCSRV_INFO pInfo = (PDDCSRV_INFO)args;
	CDdcSrv::SetChanMask(pModule, pInfo->ChanMask);
//	CtrlChanMask(pDev, NULL, BRDctrl_DDC_SETCHANMASK, &pInfo->ChanMask);
	ULONG master = pInfo->SyncMode;
	CtrlMaster(pDev, NULL, BRDctrl_DDC_SETMASTER, &master);
//	ULONG chan_mask = pInfo->ChanMask;
//	CtrlChanMask(pDev, NULL, BRDctrl_DDC_SETCHANMASK, &chan_mask);
	CDdcSrv::SetClkSource(pModule, pInfo->ClockSrc);
	CDdcSrv::SetClkValue(pModule, pInfo->ClockSrc, pInfo->ClockValue);
	CDdcSrv::SetRate(pModule, pInfo->SamplingRate, pInfo->ClockSrc, pInfo->ClockValue);
	//BRD_SyncMode sync_mode;
	//sync_mode.clkSrc	 = pInfo->ClockSrc;
	//sync_mode.clkValue	 = pInfo->ClockValue;
	//sync_mode.rate		 = pInfo->SamplingRate;
	//CtrlSyncMode(pDev, NULL, BRDctrl_DDC_SETSYNCMODE, &sync_mode);

	BRD_StartMode start_mode;
//	CtrlStartMode(pDev, NULL, BRDctrl_DDC_GETSTARTMODE, &start_mode);
	start_mode.startSrc = pInfo->StartSrc;
	start_mode.startInv = pInfo->StartInv;		
	start_mode.trigOn = pInfo->StartStopMode;
	start_mode.trigStopSrc = pInfo->StopSrc;		
	start_mode.stopInv = pInfo->StopInv;
	CDdcSrv::SetStartMode(pModule, &start_mode);
//	CtrlStartMode(pDev, NULL, BRDctrl_DDC_SETSTARTMODE, &start_mode);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	BRDCHAR Buffer[128];
	BRDCHAR* endptr;

	IPC_getPrivateProfileString(SectionName, _BRDC("MasterMode") ,_BRDC("1") ,Buffer, sizeof(Buffer), iniFilePath);
	ULONG master = BRDC_atoi(Buffer);
	CtrlMaster(pDev, NULL, BRDctrl_DDC_SETMASTER, &master);

//	IPC_getPrivateProfileString(SectionName, _BRDC("ChannelMask") ,_BRDC("1") ,Buffer, sizeof(Buffer), iniFilePath);
//	ULONG chanMask = atoi(Buffer);
//	CDdcSrv::SetChanMask(pDev, chanMask);

	IPC_getPrivateProfileString(SectionName, _BRDC("ClockSource") ,_BRDC("1") ,Buffer, sizeof(Buffer), iniFilePath);
	ULONG clkSrc = BRDC_strtol(Buffer, &endptr, 0);
//	ULONG clkSrc = atoi(Buffer);
	CDdcSrv::SetClkSource(pDev, clkSrc);
	IPC_getPrivateProfileString(SectionName, _BRDC("ExternalClockValue") ,_BRDC("100000.0") ,Buffer, sizeof(Buffer), iniFilePath);
	double clkValue = BRDC_atof(Buffer);
	CDdcSrv::SetClkValue(pDev, clkSrc, clkValue);
	IPC_getPrivateProfileString(SectionName, _BRDC("SamplingRate") ,_BRDC("100000.0") ,Buffer, sizeof(Buffer), iniFilePath);
	double rate = BRDC_atof(Buffer);
	CDdcSrv::SetRate(pDev, rate, clkSrc, clkValue);

//	IPC_getPrivateProfileString(SectionName, _BRDC("DataFormat") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
//	ULONG format = atoi(Buffer);
//	SetFormat(pDev, format);
//	CtrlFormat(pDev, NULL, BRDctrl_DDC_SETFORMAT, &format);

	BRD_StartMode start_mode;
	IPC_getPrivateProfileString(SectionName, _BRDC("StartSource") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_mode.startSrc = BRDC_strtol(Buffer, &endptr, 0);//atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("StartInverting") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_mode.startInv = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("StartMode") ,_BRDC("1") ,Buffer, sizeof(Buffer), iniFilePath);
	start_mode.trigOn = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("StopSource") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_mode.trigStopSrc = BRDC_strtol(Buffer, &endptr, 0);//atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("StopInverting") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_mode.stopInv = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("ReStart") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_mode.reStartMode = BRDC_atoi(Buffer);
	CDdcSrv::SetStartMode(pDev, &start_mode);

	//BRD_EnVal start_delay;
	//IPC_getPrivateProfileString(SectionName, _BRDC("StartDelayEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	//start_delay.enable = atoi(Buffer);
	//IPC_getPrivateProfileString(SectionName, _BRDC("StartDelayCounter") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	//start_delay.value = atoi(Buffer);
	//SetCnt(pDev, 0, &start_delay);

	//BRD_EnVal acq_data;
	//IPC_getPrivateProfileString(SectionName, _BRDC("AcquiredSampleEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	//acq_data.enable = atoi(Buffer);
	//IPC_getPrivateProfileString(SectionName, _BRDC("AcquiredSampleCounter") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	//acq_data.value = atoi(Buffer);
	//SetCnt(pDev, 1, &acq_data);

	//BRD_EnVal skip_data;
	//IPC_getPrivateProfileString(SectionName, _BRDC("SkipSampleEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	//skip_data.enable = atoi(Buffer);
	//IPC_getPrivateProfileString(SectionName, _BRDC("SkipSampleCounter") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	//skip_data.value = atoi(Buffer);
	//SetCnt(pDev, 2, &skip_data);

	BRD_EnVal title_mode;
	IPC_getPrivateProfileString(SectionName, _BRDC("TitleEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	title_mode.enable = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("TitleSize") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	title_mode.value = BRDC_atoi(Buffer);
	SetTitleMode(pDev, &title_mode);

	ULONG title_data;
	IPC_getPrivateProfileString(SectionName, _BRDC("TitleData") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	title_data = BRDC_atoi(Buffer);
	SetTitleData(pDev, &title_data);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	ULONG master;
	CtrlMaster(pDev, NULL, BRDctrl_DDC_GETMASTER, &master);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("MasterMode"), master, 10, NULL);

	ULONG chanMask;
	CDdcSrv::GetChanMask(pDev, chanMask);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ChannelMask"), chanMask, 16, NULL);

	ULONG clkSrc;
	CDdcSrv::GetClkSource(pDev, clkSrc);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ClockSource"), clkSrc, 16, NULL);
	double clkValue;
	CDdcSrv::GetClkValue(pDev, clkSrc, clkValue);
	if((clkSrc != BRDclks_BASEGEN0) && (clkSrc != BRDclks_BASEGEN1))
	{
		WriteInifileParam(iniFilePath, SectionName, _BRDC("BaseExternalClockValue"), clkValue, 2, NULL);
	}
	double rate;
	CDdcSrv::GetRate(pDev, rate, clkValue);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SamplingRate"), rate, 2, NULL);

	ULONG format;
	GetFormat(pDev, format);
//	CtrlFormat(pDev, NULL, BRDctrl_DDC_GETFORMAT, &format);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("DataFormat"), format, 10, NULL);

	BRD_StartMode start_mode;
	CDdcSrv::GetStartMode(pDev, &start_mode);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartBaseSource"), (ULONG)start_mode.startSrc, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartBaseInverting"), (ULONG)start_mode.startInv, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartMode"), (ULONG)start_mode.trigOn, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StopSource"), (ULONG)start_mode.trigStopSrc, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StopInverting"), (ULONG)start_mode.stopInv, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ReStart"), (ULONG)start_mode.reStartMode, 10, NULL);

	ULONG sample_size = format ? format : 2;
	sample_size <<= 1;
	int chans = 0;
	for(ULONG i = 0; i < 16; i++)
		chans += (chanMask >> i) & 0x1;

	BRD_EnVal start_delay;
	GetCnt(pDev, 0, &start_delay);
	start_delay.value = start_delay.value * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartDelayEnable"), (ULONG)start_delay.enable, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("StartDelayCounter"), (ULONG)start_delay.value, 10, NULL);

	BRD_EnVal acq_data;
	GetCnt(pDev, 1, &acq_data);
	acq_data.value = acq_data.value * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал
	WriteInifileParam(iniFilePath, SectionName, _BRDC("AcquiredSampleEnable"), (ULONG)acq_data.enable, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("AcquiredSampleCounter"), (ULONG)acq_data.value, 10, NULL);

	BRD_EnVal skip_data;
	GetCnt(pDev, 2, &skip_data);
	skip_data.value = skip_data.value * sizeof(ULONG) / sample_size / chans; // было в 32-битных словах, стало в отсчетах на канал
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SkipSampleEnable"), (ULONG)skip_data.enable, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SkipSampleCounter"), (ULONG)skip_data.value, 10, NULL);


	BRD_EnVal title_mode;
	GetTitleMode(pDev, &title_mode);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("TitleEnable"), (ULONG)title_mode.enable, 10, NULL);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("TitleSize"), (ULONG)title_mode.value, 10, NULL);

	ULONG title_data;
	GetTitleData(pDev, &title_data);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("TitleData"), title_data, 10, NULL);

	// the function flushes the cache
	IPC_writePrivateProfileString(NULL, NULL, NULL, iniFilePath);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetCfg(PBRD_DdcCfg pCfg)
{
	PDDCSRV_CFG pSrvCfg = (PDDCSRV_CFG)m_pConfig;
	pCfg->Bits = 16;
	pCfg->Encoding = 1;
	pCfg->FifoSize = pSrvCfg->FifoSize;
	pCfg->DdcCnt = pSrvCfg->MaxChan;
	pCfg->AdmConst = m_AdmConst.AsWhole;
//	pCfg->InpRange = pSrvCfg->InpRange;
//	pCfg->MinRate = pSrvCfg->MinRate;
//	pCfg->MaxRate = pSrvCfg->MaxRate;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetChanMask(CModule* pModule, ULONG mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	pChan1->ByBits.ChanSel = mask;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetChanMask(CModule* pModule, ULONG& mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	mask = pChan1->ByBits.ChanSel;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetFormat(CModule* pModule, ULONG format)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	pFormat->ByBits.Pack = format;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetFormat(CModule* pModule, ULONG& format)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	format = pFormat->ByBits.Pack;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetClkSource(CModule* pModule, ULONG& ClkSrc)
{
	ULONG source;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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
int CDdcSrv::GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	return BRDerr_OK;
}
	
//***************************************************************************************
int CDdcSrv::SetClkSource(CModule* pModule, ULONG ClkSrc)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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
			return BRDerr_NOT_ACTION; // функци€ в режиме Slave не выполнима
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	return BRDerr_OK;
}
	
//***************************************************************************************
int CDdcSrv::SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_FDIV;
	ULONG DdcRateDivider = ROUND(ClkValue / Rate);
	DdcRateDivider = DdcRateDivider ? DdcRateDivider : 1;
	param.val = DdcRateDivider;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	Rate = ClkValue / DdcRateDivider;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetRate(CModule* pModule, double& Rate, double ClkValue)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_FDIV;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	Rate = ClkValue / param.val;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;//ADM2IF_ntBASEDAC;
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

		param.tetr = m_DdcTetrNum;
		pStMode->ByBits.SrcStart  = BRDsts_EXTSYNX; // SLAVE
		pStMode->ByBits.SrcStop   = BRDsts_PRG;
		pStMode->ByBits.InvStart  = 0;
		pStMode->ByBits.InvStop   = 0;
		pStMode->ByBits.TrigStart = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
/*
	{ // Master/Slave
		param.tetr = m_MainTetrNum;//ADM2IF_ntMAIN;
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
			pStMode->ByBits.Restart   = pStartMode->reStartMode;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
//		else
//			return BRDerr_; // функци€ в режиме Slave не выполнима
	}
*/
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;//ADM2IF_ntBASEDAC;
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
/*	{ // Master/Slave
		param.tetr = m_MainTetrNum;//ADM2IF_ntMAIN;
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
//		else
//		{ // Slave
//			source = BRD_clksEXTSYNX;
//		}
	}*/
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetInpSrc(CModule* pModule, ULONG& adc_num, ULONG chan_mask)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::GetInpSrc(CModule* pModule, ULONG& adc_num, ULONG chan_mask)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::SetFC(CModule* pModule, double& Freq, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::GetFC(CModule* pModule, double& Freq, ULONG Chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Freq = 0.0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::SetFrame(CModule* pModule, ULONG Len)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::GetFrame(CModule* pModule, ULONG& Len)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Len = 0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::SetProgram(CModule* pModule, PVOID args)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::SetDDCSync(CModule* pModule, ULONG mode, ULONG prog, ULONG async)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::GetDDCSync(CModule* pModule, ULONG& mode, ULONG& async)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	return Status;
}

//***************************************************************************************
//int CDdcSrv::SetDecim(CModule* pModule, ULONG& decim, ULONG chan)
int CDdcSrv::SetDecim(CModule* pModule, double& decim, ULONG chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
//int CDdcSrv::GetDecim(CModule* pModule, ULONG& decim, ULONG chan)
int CDdcSrv::GetDecim(CModule* pModule, double& decim, ULONG chan)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	return Status;
}

//***************************************************************************************
int CDdcSrv::SetTestMode(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Test = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetTestMode(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	mode = pMode1->ByBits.Test;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetSpecific(CModule* pModule, PBRD_DdcSpec pSpec)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::GetSpecific(CModule* pModule, PBRD_DdcSpec pSpec)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
//	mode = 0;
//	int Status = BRDerr_OK;
	return Status;
}

//***************************************************************************************
int CDdcSrv::SetTarget(CModule* pModule, ULONG target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Out = target;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetTarget(CModule* pModule, ULONG& target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	target = pMode1->ByBits.Out;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::StartEnable(CModule* pModule, ULONG Enbl)
{
	U32			val, flag;

	// ”становить бит Start регистра MODE0
	IndRegRead( pModule, m_DdcTetrNum, ADM2IFnr_MODE0, &val );
	val &= ~0x20;
	val |= (Enbl & 0x1) << 5;	
	IndRegWrite( pModule, m_DdcTetrNum, ADM2IFnr_MODE0, val );

	
	// ≈сли не установлен бит Master регистра MODE0
	flag = (Enbl & 2) >> 1;
	if( 0==(val&0x10) && flag )
	{
		IndRegRead( pModule, m_MainTetrNum, ADM2IFnr_MODE0, &val );
		val &= ~0x10;
		val |= (Enbl & 0x1) << 4;	
		IndRegWrite( pModule, m_MainTetrNum, ADM2IFnr_MODE0, val );
	}

	return BRDerr_OK;

	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;
	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	////pMode0->ByBits.Start = Enbl;
	////pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	////if(!pMode0->ByBits.Master)
	////{ // Master/Slave
	////	param.tetr = m_MainTetrNum;
	////	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	////	pMode0->ByBits.Start = Enbl;
	////	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	////}
	//pMode0->ByBits.Start = Enbl & 1;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//ULONG flag = (Enbl & 2) >> 1;
	//if(!pMode0->ByBits.Master && flag)
	//{ // Master/Slave
	//	param.tetr = m_MainTetrNum;
	//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//	pMode0->ByBits.Start = Enbl & 1;
	//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//}
	//return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::PrepareStart(CModule* pModule, void *arg)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode)
{
	if(m_AdmConst.ByBits.Mod != 5)
		return BRDerr_CMD_UNSUPPORTED;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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
	pTlMode->ByBits.Size = pTitleMode->value * sizeof(ULONG) / widthFifo; // pTitleMode->value - размер заголовка в 32-разр€дных словах
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pTitleMode->value = pTlMode->ByBits.Size * widthFifo / sizeof(ULONG); // pTitleMode->value - размер заголовка в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode)
{
	if(m_AdmConst.ByBits.Mod != 5)
		return BRDerr_CMD_UNSUPPORTED;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;

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
int CDdcSrv::SetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal)
{
	if(m_AdmConst.ByBits.Mod != 5)
		return BRDerr_CMD_UNSUPPORTED;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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
	pEnVal->value = param.val * widthFifo / sizeof(ULONG); // pEnVal->value - в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal)
{
	if(m_AdmConst.ByBits.Mod != 5)
		return BRDerr_CMD_UNSUPPORTED;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
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
	pEnVal->value = param.val * widthFifo / sizeof(ULONG); // pEnVal->value - в 32-битных словах

	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::SetTitleData(CModule* pModule, PULONG pTitleData)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_TLDATAL;
	param.val = (*pTitleData) & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = DDCnr_TLDATAH;
	param.val = ((*pTitleData) >> 16) & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdcSrv::GetTitleData(CModule* pModule, PULONG pTitleData)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_TLDATAL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG tldata = param.val & 0xffff;
	param.reg = DDCnr_TLDATAH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	tldata |= param.val << 16;
	*pTitleData = tldata;
	return BRDerr_OK;
}

// ***************** End of file DdcSrv.cpp ***********************
