// clang-format off
/*
 ***************** File DdcSrv.cpp ************
 *
 * BRD Driver for DDС service on DDС submodule
 *
 * (C) InSys by Dorokhin A. Jan 2006
 *
 * 12.10.2006 - submodule ver 3.0 - synchronous mode
 *
 ******************************************************
*/

#include "module.h"
#include "ddc4x16srv.h"

#define	CURRFILE _BRDC("DDC4X16SRV")

//***************************************************************************************
CDdc4x16Srv::CDdc4x16Srv(int idx, int srv_num, CModule* pModule, PDDC4X16SRV_CFG pCfg) :
	CDdcSrv(idx, _BRDC("DDC4X16"), srv_num, pModule, pCfg, sizeof(DDC4X16SRV_CFG))
{
}

//***************************************************************************************
int CDdc4x16Srv::CtrlRelease(
								void			*pDev,		// InfoSM or InfoBM
								void			*pServData,	// Specific Service Data
								ULONG			cmd,		// Command Code (from BRD_ctrl())
								void			*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	CDdcSrv::SetChanMask(pModule, 0);
//	return BRDerr_OK;
	return BRDerr_CMD_UNSUPPORTED; // для освобождения подслужб
}

//***************************************************************************************
void CDdc4x16Srv::GetDdcTetrNum(CModule* pModule)
{
	m_DdcTetrNum = GetTetrNum(pModule, m_index, DDC4x16_TETR_ID);
	if(m_DdcTetrNum == -1)
		m_DdcTetrNum = GetTetrNum(pModule, m_index, DDC4x16S_TETR_ID);
}

//***************************************************************************************
void CDdc4x16Srv::FreeInfoForDialog(PVOID pInfo)
{
	PDDCSRV_INFO pSrvInfo = (PDDCSRV_INFO)pInfo;
	delete pSrvInfo;
}

//***************************************************************************************
PVOID CDdc4x16Srv::GetInfoForDialog(CModule* pDev)
{
	PDDC4X16SRV_INFO pSrvInfo = new DDC4X16SRV_INFO;
	pSrvInfo->Size = sizeof(DDC4X16SRV_INFO);
	pSrvInfo->pDdcInfo = (PDDCSRV_INFO)CDdcSrv::GetInfoForDialog(pDev);

	return pSrvInfo;
}

//***************************************************************************************
int CDdc4x16Srv::SetPropertyFromDialog(void	*pDev, void	*args)
{
	CModule* pModule = (CModule*)pDev;
	PDDC4X16SRV_INFO pInfo = (PDDC4X16SRV_INFO)args;

	CDdcSrv::SetChanMask(pModule, pInfo->pDdcInfo->ChanMask);
	ULONG master = pInfo->pDdcInfo->SyncMode;
	CtrlMaster(pDev, NULL, BRDctrl_DDC_SETMASTER, &master);

	PDDC4X16SRV_CFG pDdcSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	pDdcSrvCfg->SubExtClk = ROUND(pInfo->pDdcInfo->ExtClk);
	SetClkSource(pModule, pInfo->pDdcInfo->ClockSrc);
	SetClkValue(pModule, pInfo->pDdcInfo->ClockSrc, pInfo->pDdcInfo->ClockValue);
	SetRate(pModule, pInfo->pDdcInfo->SamplingRate, pInfo->pDdcInfo->ClockSrc, pInfo->pDdcInfo->ClockValue);

	return BRDerr_OK;
}
/*
static int ReadPrgFile(char *file, PULONG dst)
{
    FILE    *stream;
	ULONG	page,adr,data;
	LONG	ind=0;
    char    *endptr;
	char    str[256];

//	stream=fopen(file,"rt");
//	if(stream == NULL)
	errno_t err = fopen_s(&stream, file,"rt");
	if(err != NULL)
		return ind;
    fgets(str, 80, stream );
    fgets(str, 80, stream );
    fgets(str, 80, stream );
//	Decimation=atoi(g_str+21);
    fgets(str, 80, stream );
    fgets(str, 80, stream );
    fgets(str, 80, stream );
    fgets(str, 80, stream );
    while(!feof(stream))
    {
        fgets(str, 80, stream );
       	page = strtoul(str, &endptr, 0 );
       	adr  = strtoul(endptr, &endptr, 0 );
       	data = strtoul(endptr, &endptr, 0 );
       	dst[ind]=((page&0xff)<<16) | ((adr&0xff)<<8) | (data&0xff);
        ind++;
    }
//	 printf("\nEnd PrgFile");
//   printf( "\nDecimation= %d") ,Decimation );
//   printf( "\nOutRateDDC= %d") ,g_examParam.aMCLOCK/Decimation );
//   printf( "\nMCLOCK= %d") ,g_examParam.aMCLOCK);
    return	ind;
}
*/
//********************************************************
static int CheckPrgFile(BRDCHAR *file)
{
	// удалить комментарий из строки
	BRDCHAR* pChar = BRDC_strchr(file, ';'); // признак комментария или ;
	if(pChar) *pChar = 0;
	pChar = BRDC_strchr(file, '/');			// или //
	if( pChar ) if( *(pChar+1)=='/' )	*pChar = 0;

	// Удалить пробелы в конце строки
	int str_size = (int)BRDC_strlen(file);
	for(int i = str_size - 1; i > 1; i--)
		if(file[i] != ' ' && file[i] != '\t')
		{
			file[i+1] = 0;
			break;
		}

    FILE* stream;
	stream = BRDC_fopen(file, _BRDC("rt"));
	if(stream == NULL)
		return 0;
	fclose(stream);
	return 1;
}

//********************************************************
static int ReadPrgFile(BRDCHAR *file, PULONG dst, PPRGFILE_PARAM pParam, ULONG Mod)
{
    FILE    *stream;
	ULONG	page,adr,data;
	LONG	ind = 0;
    char    *endptr;
	char    str[256];

	stream = BRDC_fopen(file, _BRDC("rt"));
	if(stream == NULL)
		return ind;
	fgets(str, 80, stream );

	while(strncmp(str, "Page Reg Val", 12) != 0)
    {
		if(strncmp(str,"Clock=" ,6) == 0)
		{
			int     base = 10;
			char    *s = str+6;

			while(isspace(*s)) s++;
			if((s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
				base = 16;
			pParam->RefClk = strtoul(str+6, &endptr, base);
		}

		//if(Mod != 5)
		if(Mod == 2)
		{ // в синхронной прошивке отсутствует
			if(strncmp(str,"AsyncMode=" ,10) == 0)
			{
				int     base = 10;
				char    *s = str+10;

				while(isspace(*s)) s++;
				if((s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
					base = 16;
				pParam->ASyncMode = strtoul(str+10, &endptr, base);
			}

			if(strncmp(str,"MaskDdc=" ,8) == 0)
			{
				int     base = 10;
				char    *s = str+8;

				while (isspace(*s)) s++;
				if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
					base = 16;
				pParam->DdcMask = strtoul(str+8, &endptr, base);
			}
		}
		if(strncmp(str,"Decimation Channel0=" ,20) == 0)
		{
			pParam->Decim[0] = atof(str+20);
			//while (isspace(*s)) s++;
			//if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
			//	base = 16;
			//pParam->Decim[0] = strtoul(str+20, &endptr, base);
		}

		if(strncmp(str,"Decimation Channel1=" ,20) == 0)
		{
			pParam->Decim[1] = atof(str+20);
			//while (isspace(*s)) s++;
			//if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
			//	base = 16;
			//pParam->Decim[1] = strtoul(str+20, &endptr, base);
		}

		if(strncmp(str,"Decimation Channel2=" ,20) == 0)
		{
			pParam->Decim[2] = atof(str+20);
			//while (isspace(*s)) s++;
			//if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
			//	base = 16;
			//pParam->Decim[2] = strtoul(str+20, &endptr, base );
		}


		if(strncmp(str,"Decimation Channel3=" ,20) == 0)
		{
			pParam->Decim[3] = atof(str+20);
			//while (isspace(*s)) s++;
			//if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
			//	base = 16;
			//pParam->Decim[3] = strtoul(str+20, &endptr, base);
		}

		//if(Mod != 5)
		if(Mod == 2)
		{ // в синхронной прошивке отсутствует
			if(strncmp(str,"ModeOfDdc=" ,10) == 0)
			{
				int     base = 10;
				char    *s = str+10;

				while (isspace(*s)) s++;
				if( (s[0]=='0') && ((s[1]=='x') || (s[1]=='X')) )
					base = 16;

				pParam->ModeOfDdc = strtoul(str+10, &endptr, base);
			}
		}
		fgets(str, 80, stream);

    }

    while( !feof(stream) )
    {
        fgets(str, 80, stream );
       	page = strtoul(str, &endptr, 0);
       	adr  = strtoul(endptr, &endptr, 0);
       	data = strtoul(endptr, &endptr, 0);
       	dst[ind]=((page&0xff)<<16) | ((adr&0xff)<<8) | (data&0xff);
        ind++;
    }
 //    printf( "\nDecimation= %d") ,Decimation );
 //    printf( "\nOutRateDDC= %d") ,g_examParam.aMCLOCK/Decimation );
	fclose(stream);
    return	ind;
}

//***************************************************************************************
int CDdc4x16Srv::SetProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	BRDCHAR Buffer[128];
	BRDCHAR ParamName[128];
	BRDCHAR* endptr;
	CDdcSrv::SetProperties(pDev, iniFilePath, SectionName);

	ULONG chanMask = 1;
	IPC_getPrivateProfileString(SectionName, _BRDC("DdcChannelMask") ,_BRDC("1") ,Buffer, sizeof(Buffer), iniFilePath);
	chanMask = BRDC_strtol(Buffer, &endptr, 0);
	if(m_AdmConst.ByBits.Mod == 5)
	{
		CDdcSrv::SetChanMask(pDev, chanMask);

		BRD_Adc4x16Mode AdcMode;
		IPC_getPrivateProfileString(SectionName, _BRDC("AdcGainMask") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
		AdcMode.gainMask = BRDC_strtol(Buffer, &endptr, 0);
		IPC_getPrivateProfileString(SectionName, _BRDC("AdcBits") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
		AdcMode.bits = BRDC_atoi(Buffer);
		IPC_getPrivateProfileString(SectionName, _BRDC("Randomization") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
		AdcMode.rand = BRDC_atoi(Buffer);
		IPC_getPrivateProfileString(SectionName, _BRDC("Dither") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
		AdcMode.dither = BRDC_atoi(Buffer);
		SetAdcMode(pDev, &AdcMode);

		IPC_getPrivateProfileString(SectionName, _BRDC("DataAlign") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
		ULONG align = BRDC_atoi(Buffer);
		CtrlAlign(pDev, DDC4X16cmd_SETALIGN, &align);
	}

	IPC_getPrivateProfileString(SectionName, _BRDC("ExternalClockValue") ,_BRDC("80000000.0") ,Buffer, sizeof(Buffer), iniFilePath);
	double clkValue = BRDC_atof(Buffer);
	SetClkValue(pDev, BRDclks_DDC_EXTCLK, clkValue);

	IPC_getPrivateProfileString(SectionName, _BRDC("ClockSource") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	ULONG clkSrc = BRDC_strtol(Buffer, &endptr, 0);
	if(clkSrc)
		clkSrc = BRDclks_DDC_EXTCLK;
//	ULONG clkSrc = atoi(Buffer);
	SetClkSource(pDev, clkSrc);
	if(clkSrc != BRDclks_DDC_EXTCLK)
		SetClkValue(pDev, clkSrc, clkValue);
	GetClkValue(pDev, clkSrc, clkValue);

	IPC_getPrivateProfileString(SectionName, _BRDC("SamplingRate") ,_BRDC("100000000.0") ,Buffer, sizeof(Buffer), iniFilePath);
	double rate = BRDC_atof(Buffer);
	SetRate(pDev, rate, clkSrc, clkValue);

	PDDC4X16SRV_CFG pSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	int num_ddc = pSrvCfg->DdcCfg.MaxChan / 4;
	ULONG ddcMask = 0;
	ULONG Async_mode = 0;
	//ULONG Decim[BRD_DDC_CHANCNT];
	double Decim[BRD_DDC_CHANCNT];
	ULONG ModeOfDdc = 0;

	IPC_getPrivateProfileString(SectionName, _BRDC("DdcProgramFile") ,_BRDC("DdcProgram.prg") ,Buffer, sizeof(Buffer), iniFilePath);
	if(CheckPrgFile(Buffer))
	{
		//if(m_AdmConst.ByBits.Mod != 5)
		if(m_AdmConst.ByBits.Mod == 2)
			ddcMask = 0xf;
		PULONG pDdcData = new ULONG[1024];
		PRGFILE_PARAM prg_param;
		prg_param.ASyncMode = 0;
		prg_param.ModeOfDdc = 0;
		int nData = ReadPrgFile(Buffer, pDdcData, &prg_param, m_AdmConst.ByBits.Mod);
//		if(prg_param.RefClk != ROUND(clkValue))
//			return BRDerr_DDC_INVALID_PRGFLCLK;
		Async_mode = prg_param.ASyncMode;
		for(int iDDC = 0; iDDC < num_ddc; iDDC++)
		{
			//if(m_AdmConst.ByBits.Mod != 5)
			if(m_AdmConst.ByBits.Mod == 2)
				chanMask |= prg_param.DdcMask << (4*iDDC);
			else
			{
				if(chanMask & (0xf << (4*iDDC)))
					ddcMask |= (1 << iDDC);
			}
			for(int iChan = 0; iChan < 4; iChan++)
				Decim[4*iDDC + iChan] = prg_param.Decim[iChan];
		}
		ModeOfDdc = prg_param.ModeOfDdc;
		BRD_DdcProgram ddc_prog;
		ddc_prog.mask = ddcMask;
		ddc_prog.num = nData;
		ddc_prog.pRecords = (U32*)pDdcData;
		SetProgram(pDev, &ddc_prog);
		delete[] pDdcData;
	}
	else
	{
		BRDCHAR NamePrg0[128];
		BRDCHAR NamePrg1[128];
		BRDCHAR NamePrg2[128];
		BRDCHAR NamePrg3[128];
		IPC_getPrivateProfileString(SectionName, _BRDC("DdcProgramFile0") ,_BRDC("DdcProgram0.prg") ,NamePrg0, sizeof(NamePrg0), iniFilePath);
		if(CheckPrgFile(NamePrg0))
			ddcMask |= 0x1;
		else
			return BRDerr_DDC_PRGFILE_NOT;
		IPC_getPrivateProfileString(SectionName, _BRDC("DdcProgramFile1") ,_BRDC("DdcProgram1.prg") ,NamePrg1, sizeof(NamePrg1), iniFilePath);
		if(CheckPrgFile(NamePrg1))
			ddcMask |= 0x2;
		IPC_getPrivateProfileString(SectionName, _BRDC("DdcProgramFile2") ,_BRDC("DdcProgram2.prg") ,NamePrg2, sizeof(NamePrg2), iniFilePath);
		if(CheckPrgFile(NamePrg2))
			ddcMask |= 0x4;
		IPC_getPrivateProfileString(SectionName, _BRDC("DdcProgramFile3") ,_BRDC("DdcProgram3.prg") ,NamePrg3, sizeof(NamePrg3), iniFilePath);
		if(CheckPrgFile(NamePrg3))
			ddcMask |= 0x8;

		PULONG pDdcData = new ULONG[1024];
		BRD_DdcProgram ddc_prog;
		PRGFILE_PARAM prg_param;
		for(int iDDC = 0; iDDC < num_ddc; iDDC++)
		{
			prg_param.ASyncMode = 0;
			prg_param.ModeOfDdc = 0;
			ULONG mask = 1 << iDDC;
			if(ddcMask & mask)
			{
				int nData;
				switch(iDDC)
				{
				case 0:
				nData = ReadPrgFile(NamePrg0, pDdcData, &prg_param, m_AdmConst.ByBits.Mod);
				break;
				case 1:
				nData = ReadPrgFile(NamePrg1, pDdcData, &prg_param, m_AdmConst.ByBits.Mod);
				break;
				case 2:
				nData = ReadPrgFile(NamePrg2, pDdcData, &prg_param, m_AdmConst.ByBits.Mod);
				break;
				case 3:
				nData = ReadPrgFile(NamePrg3, pDdcData, &prg_param, m_AdmConst.ByBits.Mod);
				break;
				}
//				if(prg_param.RefClk != ROUND(clkValue))
//					return BRDerr_DDC_INVALID_PRGFLCLK;
				if(!iDDC)
				{
					Async_mode = prg_param.ASyncMode;
					ModeOfDdc = prg_param.ModeOfDdc;
				}
				else
				{
					if(prg_param.ASyncMode != Async_mode)
						return BRDerr_DDC_INVALID_PRGFLASYNC;
					if(prg_param.ModeOfDdc != ModeOfDdc)
						return BRDerr_DDC_INVALID_MODEOFDDC;
				}
				//if(m_AdmConst.ByBits.Mod != 5)
				if(m_AdmConst.ByBits.Mod == 2)
					chanMask |= prg_param.DdcMask << (4*iDDC);
				for(int iChan = 0; iChan < 4; iChan++)
					Decim[4*iDDC + iChan] = prg_param.Decim[iChan];
				ddc_prog.mask = mask;
				ddc_prog.num = nData;
				ddc_prog.pRecords = (U32*)pDdcData;
				SetProgram(pDev, &ddc_prog);
			}
		}
		delete[] pDdcData;
	}

	BRDCHAR DefFcBuf[128];
//	IPC_getPrivateProfileString(SectionName, _BRDC("FrequencyNCO") ,_BRDC("10700000.0") ,DefFcBuf, sizeof(DefFcBuf), iniFilePath);
	IPC_getPrivateProfileString(SectionName, _BRDC("FrequencyNCO") ,_BRDC("-1.0") ,DefFcBuf, sizeof(DefFcBuf), iniFilePath);
//	double fc = atof(DefFcBuf);
//	for(int iDDC = 0; iDDC < pSrvCfg->DdcCfg.MaxChan; iDDC++)
//		SetFC(pDev, fc, iDDC);

	BRDCHAR DefInpBuf[128];
	IPC_getPrivateProfileString(SectionName, _BRDC("InputSource") ,_BRDC("-1") ,DefInpBuf, sizeof(DefInpBuf), iniFilePath);

//	BRDCHAR DefDecim[128];
//	IPC_getPrivateProfileString(SectionName, _BRDC("Decimation") ,_BRDC("1") ,DefDecim, sizeof(DefInpBuf), iniFilePath);
//	ULONG inpSrc = atoi(Buffer);
//	for(iDDC = 0; iDDC < pSrvCfg->DdcCfg.MaxChan; iDDC++)
//		SetInpSrc(pDev, inpSrc, iDDC);

	//if(m_AdmConst.ByBits.Mod != 5)
	if(m_AdmConst.ByBits.Mod != 2)
		CDdcSrv::SetChanMask(pDev, chanMask);

	for(int iChan = 0; iChan < pSrvCfg->DdcCfg.MaxChan; iChan++)
	{
		int num = (iChan >> ModeOfDdc) << ModeOfDdc;

		BRDC_sprintf(ParamName, _BRDC("FrequencyNCO%d") ,num);
		IPC_getPrivateProfileString(SectionName, ParamName, DefFcBuf, Buffer, sizeof(Buffer), iniFilePath);
		double fc = BRDC_atof(Buffer);
		if(fc > 0)
			SetFC(pDev, fc, iChan);

		//sprintf(ParamName, _BRDC("Decimation%d") ,iDDC);
		//IPC_getPrivateProfileString(SectionName, ParamName, DefDecim, Buffer, sizeof(Buffer), iniFilePath);
		//ULONG decim = atoi(DefDecim);
		SetDecim(pDev, Decim[num], iChan);

		ULONG inpSrc;
		BRDC_sprintf(ParamName, _BRDC("InputSource%d") ,iChan);
		IPC_getPrivateProfileString(SectionName, ParamName, DefInpBuf, Buffer, sizeof(Buffer), iniFilePath);
		LONG tmp_inpSrc = inpSrc = BRDC_atoi(Buffer);
		if(tmp_inpSrc >= 0)
			SetInpSrc(pDev, inpSrc, iChan);
	}

	IPC_getPrivateProfileString(SectionName, _BRDC("DdcSyncMode") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	ULONG DDC_sync_mode = BRDC_atoi(Buffer);
//	IPC_getPrivateProfileString(SectionName, _BRDC("AsyncMode") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
//	ULONG Async_mode = atoi(Buffer);
	SetDDCSync(pDev, DDC_sync_mode, 0, Async_mode);

	IPC_getPrivateProfileString(SectionName, _BRDC("DataFormat") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	ULONG format = BRDC_atoi(Buffer);
	SetFormat(pDev, format);

	ULONG sample_size = format ? format : 2;
	sample_size <<= 1;
	int chans = 0;
	for(ULONG i = 0; i < 16; i++)
		chans += (chanMask >> i) & 0x1;

	BRD_EnVal start_delay;
	IPC_getPrivateProfileString(SectionName, _BRDC("StartDelayEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_delay.enable = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("StartDelayCounter") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	start_delay.value = BRDC_atoi(Buffer);
	start_delay.value = USHORT(start_delay.value * sample_size * chans / sizeof(ULONG)); // было в отсчетах на канал, стало в 32-битных словах
	SetCnt(pDev, 0, &start_delay);

	BRD_EnVal acq_data;
	IPC_getPrivateProfileString(SectionName, _BRDC("AcquiredSampleEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	acq_data.enable = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("AcquiredSampleCounter") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	acq_data.value = BRDC_atoi(Buffer);
	acq_data.value = USHORT(acq_data.value * sample_size * chans / sizeof(ULONG)); // было в отсчетах на канал, стало в 32-битных словах
	SetCnt(pDev, 1, &acq_data);

	BRD_EnVal skip_data;
	IPC_getPrivateProfileString(SectionName, _BRDC("SkipSampleEnable") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	skip_data.enable = BRDC_atoi(Buffer);
	IPC_getPrivateProfileString(SectionName, _BRDC("SkipSampleCounter") ,_BRDC("0") ,Buffer, sizeof(Buffer), iniFilePath);
	skip_data.value = BRDC_atoi(Buffer);
	skip_data.value = USHORT(skip_data.value * sample_size * chans / sizeof(ULONG)); // было в отсчетах на канал, стало в 32-битных словах
	SetCnt(pDev, 2, &skip_data);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::SaveProperties(CModule* pDev, BRDCHAR* iniFilePath, BRDCHAR* SectionName)
{
	//BRDCHAR Buffer[128];
	BRDCHAR ParamName[128];
	CDdcSrv::SaveProperties(pDev, iniFilePath, SectionName);

	ULONG clkSrc;
	GetClkSource(pDev, clkSrc);
	//BRDC_sprintf(Buffer, _BRDC("%u") ,clkSrc);
	//WritePrivateProfileString(SectionName, _BRDC("ClockSource") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("ClockSource"), clkSrc, 10, NULL);
	double clkValue;
	GetClkValue(pDev, clkSrc, clkValue);
	if(clkSrc == BRDclks_DDC_EXTCLK)
	{
		//BRDC_sprintf(Buffer, _BRDC("%.2f") ,clkValue);
		//WritePrivateProfileString(SectionName, _BRDC("ExternalClockValue") ,Buffer, iniFilePath);
		WriteInifileParam(iniFilePath, SectionName, _BRDC("ExternalClockValue"), clkValue, 2, NULL);
	}
	double rate;
	GetRate(pDev, rate, clkValue);
	//BRDC_sprintf(Buffer, _BRDC("%.2f") ,rate);
	//WritePrivateProfileString(SectionName, _BRDC("SamplingRate") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("SamplingRate"), rate, 2, NULL);

	double fc_def;
	GetFC(pDev, fc_def, 0);
	//BRDC_sprintf(Buffer, _BRDC("%.2f") ,fc_def);
	//WritePrivateProfileString(SectionName, _BRDC("FrequencyNCO") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("FrequencyNCO"), fc_def, 2, NULL);

	ULONG inpSrc_def;
	GetInpSrc(pDev, inpSrc_def, 0);
	//BRDC_sprintf(Buffer, _BRDC("%u") ,inpSrc_def);
	//WritePrivateProfileString(SectionName, _BRDC("InputSource") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("InputSource"), inpSrc_def, 10, NULL);

	//ULONG decim_def;
	double decim_def;
	GetDecim(pDev, decim_def, 0);
	//sprintf_s(Buffer, _BRDC("%u") ,decim_def);
	//BRDC_sprintf(Buffer, _BRDC("%.2f") ,decim_def);
	//WritePrivateProfileString(SectionName, _BRDC("Decimation") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("Decimation"), decim_def, 2, NULL);

	double fc;
	ULONG inpSrc;
	double decim;
	PDDC4X16SRV_CFG pSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	for(int iDDC = 1; iDDC < pSrvCfg->DdcCfg.MaxChan; iDDC++)
	{
		GetFC(pDev, fc, iDDC);
		if(fc != fc_def)
		{
			BRDC_sprintf(ParamName, _BRDC("FrequencyNCO%d") ,iDDC);
			//BRDC_sprintf(Buffer, _BRDC("%.2f") ,fc);
			//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
			WriteInifileParam(iniFilePath, SectionName, ParamName, fc, 2, NULL);
		}
		GetInpSrc(pDev, inpSrc, iDDC);
		if(inpSrc != inpSrc_def)
		{
			BRDC_sprintf(ParamName, _BRDC("InputSource%d") ,iDDC);
			//BRDC_sprintf(Buffer, _BRDC("%u") ,inpSrc);
			//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
			WriteInifileParam(iniFilePath, SectionName, ParamName, inpSrc, 10, NULL);
		}
		GetDecim(pDev, decim, iDDC);
		if(decim != decim_def)
		{
			//sprintf_s(Buffer, _BRDC("%u") ,decim);
			BRDC_sprintf(ParamName, _BRDC("Decimation%d") ,iDDC);
			//BRDC_sprintf(Buffer, _BRDC("%.2f") ,decim);
			//WritePrivateProfileString(SectionName, ParamName, Buffer, iniFilePath);
			WriteInifileParam(iniFilePath, SectionName, ParamName, decim, 2, NULL);
		}
	}

	ULONG DDC_sync_mode;
	ULONG Async_mode;
	GetDDCSync(pDev, DDC_sync_mode, Async_mode);
	//BRDC_sprintf(Buffer, _BRDC("%u") ,DDC_sync_mode);
	//WritePrivateProfileString(SectionName, _BRDC("DdcSyncMode") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("DdcSyncMode"), DDC_sync_mode, 10, NULL);
	//BRDC_sprintf(Buffer, _BRDC("%u") ,Async_mode);
	//WriteInifileParam(SectionName, _BRDC("AsyncMode") ,Buffer, iniFilePath);
	WriteInifileParam(iniFilePath, SectionName, _BRDC("AsyncMode"), Async_mode, 10, NULL);

	// the function flushes the cache
	IPC_writePrivateProfileString(NULL, NULL, NULL, iniFilePath);
	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::SetClkSource(CModule* pModule, ULONG ClkSrc)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_FSRC;
	param.val = ClkSrc;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//Sleep(1);
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(!pMode0->ByBits.Master)
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_FMODE;//ADM2IFnr_FSRC;
			param.val = BRDclks_SMCLK;//ClkSrc;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		else
			return BRDerr_NOT_ACTION; // функция в режиме Slave не выполнима
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::GetClkSource(CModule* pModule, ULONG& ClkSrc)
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
		param.reg = ADM2IFnr_FSRC;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		source = param.val;
	}
	else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.tetr = m_DdcTetrNum;
			param.reg = ADM2IFnr_FSRC;
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
int CDdc4x16Srv::SetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PDDC4X16SRV_CFG pDdcSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	switch(ClkSrc)
	{
	case BRDclks_DDC_REFGEN:		// Submodule Reference Generator
		ClkValue = pDdcSrvCfg->SubRefGen;
		break;
	case BRDclks_EXTSYNX:			// External clock from SYNX
	case BRDclks_DDC_EXTCLK:		// External clock
		pDdcSrvCfg->SubExtClk = ROUND(ClkValue);
		break;
	default:
//		ClkValue = 0.0;
		break;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::GetClkValue(CModule* pModule, ULONG ClkSrc, double& ClkValue)
{
	PDDC4X16SRV_CFG pDdcSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	double Clk;
	switch(ClkSrc)
	{
	case BRDclks_DDC_REFGEN:		// Submodule Reference Generator
		Clk = pDdcSrvCfg->SubRefGen;
		break;
	case BRDclks_EXTSYNX:			// External clock from SYNX
	case BRDclks_DDC_EXTCLK:		// External clock
		Clk = pDdcSrvCfg->SubExtClk;
		break;
	default:
		Clk = pDdcSrvCfg->SubRefGen;
		//Clk = 0.0;
		break;
	}
	ClkValue = Clk;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::SetRate(CModule* pModule, double& Rate, ULONG ClkSrc, double ClkValue)
{
	Rate = ClkValue;
	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::GetRate(CModule* pModule, double& Rate, double ClkValue)
{
	Rate = ClkValue;
	return BRDerr_OK;
}

//***************************************************************************************
void CDdc4x16Srv::ProgDdcReg(CModule* pModule, ULONG cmd, ULONG chipMask, ULONG addr, ULONG& data)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_ADR;
	PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	pAdr->ByBits.RegAdr = addr;			//
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_WRDATA;
	PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	pData->ByBits.Cmd = cmd; // 0 - writing, 1 - reading
	if(cmd)
	{
		pData->ByBits.Data = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = DDCnr_RDDATA;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PDDC_RDDATA pReadData = (PDDC_RDDATA)&param.val;
		data = pReadData->ByBits.Data;
	}
	else
	{
		pData->ByBits.Data = data;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
}

//***************************************************************************************
// AdcNum[0...3]
// DdcChan[0...15]
int CDdc4x16Srv::SetInpSrc(CModule* pModule, ULONG& AdcNum, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG data = (7+ChanNum*8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, data);
	ProgDdcReg(pModule, 0, chipMask, 27, AdcNum);
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;

	//param.reg = DDCnr_ADR;
	//PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = (7+ChanNum*8) << 1; // Channel Control page
	//pData->ByBits.Cmd = 0; // writing
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_ADR;
	//pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = 27;			// Input (Reg 27)
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = AdcNum;
	//pData->ByBits.Cmd = 0; // writing
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::GetInpSrc(CModule* pModule, ULONG& AdcNum, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG data = (7+ChanNum*8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, data);
	ProgDdcReg(pModule, 1, chipMask, 27, AdcNum);
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;

	//param.reg = DDCnr_ADR;
	//PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = (7+ChanNum*8) << 1; // Channel Control page
	//pData->ByBits.Cmd = 0; // writing
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_ADR;
	//pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = 27;			// Input (Reg 27)
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = 0;
	//pData->ByBits.Cmd = 1; // reading
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_RDDATA;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PDDC_RDDATA pReadData = (PDDC_RDDATA)&param.val;
	//AdcNum = pReadData->ByBits.Data;

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::SetFC(CModule* pModule, double& Freq, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG src;
	Status = GetClkSource(pModule, src);
	double clkValue;
	Status = GetClkValue(pModule, src, clkValue);
	if(clkValue == 0.0) clkValue = 100000000.; // чтобы у Аникина не было деления на 0
	//ULONG NcoFreq = UINT(pow(2.0, 32) * (Freq / clkValue));
	ULONG NcoFreq = UINT(pow(2.0, 32) * (Freq / clkValue)+0.5);

	ULONG page = (6 + ChanNum * 8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, page);

	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;

	//param.reg = DDCnr_ADR;
	//PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = (6+ChanNum*8) << 1; // Channel Frequency page
	//pData->ByBits.Cmd = 0; // writing
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	for(int i = 0; i < 4; i++)
	{
		ULONG data = (NcoFreq >> (i * 8)) & 0xff;
		ProgDdcReg(pModule, 0, chipMask, 18 + i, data);
		//param.reg = DDCnr_ADR;
		//pAdr = (PDDC_ADR)&param.val;
		//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
		//pAdr->ByBits.RegAdr = 18 + i;
		//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		//param.reg = DDCnr_WRDATA;
		//pData = (PDDC_WRDATA)&param.val;
		//pData->ByBits.Data = (NcoFreq >> (i * 8)) & 0xff;
		//pData->ByBits.Cmd = 0; // writing
		//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_ADR;
	PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	pAdr->ByBits.Mask = 0xf;			// выбираем все микросхемы DDC для программирования
	pAdr->ByBits.RegAdr = 5;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_WRDATA;
	PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	pData->ByBits.Data = 0x98;
	pData->ByBits.Cmd = 0; // writing
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pData->ByBits.Data = 0x18;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::GetFC(CModule* pModule, double& Freq, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG page = (6 + ChanNum * 8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, page);

	ULONG NcoFreq = 0;
	for(int i = 0; i < 4; i++)
	{
		ULONG data;
		ProgDdcReg(pModule, 1, chipMask, 18 + i, data);
		NcoFreq |= data << (i * 8);
	}

	ULONG src;
	Status = GetClkSource(pModule, src);
	double clkValue;
	Status = GetClkValue(pModule, src, clkValue);

	Freq = NcoFreq / pow(2.0, 32) * clkValue;

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::SetFcCode(CModule* pModule, U32& NcoFreq, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	//ULONG src;
	//Status = GetClkSource(pModule, src);
	//double clkValue;
	//Status = GetClkValue(pModule, src, clkValue);
	//ULONG NcoFreq = UINT(pow(2.0, 32) * (Freq / clkValue));

	ULONG page = (6 + ChanNum * 8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, page);

	for(int i = 0; i < 4; i++)
	{
		ULONG data = (NcoFreq >> (i * 8)) & 0xff;
		ProgDdcReg(pModule, 0, chipMask, 18 + i, data);
	}

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_ADR;
	PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	pAdr->ByBits.Mask = 0xf;			// выбираем все микросхемы DDC для программирования
	pAdr->ByBits.RegAdr = 5;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_WRDATA;
	PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	pData->ByBits.Data = 0x98;
	pData->ByBits.Cmd = 0; // writing
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pData->ByBits.Data = 0x18;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::GetFcCode(CModule* pModule, U32& NcoFreq, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG page = (6 + ChanNum * 8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, page);

//	ULONG NcoFreq = 0;
	for(int i = 0; i < 4; i++)
	{
		ULONG data;
		ProgDdcReg(pModule, 1, chipMask, 18 + i, data);
		NcoFreq |= data << (i * 8);
	}

	//ULONG src;
	//Status = GetClkSource(pModule, src);
	//double clkValue;
	//Status = GetClkValue(pModule, src, clkValue);

	//Freq = NcoFreq / pow(2.0, 32) * clkValue;

	return Status;
}

//***************************************************************************************
//int CDdc4x16Srv::SetDecim(CModule* pModule, ULONG& decim, ULONG DdcChan)
int CDdc4x16Srv::SetDecim(CModule* pModule, double& decim, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	PDDC4X16SRV_CFG pSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	pSrvCfg->Decimation[DdcChan] = decim;
/*	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG page = (7+ChanNum*8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, page);
	ULONG data = decim & 0xff;
	ProgDdcReg(pModule, 0, chipMask, 21, data);
	ProgDdcReg(pModule, 1, chipMask, 22, data);
	ULONG msb_decim = (decim >> 8) & 0x000f;
//	data = (data & 0xfff0) | ((decim >> 8) & 0xf);
	data = (data & 0xfff0) | msb_decim;
	ProgDdcReg(pModule, 0, chipMask, 22, data);
*/
	return Status;
}

//***************************************************************************************
//int CDdc4x16Srv::GetDecim(CModule* pModule, ULONG& decim, ULONG DdcChan)
int CDdc4x16Srv::GetDecim(CModule* pModule, double& decim, ULONG DdcChan)
{
	int Status = BRDerr_OK;

	PDDC4X16SRV_CFG pSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	decim = pSrvCfg->Decimation[DdcChan];
/*	ULONG chipMask = 1 << (DdcChan >> 2);
	ULONG ChanNum = DdcChan % 4;

	ULONG page = (7+ChanNum*8) << 1;
	ProgDdcReg(pModule, 0, chipMask, 2, page);
	ULONG data;
	ProgDdcReg(pModule, 1, chipMask, 21, data);
	decim = data;
	ProgDdcReg(pModule, 1, chipMask, 22, data);
	decim |= (data & 0x000f) << 8;
*/
	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::SetFormat(CModule* pModule, ULONG format)
{
	if(m_AdmConst.ByBits.Mod != 5)
		return BRDerr_CMD_UNSUPPORTED;

	CDdcSrv::SetFormat(pModule, format);

	ULONG chipMask = 0xf;
//	GetChanMask(pModule, chipMask);

	ULONG data = 64 << 1;	// 64th page
	ProgDdcReg(pModule, 0, chipMask, 2, data); // Register 2 (address page)

	data = (format == 4) ? 0x34 : 0x14; // ROUND = 24 bits : 16 bits
	ProgDdcReg(pModule, 0, chipMask, 19, data); // Final SHIFT Register

	data = 98 << 1;	// 98th page
	ProgDdcReg(pModule, 0, chipMask, 2, data); // Register 2 (address page)

	data = (format == 4) ? 0xE9 : 0xD9; // BITS_PER_WORD = 24 bits : 16 bits
	ProgDdcReg(pModule, 0, chipMask, 20, data); // Output Word Size

	return BRDerr_OK;
}

//***************************************************************************************
int CDdc4x16Srv::SetProgram(CModule* pModule, PVOID args)
{
	int Status = BRDerr_OK;
	PBRD_DdcProgram pProg = (PBRD_DdcProgram)args;
	ULONG chipMask = pProg->mask;
	ULONG num_rec = pProg->num;
	PBRD_DdcRec pRec = (PBRD_DdcRec)pProg->pRecords;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;

	ULONG cur_page = 0xff;
	for(ULONG i = 0; i < num_rec; i++)
	{
		if(cur_page != pRec[i].page)
		{
			param.reg = DDCnr_ADR;
			PDDC_ADR pAdr = (PDDC_ADR)&param.val;
			pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
			pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

			param.reg = DDCnr_WRDATA;
			PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
			pData->ByBits.Data = pRec[i].page << 1;
			pData->ByBits.Cmd = 0; // writing
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		param.reg = DDCnr_ADR;
		PDDC_ADR pAdr = (PDDC_ADR)&param.val;
		pAdr->ByBits.Mask = chipMask;				// выбираем микросхемы DDC для программирования
		pAdr->ByBits.RegAdr = pRec[i].reg;			// register address
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.reg = DDCnr_WRDATA;
		PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
		pData->ByBits.Data = pRec[i].data;
		pData->ByBits.Cmd = 0; // writing
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}

	for(int iDdc = 0; iDdc < 4; iDdc++)
	{
		if((chipMask >> iDdc) & 1)
			if(VerifyProgram(pModule, 1 << iDdc, pRec, num_rec ) != BRDerr_OK)
			{
				printf("Press any key for continue...\n");
                IPC_getch();
//				return BRDerr_DDC_INVALID_WRRDREG;
				Status = BRDerr_DDC_INVALID_WRRDREG;
			}
	}

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::VerifyProgram(CModule* pModule, ULONG chipMask, PBRD_DdcRec pRec, ULONG num_rec)
{
	int Status = BRDerr_OK;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;

	ULONG cur_page = 0xff;
	for(ULONG i = 0; i < num_rec; i++)
	{
		if(pRec[i].reg == 0 ||
		   pRec[i].reg == 1 ||
		   pRec[i].reg == 3 ||
		   pRec[i].reg == 5 ||
		  (pRec[i].page == 98 && pRec[i].reg == 27))
			continue;
		if(cur_page != pRec[i].page)
		{
			cur_page = pRec[i].page;
			param.reg = DDCnr_ADR;
			PDDC_ADR pAdr = (PDDC_ADR)&param.val;
			pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
			pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

			param.reg = DDCnr_WRDATA;
			PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
			pData->ByBits.Data = cur_page << 1;
			pData->ByBits.Cmd = 0; // writing
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		}
		param.reg = DDCnr_ADR;
		PDDC_ADR pAdr = (PDDC_ADR)&param.val;
		pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
		pAdr->ByBits.RegAdr = pRec[i].reg;			// register address
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.reg = DDCnr_WRDATA;
		PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
		pData->ByBits.Data = 0;
		pData->ByBits.Cmd = 1; // reading
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.reg = DDCnr_RDDATA;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PDDC_RDDATA pReadData = (PDDC_RDDATA)&param.val;
		if(pReadData->ByBits.Data != pRec[i].data)
		{
			BRDC_printf(_BRDC("chip = %d: page = %d, reg = %d, write data = 0x%X, read data = 0x%X - is ERROR!!!\n") ,
									chipMask, pRec[i].page, pRec[i].reg, pRec[i].data, pReadData->ByBits.Data);
//			printf("Press any key for continue...\n");
//			_getch();
//			return BRDerr_DDC_INVALID_WRRDREG;
			Status = BRDerr_DDC_INVALID_WRRDREG;
		}
		//else
		//	printf("page = %d, reg = %d, write data = 0x%X, read data = 0x%X - is OK!!!\n") ,pRec[i].page, pRec[i].reg, pRec[i].data, pReadData->ByBits.Data);
	}

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::SetDDCSync(CModule* pModule, ULONG mode, ULONG prog, ULONG async)
{
	int Status = BRDerr_OK;

	DDC_MODE2 Mode2;
	Mode2.AsWhole = REG_PEEK_IND(pModule, m_DdcTetrNum, DDCnr_MODE2);
	Mode2.ByBits.SyncMode = mode;
	Mode2.ByBits.Prog = prog;
	Mode2.ByBits.Async = async;
	REG_POKE_IND(pModule, m_DdcTetrNum, DDCnr_MODE2, Mode2.AsWhole);

	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;
	//param.reg = DDCnr_MODE2;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PDDC_MODE2 pMode2 = (PDDC_MODE2)&param.val;
	//pMode2->ByBits.SyncMode = mode;
	//pMode2->ByBits.Prog = prog;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//PDDC4X16SRV_CFG pSrvCfg = (PDDC4X16SRV_CFG)m_pConfig;
	//int num_ddc = pSrvCfg->DdcCfg.MaxChan / 4;
	//ULONG chanMask;
	//GetChanMask(pModule, chanMask);
	//ULONG mask = 0xf;
	//for(int iDDC = 0; iDDC < num_ddc; iDDC++)
	//{
	//	ULONG chanDdc = (chanMask >> (4 * iDDC)) & mask;
	//	ULONG chipMask = 1 << iDDC;
	//	for(int iChan = 0; iChan < 4; iChan++)
	//	{
	//		ULONG page = 7 + (iChan << 3);
	//		ULONG data;
	//		ReadDdcReg(pModule, chipMask, page, 16, data);

	//		ULONG fl_reset = ((chanDdc >> iChan) & 1) ? 0 : 1;
	//		data = (data & 0x7F) | (fl_reset << 7);
	//		WriteDdcReg(pModule, chipMask, page, 16, data);
	//	}
	//	if(async)
	//	{
	//		ULONG chan_num = (chanDdc & 1) + ((chanDdc >> 1) & 1) + ((chanDdc >> 2) & 1) + ((chanDdc >> 3) & 1);
	//		ULONG data = chan_num << 1;
	//		WriteDdcReg(pModule, chipMask, 98, 19, data);
	//		data = (chan_num << 1) - 1;
	//		WriteDdcReg(pModule, chipMask, 98, 20, data);
	//	}
	//}

	//WriteDdcReg(pModule, 0xF, 98, 16, 0x7F);
	//WriteDdcReg(pModule, 0xF, 98, 18, 0x4A);
	//WriteDdcReg(pModule, 0xF, 98, 21, 0x00);
	//WriteDdcReg(pModule, 0xF, 98, 22, 0xE4);
	//WriteDdcReg(pModule, 0xF, 98, 23, 0x11);
	//WriteDdcReg(pModule, 0xF, 98, 24, 0x22);
	//WriteDdcReg(pModule, 0xF, 98, 25, 0x44);
	//WriteDdcReg(pModule, 0xF, 98, 26, 0x88);
	//WriteDdcReg(pModule, 0xF, 98, 27, 0x00);
	//WriteDdcReg(pModule, 0xF, 98, 28, 0x03);

	//if(async)
	//	WriteDdcReg(pModule, 0xF, 98, 17, 0x40);
	//else
	//{
	//	WriteDdcReg(pModule, 0xF, 98, 17, 0x48);
	//	WriteDdcReg(pModule, 0xF, 98, 19, 0x01);
	//	WriteDdcReg(pModule, 0xF, 98, 20, 0x09);
	//}

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::GetDDCSync(CModule* pModule, ULONG& mode, ULONG& async)
{
	int Status = BRDerr_OK;
	DDC_MODE2 Mode2;
	Mode2.AsWhole = REG_PEEK_IND(pModule, m_DdcTetrNum, DDCnr_MODE2);
	mode = Mode2.ByBits.SyncMode;
	async = Mode2.ByBits.Async;

	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;
	//param.reg = DDCnr_MODE2;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PDDC_MODE2 pMode2 = (PDDC_MODE2)&param.val;
	//mode = pMode2->ByBits.SyncMode;

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::SetFrame(CModule* pModule, ULONG Len)
{
	//if(m_AdmConst.ByBits.Mod == 5)
	if(m_AdmConst.ByBits.Mod != 2)
		return BRDerr_CMD_UNSUPPORTED;

	int Status = BRDerr_OK;

	if(Len > 511)
		Status = BRDerr_BAD_PARAMETER;
	else
	{
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = m_DdcTetrNum;
		param.reg = DDCnr_FRLEN;
		param.val = Len;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::GetFrame(CModule* pModule, ULONG& Len)
{
	//if(m_AdmConst.ByBits.Mod == 5)
	if(m_AdmConst.ByBits.Mod != 2)
		return BRDerr_CMD_UNSUPPORTED;

	int Status = BRDerr_OK;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_FRLEN;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	Len = param.val;
	return Status;
}

//***************************************************************************************
void CDdc4x16Srv::WriteDdcReg(CModule* pModule, ULONG chipMask, ULONG page, ULONG reg, ULONG data)
{
	DDC_ADR addr;
	addr.ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	addr.ByBits.RegAdr = 2;			// 2 = address of register of page address
	REG_POKE_IND(pModule, m_DdcTetrNum, DDCnr_ADR, addr.AsWhole);

	DDC_WRDATA wrdata;
	wrdata.ByBits.Data = page << 1;
	wrdata.ByBits.Cmd = 0; // writing
	REG_POKE_IND(pModule, m_DdcTetrNum, DDCnr_WRDATA, wrdata.AsWhole);

	addr.ByBits.RegAdr = reg;			// register address
	REG_POKE_IND(pModule, m_DdcTetrNum, DDCnr_ADR, addr.AsWhole);

	wrdata.ByBits.Data = data;
	REG_POKE_IND(pModule, m_DdcTetrNum, DDCnr_WRDATA, wrdata.AsWhole);

	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_DdcTetrNum;
	//param.reg = DDCnr_ADR;
	//PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = page << 1;
	//pData->ByBits.Cmd = 0; // writing
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_ADR;
	//pAdr = (PDDC_ADR)&param.val;
	//pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	//pAdr->ByBits.RegAdr = reg;			// register address
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	//param.reg = DDCnr_WRDATA;
	//pData = (PDDC_WRDATA)&param.val;
	//pData->ByBits.Data = data;
	//pData->ByBits.Cmd = 0; // writing
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
}

//***************************************************************************************
void CDdc4x16Srv::ReadDdcReg(CModule* pModule, ULONG chipMask, ULONG page, ULONG reg, ULONG& data)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_ADR;
	PDDC_ADR pAdr = (PDDC_ADR)&param.val;
	pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	pAdr->ByBits.RegAdr = 2;			// 2 = address of register of page address
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_WRDATA;
	PDDC_WRDATA pData = (PDDC_WRDATA)&param.val;
	pData->ByBits.Data = page << 1;
	pData->ByBits.Cmd = 0; // writing
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_ADR;
	pAdr = (PDDC_ADR)&param.val;
	pAdr->ByBits.Mask = chipMask;		// выбираем микросхемы DDC для программирования
	pAdr->ByBits.RegAdr = reg;			// register address
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_WRDATA;
	pData = (PDDC_WRDATA)&param.val;
	pData->ByBits.Data = 0;
	pData->ByBits.Cmd = 1; // reading
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_RDDATA;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDC_RDDATA pReadData = (PDDC_RDDATA)&param.val;
	data = pReadData->ByBits.Data;
}

//***************************************************************************************
int CDdc4x16Srv::SetAdcMode(CModule* pModule, PBRD_Adc4x16Mode pMode)
{
	int Status = BRDerr_OK;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_MODE2;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDC_MODE2 pMode2 = (PDDC_MODE2)&param.val;
	pMode2->ByBits.AdcFmt = pMode->bits;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_GAIN;
	param.val = pMode->gainMask;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = DDCnr_ADCCTRL;
	PDDC_ADCCTRL pAdcCtrl = (PDDC_ADCCTRL)&param.val;
	pAdcCtrl->ByBits.Dither = pMode->dither;
	pAdcCtrl->ByBits.Rand = pMode->rand;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::GetAdcMode(CModule* pModule, PBRD_Adc4x16Mode pMode)
{
	int Status = BRDerr_OK;

	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = DDCnr_MODE2;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDC_MODE2 pMode2 = (PDDC_MODE2)&param.val;
	pMode->bits = pMode2->ByBits.AdcFmt;

	param.reg = DDCnr_GAIN;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pMode->gainMask = param.val;

	param.reg = DDCnr_GAIN;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDC_ADCCTRL pAdcCtrl = (PDDC_ADCCTRL)&param.val;
	pMode->dither = pAdcCtrl->ByBits.Dither;
	pMode->rand = pAdcCtrl->ByBits.Rand;

	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::CtrlAlign(CModule* pModule, U32 cmd, PVOID arg)
{
	int Status = BRDerr_OK;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdcTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	if(DDC4X16cmd_SETALIGN == cmd)
	{
		pFormat->ByBits.Align = *(ULONG*)arg;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
		*(ULONG*)arg = pFormat->ByBits.Align;
	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::SetSpecific(CModule* pModule, PBRD_DdcSpec pSpec)
{
	int Status = BRDerr_OK;
	switch(pSpec->command)
	{
	case  DDC4X16cmd_WRITE:
		{
		PBRD_DdcReg pReg = (PBRD_DdcReg)pSpec->arg;
		WriteDdcReg(pModule, 1 << pReg->ddcNum, pReg->page, pReg->reg, pReg->val);
		break;
		}
	case  DDC4X16cmd_READ:
		{
		PBRD_DdcReg pReg = (PBRD_DdcReg)pSpec->arg;
		ULONG data;
		ReadDdcReg(pModule, 1 << pReg->ddcNum, pReg->page, pReg->reg, data);
		pReg->val = data;
		break;
		}
	case DDC4X16cmd_SETADCMODE:
		{
		PBRD_Adc4x16Mode pAdcMode = (PBRD_Adc4x16Mode)pSpec->arg;
		//if(m_AdmConst.ByBits.Mod == 5)
			SetAdcMode(pModule, pAdcMode);
		//else
		//	Status = BRDerr_CMD_UNSUPPORTED;
		break;
		}
	case DDC4X16cmd_GETADCMODE:
		{
		PBRD_Adc4x16Mode pAdcMode = (PBRD_Adc4x16Mode)pSpec->arg;
		if(m_AdmConst.ByBits.Mod == 5)
			GetAdcMode(pModule, pAdcMode);
		else
			Status = BRDerr_CMD_UNSUPPORTED;
		break;
		}
	case DDC4X16cmd_SETALIGN:
	case DDC4X16cmd_GETALIGN:
		{
		if(m_AdmConst.ByBits.Mod == 5)
			CtrlAlign(pModule, pSpec->command, pSpec->arg);
		else
			Status = BRDerr_CMD_UNSUPPORTED;
		break;
		}
	case DDC4X16cmd_SETFCCODE:
		{
		PBRD_UvalChan pVal = (PBRD_UvalChan)pSpec->arg;
		SetFcCode(pModule, pVal->value, pVal->chan);
		break;
		}
	case DDC4X16cmd_GETFCCODE:
		{
		PBRD_UvalChan pVal = (PBRD_UvalChan)pSpec->arg;
		GetFcCode(pModule, pVal->value, pVal->chan);
		break;
		}
	}
	return Status;
}

//***************************************************************************************
int CDdc4x16Srv::StartEnable(CModule* pModule, ULONG Enbl)
{
	int Status = BRDerr_OK;
	ULONG chipMask;
	GetChanMask(pModule, chipMask);

//	ULONG data = 0xf8;
//	ProgDdcReg(pModule, 0, chipMask, 0, data); // Global Reset Register - reset DDC
	if(Enbl)
	{
//		data = 0xc0;
//		ProgDdcReg(pModule, 0, chipMask, 5, data); // Count Sync Register - Start ONE SHOT
//		data = 0x08;
//		ProgDdcReg(pModule, 0, chipMask, 0, data); // Global Reset Register - unreset DDC
//		data = 0x40;
//		ProgDdcReg(pModule, 0, chipMask, 5, data); // Count Sync Register - End ONE SHOT
	}

	Status = CDdcSrv::StartEnable(pModule, Enbl);

	return BRDerr_OK;
}

// ***************** End of file DdcSrv.cpp ***********************
