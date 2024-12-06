/*
 ***************** File AdcCtrl.cpp ************
 *
 * CTRL-command for BRD Driver for ADC service on ADC submodule
 *
 * (C) InSys by Dorokhin A. Apr 2004
 *
 ******************************************************
*/

#include "module.h"
#include "adcsrv.h"

#define	CURRFILE "ADCCTRL"

static CMD_Info GETTRACETEXT_CMD	= { BRDctrl_GETTRACETEXT,    1, 0, 0, NULL};
static CMD_Info GETTRACEPARAM_CMD	= { BRDctrl_GETTRACEPARAM,   1, 0, 0, NULL};
static CMD_Info SETCHANMASK_CMD		= { BRDctrl_ADC_SETCHANMASK,	0, 0, 0, NULL};
static CMD_Info GETCHANMASK_CMD		= { BRDctrl_ADC_GETCHANMASK,	1, 0, 0, NULL};
static CMD_Info SETGAIN_CMD			= { BRDctrl_ADC_SETGAIN,		0, 0, 0, NULL};
static CMD_Info GETGAIN_CMD			= { BRDctrl_ADC_GETGAIN,		1, 0, 0, NULL};
static CMD_Info SETINPRANGE_CMD		= { BRDctrl_ADC_SETINPRANGE,	0, 0, 0, NULL};
static CMD_Info GETINPRANGE_CMD		= { BRDctrl_ADC_GETINPRANGE,	1, 0, 0, NULL};
static CMD_Info SETBIAS_CMD			= { BRDctrl_ADC_SETBIAS,		0, 0, 0, NULL};
static CMD_Info GETBIAS_CMD			= { BRDctrl_ADC_GETBIAS,		1, 0, 0, NULL};
static CMD_Info SETFORMAT_CMD		= { BRDctrl_ADC_SETFORMAT,		0, 0, 0, NULL};
static CMD_Info GETFORMAT_CMD		= { BRDctrl_ADC_GETFORMAT,		1, 0, 0, NULL};
static CMD_Info SETRATE_CMD			= { BRDctrl_ADC_SETRATE,		0, 0, 0, NULL};
static CMD_Info GETRATE_CMD			= { BRDctrl_ADC_GETRATE,		1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD		= { BRDctrl_ADC_SETCLKMODE,		0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD		= { BRDctrl_ADC_GETCLKMODE,		1, 0, 0, NULL};
static CMD_Info SETSYNCMODE_CMD		= { BRDctrl_ADC_SETSYNCMODE,	0, 0, 0, NULL};
static CMD_Info GETSYNCMODE_CMD		= { BRDctrl_ADC_GETSYNCMODE,	1, 0, 0, NULL};
static CMD_Info SETCLKLOCATION_CMD	= { BRDctrl_ADC_SETCLKLOCATION,	0, 0, 0, NULL};
static CMD_Info GETCLKLOCATION_CMD	= { BRDctrl_ADC_GETCLKLOCATION,	1, 0, 0, NULL};
static CMD_Info SETMASTER_CMD		= { BRDctrl_ADC_SETMASTER,		0, 0, 0, NULL};
static CMD_Info GETMASTER_CMD		= { BRDctrl_ADC_GETMASTER,		1, 0, 0, NULL};
static CMD_Info SETSTARTMODE_CMD	= { BRDctrl_ADC_SETSTARTMODE,	0, 0, 0, NULL};
static CMD_Info GETSTARTMODE_CMD	= { BRDctrl_ADC_GETSTARTMODE,	1, 0, 0, NULL};
static CMD_Info SETSTDELAY_CMD		= { BRDctrl_ADC_SETSTDELAY,		0, 0, 0, NULL};
static CMD_Info GETSTDELAY_CMD		= { BRDctrl_ADC_GETSTDELAY,		1, 0, 0, NULL};
static CMD_Info SETACQDATA_CMD		= { BRDctrl_ADC_SETACQDATA,		0, 0, 0, NULL};
static CMD_Info GETACQDATA_CMD		= { BRDctrl_ADC_GETACQDATA,		1, 0, 0, NULL};
static CMD_Info SETSKIPDATA_CMD		= { BRDctrl_ADC_SETSKIPDATA,	0, 0, 0, NULL};
static CMD_Info GETSKIPDATA_CMD		= { BRDctrl_ADC_GETSKIPDATA,	1, 0, 0, NULL};
static CMD_Info SETTITLEMODE_CMD	= { BRDctrl_ADC_SETTITLEMODE,	0, 0, 0, NULL};
static CMD_Info GETTITLEMODE_CMD	= { BRDctrl_ADC_GETTITLEMODE,	1, 0, 0, NULL};
static CMD_Info SETINPRESIST_CMD	= { BRDctrl_ADC_SETINPRESIST,	0, 0, 0, NULL};
static CMD_Info GETINPRESIST_CMD	= { BRDctrl_ADC_GETINPRESIST,	1, 0, 0, NULL};
static CMD_Info SETDCCOUPLING_CMD	= { BRDctrl_ADC_SETDCCOUPLING,	0, 0, 0, NULL};
static CMD_Info GETDCCOUPLING_CMD	= { BRDctrl_ADC_GETDCCOUPLING,	1, 0, 0, NULL};
static CMD_Info SETINPFILTER_CMD	= { BRDctrl_ADC_SETINPFILTER,	0, 0, 0, NULL};
static CMD_Info GETINPFILTER_CMD	= { BRDctrl_ADC_GETINPFILTER,	1, 0, 0, NULL};
static CMD_Info SETINPSRC_CMD		= { BRDctrl_ADC_SETINPSRC,		0, 0, 0, NULL};
static CMD_Info GETINPSRC_CMD		= { BRDctrl_ADC_GETINPSRC,		1, 0, 0, NULL};
static CMD_Info SETINPMUX_CMD		= { BRDctrl_ADC_SETINPMUX,		0, 0, 0, NULL};
static CMD_Info GETINPMUX_CMD		= { BRDctrl_ADC_GETINPMUX,		1, 0, 0, NULL};
static CMD_Info SETDIFMODE_CMD		= { BRDctrl_ADC_SETDIFMODE,		0, 0, 0, NULL};
static CMD_Info GETDIFMODE_CMD		= { BRDctrl_ADC_GETDIFMODE,		1, 0, 0, NULL};
static CMD_Info SETSAMPLEHOLD_CMD	= { BRDctrl_ADC_SETSAMPLEHOLD,	0, 0, 0, NULL};
static CMD_Info GETSAMPLEHOLD_CMD	= { BRDctrl_ADC_GETSAMPLEHOLD,	1, 0, 0, NULL};
static CMD_Info SETGAINTUNING_CMD	= { BRDctrl_ADC_SETGAINTUNING,	0, 0, 0, NULL};
static CMD_Info GETGAINTUNING_CMD	= { BRDctrl_ADC_GETGAINTUNING,	1, 0, 0, NULL};
static CMD_Info SETTARGET_CMD		= { BRDctrl_ADC_SETTARGET,		0, 0, 0, NULL};
static CMD_Info GETTARGET_CMD		= { BRDctrl_ADC_GETTARGET,		1, 0, 0, NULL};
static CMD_Info SETCLKPHASE_CMD		= { BRDctrl_ADC_SETCLKPHASE,	0, 0, 0, NULL};
static CMD_Info GETCLKPHASE_CMD		= { BRDctrl_ADC_GETCLKPHASE,	1, 0, 0, NULL};
static CMD_Info SETDBLCLK_CMD		= { BRDctrl_ADC_SETDBLCLK,		0, 0, 0, NULL};
static CMD_Info GETDBLCLK_CMD		= { BRDctrl_ADC_GETDBLCLK,		1, 0, 0, NULL};
static CMD_Info SELFCLBR_CMD		= { BRDctrl_ADC_SELFCLBR,		0, 0, 0, NULL};
static CMD_Info SETPRETRIGMODE_CMD	= { BRDctrl_ADC_SETPRETRIGMODE,	0, 0, 0, NULL};
static CMD_Info GETPRETRIGMODE_CMD	= { BRDctrl_ADC_GETPRETRIGMODE,	1, 0, 0, NULL};
static CMD_Info SETCLKTHR_CMD		= { BRDctrl_ADC_SETCLKTHR,		0, 0, 0, NULL};
static CMD_Info GETCLKTHR_CMD		= { BRDctrl_ADC_GETCLKTHR,		1, 0, 0, NULL};
static CMD_Info SETEXTCLKTHR_CMD	= { BRDctrl_ADC_SETEXTCLKTHR,	0, 0, 0, NULL};
static CMD_Info GETEXTCLKTHR_CMD	= { BRDctrl_ADC_GETEXTCLKTHR,	1, 0, 0, NULL};
static CMD_Info SETCLKINV_CMD		= { BRDctrl_ADC_SETCLKINV,		0, 0, 0, NULL};
static CMD_Info GETCLKINV_CMD		= { BRDctrl_ADC_GETCLKINV,		1, 0, 0, NULL};

static CMD_Info SETMU_CMD			= { BRDctrl_ADC_SETMU,			0, 0, 0, NULL};
static CMD_Info GETMU_CMD			= { BRDctrl_ADC_GETMU,			1, 0, 0, NULL};
static CMD_Info SETSTARTSLAVE_CMD	= { BRDctrl_ADC_SETSTARTSLAVE,	0, 0, 0, NULL};
static CMD_Info GETSTARTSLAVE_CMD	= { BRDctrl_ADC_GETSTARTSLAVE,	1, 0, 0, NULL};
static CMD_Info SETCLOCKSLAVE_CMD	= { BRDctrl_ADC_SETCLOCKSLAVE,	0, 0, 0, NULL};
static CMD_Info GETCLOCKSLAVE_CMD	= { BRDctrl_ADC_GETCLOCKSLAVE,	1, 0, 0, NULL};
static CMD_Info GETCHANORDER		= { BRDctrl_ADC_GETCHANORDER,	1, 0, 0, NULL};

static CMD_Info SETCODE_CMD			= { BRDctrl_ADC_SETCODE,		0, 0, 0, NULL};
static CMD_Info GETCODE_CMD			= { BRDctrl_ADC_GETCODE,		1, 0, 0, NULL};
static CMD_Info GETCFG_CMD			= { BRDctrl_ADC_GETCFG,			1, 0, 0, NULL};
static CMD_Info WRITEINIFILE_CMD	= { BRDctrl_ADC_WRITEINIFILE,	1, 0, 0, NULL};
static CMD_Info READINIFILE_CMD		= { BRDctrl_ADC_READINIFILE,	0, 0, 0, NULL};
static CMD_Info SETTESTMODE_CMD		= { BRDctrl_ADC_SETTESTMODE,	0, 0, 0, NULL};
static CMD_Info GETTESTMODE_CMD		= { BRDctrl_ADC_GETTESTMODE,	1, 0, 0, NULL};
static CMD_Info SETTESTSEQ_CMD		= { BRDctrl_ADC_SETTESTSEQ,		0, 0, 0, NULL};
static CMD_Info GETTESTSEQ_CMD		= { BRDctrl_ADC_GETTESTSEQ,		1, 0, 0, NULL};
static CMD_Info WRDELAY_CMD			= { BRDctrl_ADC_WRDELAY,		0, 0, 0, NULL};
static CMD_Info RDDELAY_CMD			= { BRDctrl_ADC_RDDELAY,		1, 0, 0, NULL};
static CMD_Info RSTDELAY_CMD		= { BRDctrl_ADC_RSTDELAY,		0, 0, 0, NULL};

static CMD_Info SETSPECIFIC_CMD		= { BRDctrl_ADC_SETSPECIFIC,	0, 0, 0, NULL};
static CMD_Info GETSPECIFIC_CMD		= { BRDctrl_ADC_GETSPECIFIC,	1, 0, 0, NULL};

static CMD_Info GETDLGPAGES_CMD		= { BRDctrl_ADC_GETDLGPAGES,	0, 0, 0, NULL};
//CMD_Info ENDPAGESDLG_CMD	= { BRDctrl_ADC_ENDPAGESDLG,	0, 0, 0, NULL};
//CMD_Info SETPROPDLG_CMD		= { BRDctrl_ADC_SETPROPDLG,		0, 0, 0, NULL};
//CMD_Info GETPROPDLG_CMD		= { BRDctrl_ADC_GETPROPDLG,		1, 0, 0, NULL};

static CMD_Info FIFORESET_CMD		= { BRDctrl_ADC_FIFORESET,	 0, 0, 0, NULL};
static CMD_Info ENABLE_CMD			= { BRDctrl_ADC_ENABLE,		 0, 0, 0, NULL};
static CMD_Info PREPARESTART_CMD	= { BRDctrl_ADC_PREPARESTART,0, 0, 0, NULL};
static CMD_Info ISCOMPLEX_CMD		= { BRDctrl_ADC_ISCOMPLEX, 1, 0, 0, NULL};
static CMD_Info SETFC_CMD			= { BRDctrl_ADC_SETFC, 1, 0, 0, NULL};
static CMD_Info GETFC_CMD			= { BRDctrl_ADC_GETFC, 1, 0, 0, NULL};
static CMD_Info FIFOSTATUS_CMD		= { BRDctrl_ADC_FIFOSTATUS,	 1, 0, 0, NULL};
static CMD_Info GETDATA_CMD			= { BRDctrl_ADC_GETDATA,	 0, 0, 0, NULL};
static CMD_Info ISBITSOVERFLOW_CMD	= { BRDctrl_ADC_ISBITSOVERFLOW, 1, 0, 0, NULL};
static CMD_Info GETPRETRIGEVENT_CMD = { BRDctrl_ADC_GETPRETRIGEVENT,1, 0, 0, NULL};
static CMD_Info GETPREVENTPREC_CMD	= { BRDctrl_ADC_GETPREVENTPREC, 1, 0, 0, NULL};
static CMD_Info GETSRCSTREAM_CMD	= { BRDctrl_ADC_GETSRCSTREAM, 1, 0, 0, NULL};
static CMD_Info GETBLKCNT_CMD		= { BRDctrl_ADC_GETBLKCNT,	 1, 0, 0, NULL};
static CMD_Info SETTLDATA_CMD		= { BRDctrl_ADC_SETTITLEDATA, 0, 0, 0, NULL};
static CMD_Info GETTLDATA_CMD		= { BRDctrl_ADC_GETTITLEDATA, 1, 0, 0, NULL};

static CMD_Info CLRBITSOVERFLOW_CMD		= { BRDctrl_ADC_CLRBITSOVERFLOW,  	0, 0, 0, NULL};

//***************************************************************************************
CAdcSrv::CAdcSrv(int idx,  const BRDCHAR *name, int srv_num, CModule* pModule, PVOID pCfg, ULONG cfgSize) :
CService(idx, name, srv_num, pModule, pCfg, cfgSize)
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_STREAMABLE_IN |
			BRDserv_ATTR_SDRAMABLE |
			BRDserv_ATTR_CMPABLE |
			BRDserv_ATTR_DSPABLE |
			BRDserv_ATTR_EXCLUSIVE_ONLY;
	
	Init(&GETTRACETEXT_CMD, (CmdEntry)&CAdcSrv::CtrlGetTraceText);
	Init(&GETTRACEPARAM_CMD, (CmdEntry)&CAdcSrv::CtrlGetTraceParam);

	Init(&GETCFG_CMD, (CmdEntry)&CAdcSrv::CtrlCfg);
	Init(&WRITEINIFILE_CMD, (CmdEntry)&CAdcSrv::CtrlIniFile);
	Init(&READINIFILE_CMD, (CmdEntry)&CAdcSrv::CtrlIniFile);

	Init(&GETDLGPAGES_CMD, (CmdEntry)&CAdcSrv::CtrlGetPages);
	//Init(&ENDPAGESDLG_CMD, (CmdEntry)CtrlInitEndDlg);
	//Init(&SETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);
	//Init(&GETPROPDLG_CMD, (CmdEntry)CtrlDlgProperty);

	Init(&SETCHANMASK_CMD, (CmdEntry)&CAdcSrv::CtrlChanMask);
	Init(&GETCHANMASK_CMD, (CmdEntry)&CAdcSrv::CtrlChanMask);
	Init(&SETGAIN_CMD, (CmdEntry)&CAdcSrv::CtrlGain);
	Init(&GETGAIN_CMD, (CmdEntry)&CAdcSrv::CtrlGain);
	Init(&SETINPRANGE_CMD, (CmdEntry)&CAdcSrv::CtrlInpRange);
	Init(&GETINPRANGE_CMD, (CmdEntry)&CAdcSrv::CtrlInpRange);
	Init(&SETBIAS_CMD, (CmdEntry)&CAdcSrv::CtrlBias);
	Init(&GETBIAS_CMD, (CmdEntry)&CAdcSrv::CtrlBias);
	Init(&SETFORMAT_CMD, (CmdEntry)&CAdcSrv::CtrlFormat);
	Init(&GETFORMAT_CMD, (CmdEntry)&CAdcSrv::CtrlFormat);
	Init(&SETCODE_CMD, (CmdEntry)&CAdcSrv::CtrlCode);
	Init(&GETCODE_CMD, (CmdEntry)&CAdcSrv::CtrlCode);
	Init(&SETRATE_CMD, (CmdEntry)&CAdcSrv::CtrlRate);
	Init(&GETRATE_CMD, (CmdEntry)&CAdcSrv::CtrlRate);
	Init(&SETCLKMODE_CMD, (CmdEntry)&CAdcSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CAdcSrv::CtrlClkMode);
	Init(&SETSYNCMODE_CMD, (CmdEntry)&CAdcSrv::CtrlSyncMode);
	Init(&GETSYNCMODE_CMD, (CmdEntry)&CAdcSrv::CtrlSyncMode);
	Init(&SETMASTER_CMD, (CmdEntry)&CAdcSrv::CtrlMaster);
	Init(&GETMASTER_CMD, (CmdEntry)&CAdcSrv::CtrlMaster);
	Init(&SETCLKLOCATION_CMD, (CmdEntry)&CAdcSrv::CtrlClkLocation);
	Init(&GETCLKLOCATION_CMD, (CmdEntry)&CAdcSrv::CtrlClkLocation);
	Init(&SETDBLCLK_CMD, (CmdEntry)&CAdcSrv::CtrlDblClk);
	Init(&GETDBLCLK_CMD, (CmdEntry)&CAdcSrv::CtrlDblClk);
	Init(&SETCLKTHR_CMD, (CmdEntry)&CAdcSrv::CtrlClkThr);
	Init(&GETCLKTHR_CMD, (CmdEntry)&CAdcSrv::CtrlClkThr);
	Init(&SETEXTCLKTHR_CMD, (CmdEntry)&CAdcSrv::CtrlExtClkThr);
	Init(&GETEXTCLKTHR_CMD, (CmdEntry)&CAdcSrv::CtrlExtClkThr);
	Init(&SELFCLBR_CMD, (CmdEntry)&CAdcSrv::CtrlSelfClbr);
	Init(&SETCLKINV_CMD, (CmdEntry)&CAdcSrv::CtrlClkInv);
	Init(&GETCLKINV_CMD, (CmdEntry)&CAdcSrv::CtrlClkInv);

	Init(&SETMU_CMD, (CmdEntry)&CAdcSrv::CtrlMu);
	Init(&GETMU_CMD, (CmdEntry)&CAdcSrv::CtrlMu);
	Init(&SETSTARTSLAVE_CMD, (CmdEntry)&CAdcSrv::CtrlStartSlave);
	Init(&GETSTARTSLAVE_CMD, (CmdEntry)&CAdcSrv::CtrlStartSlave);
	Init(&SETCLOCKSLAVE_CMD, (CmdEntry)&CAdcSrv::CtrlClockSlave);
	Init(&GETCLOCKSLAVE_CMD, (CmdEntry)&CAdcSrv::CtrlClockSlave);
	Init(&GETCHANORDER, (CmdEntry)&CAdcSrv::CtrlChanOrder);

	Init(&SETDCCOUPLING_CMD, (CmdEntry)&CAdcSrv::CtrlDcCoupling);
	Init(&GETDCCOUPLING_CMD, (CmdEntry)&CAdcSrv::CtrlDcCoupling);
	Init(&SETINPRESIST_CMD, (CmdEntry)&CAdcSrv::CtrlInpResist);
	Init(&GETINPRESIST_CMD, (CmdEntry)&CAdcSrv::CtrlInpResist);
	Init(&SETINPSRC_CMD, (CmdEntry)&CAdcSrv::CtrlInpSrc);
	Init(&GETINPSRC_CMD, (CmdEntry)&CAdcSrv::CtrlInpSrc);
	Init(&SETINPMUX_CMD, (CmdEntry)&CAdcSrv::CtrlInpMux);
	Init(&GETINPMUX_CMD, (CmdEntry)&CAdcSrv::CtrlInpMux);
	Init(&SETDIFMODE_CMD, (CmdEntry)&CAdcSrv::CtrlDifMode);
	Init(&GETDIFMODE_CMD, (CmdEntry)&CAdcSrv::CtrlDifMode);
	Init(&SETSAMPLEHOLD_CMD, (CmdEntry)&CAdcSrv::CtrlSampleHold);
	Init(&GETSAMPLEHOLD_CMD, (CmdEntry)&CAdcSrv::CtrlSampleHold);
	Init(&SETTESTMODE_CMD, (CmdEntry)&CAdcSrv::CtrlTestMode);
	Init(&GETTESTMODE_CMD, (CmdEntry)&CAdcSrv::CtrlTestMode);
	Init(&SETTESTSEQ_CMD, (CmdEntry)&CAdcSrv::CtrlTestSeq);
	Init(&GETTESTSEQ_CMD, (CmdEntry)&CAdcSrv::CtrlTestSeq);
	Init(&SETSPECIFIC_CMD, (CmdEntry)&CAdcSrv::CtrlSpecific);
	Init(&GETSPECIFIC_CMD, (CmdEntry)&CAdcSrv::CtrlSpecific);
	Init(&SETINPFILTER_CMD, (CmdEntry)&CAdcSrv::CtrlInpFilter);
	Init(&GETINPFILTER_CMD, (CmdEntry)&CAdcSrv::CtrlInpFilter);
	Init(&WRDELAY_CMD, (CmdEntry)&CAdcSrv::CtrlDelay);
	Init(&RDDELAY_CMD, (CmdEntry)&CAdcSrv::CtrlDelay);
	Init(&RSTDELAY_CMD, (CmdEntry)&CAdcSrv::CtrlDelay);

	Init(&SETCLKPHASE_CMD, (CmdEntry)&CAdcSrv::CtrlClkPhase);
	Init(&GETCLKPHASE_CMD, (CmdEntry)&CAdcSrv::CtrlClkPhase);
	Init(&SETGAINTUNING_CMD, (CmdEntry)&CAdcSrv::CtrlGainTuning);
	Init(&GETGAINTUNING_CMD, (CmdEntry)&CAdcSrv::CtrlGainTuning);
	Init(&SETTARGET_CMD, (CmdEntry)&CAdcSrv::CtrlTarget);
	Init(&GETTARGET_CMD, (CmdEntry)&CAdcSrv::CtrlTarget);
	Init(&GETPRETRIGEVENT_CMD, (CmdEntry)&CAdcSrv::CtrlGetPretrigEvent);
	Init(&GETPREVENTPREC_CMD, (CmdEntry)&CAdcSrv::CtrlGetPreEventPrec);
	Init(&GETBLKCNT_CMD, (CmdEntry)&CAdcSrv::CtrlGetBlkCnt);

	Init(&SETSTARTMODE_CMD, (CmdEntry)&CAdcSrv::CtrlStartMode);
	Init(&GETSTARTMODE_CMD, (CmdEntry)&CAdcSrv::CtrlStartMode);
	Init(&SETPRETRIGMODE_CMD, (CmdEntry)&CAdcSrv::CtrlPretrigMode);
	Init(&GETPRETRIGMODE_CMD, (CmdEntry)&CAdcSrv::CtrlPretrigMode);
	Init(&SETTITLEMODE_CMD, (CmdEntry)&CAdcSrv::CtrlTitleMode);
	Init(&GETTITLEMODE_CMD, (CmdEntry)&CAdcSrv::CtrlTitleMode);
	Init(&SETTLDATA_CMD, (CmdEntry)&CAdcSrv::CtrlTitleData);
	Init(&GETTLDATA_CMD, (CmdEntry)&CAdcSrv::CtrlTitleData);

	Init(&SETSTDELAY_CMD, (CmdEntry)&CAdcSrv::CtrlCnt);
	Init(&GETSTDELAY_CMD, (CmdEntry)&CAdcSrv::CtrlCnt);
	Init(&SETACQDATA_CMD, (CmdEntry)&CAdcSrv::CtrlCnt);
	Init(&GETACQDATA_CMD, (CmdEntry)&CAdcSrv::CtrlCnt);
	Init(&SETSKIPDATA_CMD, (CmdEntry)&CAdcSrv::CtrlCnt);
	Init(&GETSKIPDATA_CMD, (CmdEntry)&CAdcSrv::CtrlCnt);

	Init(&FIFORESET_CMD, (CmdEntry)&CAdcSrv::CtrlFifoReset);
	Init(&ENABLE_CMD, (CmdEntry)&CAdcSrv::CtrlEnable);
	Init(&PREPARESTART_CMD, (CmdEntry)&CAdcSrv::CtrlPrepareStart);
	Init(&ISCOMPLEX_CMD, (CmdEntry)&CAdcSrv::CtrlIsComplex);
	Init(&SETFC_CMD, (CmdEntry)&CAdcSrv::CtrlFc);
	Init(&GETFC_CMD, (CmdEntry)&CAdcSrv::CtrlFc);
	Init(&FIFOSTATUS_CMD, (CmdEntry)&CAdcSrv::CtrlFifoStatus);
	Init(&GETDATA_CMD, (CmdEntry)&CAdcSrv::CtrlGetData);
	Init(&ISBITSOVERFLOW_CMD, (CmdEntry)&CAdcSrv::CtrlIsBitsOverflow);
	Init(&CLRBITSOVERFLOW_CMD, (CmdEntry)&CAdcSrv::CtrlClrBitsOverflow);

	Init(&GETSRCSTREAM_CMD, (CmdEntry)&CAdcSrv::CtrlGetAddrData);

}

//***************************************************************************************
int CAdcSrv::ExtraInit(CModule* pModule)
{
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlRelease(
						void			*pDev,		// InfoSM or InfoBM
						void			*pServData,	// Specific Service Data
						ULONG			cmd,		// Command Code (from BRD_ctrl())
						void			*args 		// Command Arguments (from BRD_ctrl())
						)
{
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	pAdcSrvCfg->isAlreadyInit = 0;
	return BRDerr_CMD_UNSUPPORTED; // РґР»СЏ РѕСЃРІРѕР±РѕР¶РґРµРЅРёСЏ РїРѕРґСЃР»СѓР¶Р±
}

//***************************************************************************************
int CAdcSrv::CtrlIsAvailable(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PSERV_CMD_IsAvailable pServAvailable = (PSERV_CMD_IsAvailable)args;
	pServAvailable->attr = m_attribute;
	m_MainTetrNum = GetTetrNum(pModule, m_index, MAIN_TETR_ID);

	ULONG TetrInstantNum = 1;
	BRDCHAR* pBuf = m_name + (BRDC_strlen(m_name) - 2);
	if(BRDC_strchr(pBuf, '1'))
		TetrInstantNum = 2;

	//GetAdcTetrNum(pModule);
	GetAdcTetrNumEx(pModule, TetrInstantNum);

	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pAdcSrvCfg->TetrNum != -1)
		m_AdcTetrNum = pAdcSrvCfg->TetrNum;
	
/*	BRDCHAR Buffer[128];
	BRDCHAR iniFilePath[MAX_PATH];
	IPC_getCurrentDir(iniFilePath, sizeof(iniFilePath)/sizeof(BRDCHAR));
	BRDC_strcat(iniFilePath, _BRDC("//brd.ini"));
	IPC_getPrivateProfileString(_BRDC("SUB ENUM"), _BRDC("AdcTetrNum"), _BRDC("-1"), Buffer, sizeof(Buffer), iniFilePath);
	int tetr_num = BRDC_atoi(Buffer);
	if(tetr_num != -1)
		m_AdcTetrNum = tetr_num;
*/
	if(m_MainTetrNum != -1 && m_AdcTetrNum != -1)
	{
		m_isAvailable = 1;
		//PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;

		//DEVS_CMD_Reg RegParam;
		//RegParam.idxMain = m_index;
		//RegParam.tetr = m_AdcTetrNum;
		//RegParam.reg = ADM2IFnr_TRES;
		//pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//int exFifo = ((RegParam.val & 0x200) >> 9) && pAdcSrvCfg->FifoSize;
		//if(!exFifo)
		//{
		//	RegParam.reg = ADM2IFnr_FTYPE;
		//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//	int widthFifo = RegParam.val >> 3;
		//	RegParam.reg = ADM2IFnr_FSIZE;
		//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//	pAdcSrvCfg->FifoSize = RegParam.val * widthFifo;
		//}

		//if(!pAdcSrvCfg->isAlreadyInit)
		//{
		//	pAdcSrvCfg->isAlreadyInit = 1;

		//	RegParam.tetr = m_AdcTetrNum;
		//	RegParam.reg = ADM2IFnr_MODE0;
		//	RegParam.val = 0;
		////	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
		//	pMode0->ByBits.Reset = 1;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//	for(int i = 1; i < 32; i++)
		//	{
		//		RegParam.reg = i;
		//		RegParam.val = 0;
		//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
		//	}
		//	RegParam.reg = ADM2IFnr_MODE0;
		//	pMode0->ByBits.AdmClk = 1;
		//	pMode0->ByBits.Reset = 0;
		//	if(exFifo)
		//		pMode0->ByBits.extFifo = 1;
		//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

		//	//RegParam.reg = ADM2IFnr_STATUS;
		//	//pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
		//	//pStatus = (PADM2IF_STATUS)&RegParam.val;

		//	ULONG Status = ExtraInit(pModule);
		//	if(Status != BRDerr_OK)
		//		return Status;
		//}
		//printf("CAdcSrv::CtrlIsAvailable!!\n");
	}
	else
		m_isAvailable = 0;

	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	if(m_MainTetrNum != -1 && m_AdcTetrNum != -1)
	{
		PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;

		DEVS_CMD_Reg RegParam;
		RegParam.idxMain = m_index;
		RegParam.tetr = m_AdcTetrNum;
		RegParam.reg = ADM2IFnr_TRES;
		pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
		int exFifo = ((RegParam.val & 0x200) >> 9) && pAdcSrvCfg->FifoSize;
		if(!exFifo)
		{
			RegParam.reg = ADM2IFnr_FTYPE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			int widthFifo = RegParam.val >> 3;
			RegParam.reg = ADM2IFnr_FSIZE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			pAdcSrvCfg->FifoSize = RegParam.val * widthFifo;
		}

		if(!pAdcSrvCfg->isAlreadyInit)
		{
			pAdcSrvCfg->isAlreadyInit = 1;

			RegParam.tetr = m_AdcTetrNum;
			RegParam.reg = ADM2IFnr_MODE0;
			RegParam.val = 0;
		//	pModule->RegCtrl(DEVScmd_REGREADIND, &RegParam);
			PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&RegParam.val;
			pMode0->ByBits.Reset = 1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			for(int i = 1; i < 32; i++)
			{
				RegParam.reg = i;
				RegParam.val = 0;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);
			}
			RegParam.reg = ADM2IFnr_MODE0;
			pMode0->ByBits.AdmClk = 1;
			pMode0->ByBits.Reset = 0;
			if(exFifo)
				pMode0->ByBits.extFifo = 1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &RegParam);

			//RegParam.reg = ADM2IFnr_STATUS;
			//pModule->RegCtrl(DEVScmd_REGREADDIR, &RegParam);
			//pStatus = (PADM2IF_STATUS)&RegParam.val;

			ULONG Status = ExtraInit(pModule);
			if(Status != BRDerr_OK)
				return Status;
			//printf("CAdcSrv::CtrlCapture!!\n");
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlGetAddrData(
							void			*pDev,		// InfoSM or InfoBM
							void			*pServData,	// Specific Service Data
							ULONG			cmd,		// Command Code (from BRD_ctrl())
							void			*args 		// Command Arguments (from BRD_ctrl())
							)
{
/*
	ULONG m_AdmNum;
//	BRDCHAR buf[SERVNAME_SIZE];
//	BRDC_strcpy(buf, m_name);
//	BRDCHAR* pBuf = buf + (BRDC_strlen(buf) - 2);
//	if(BRDC_strchr(pBuf, '1'))
////	if(_tcschr(m_name, '1'))
//		m_AdmNum = 1;
//	else
		m_AdmNum = 0;
	*(ULONG*)args = (m_AdmNum << 16) | m_AdcTetrNum;
//	*(ULONG*)args = m_AdcTetrNum;
*/
	CModule* pModule = (CModule*)pDev;
	GetAddrData( pModule, (U32*)args );

	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlGetTraceText( void *pDev, void *pServData, ULONG cmd, void *args )
{
	CModule* pModule = (CModule*)pDev;
	BRD_TraceText	*pTT = (BRD_TraceText*)args;
	
	return GetTraceText( pModule, pTT->traceId, pTT->sizeb, pTT->pText );
}

//***************************************************************************************
int CAdcSrv::CtrlGetTraceParam( void *pDev, void *pServData, ULONG cmd, void *args )
{
	CModule* pModule = (CModule*)pDev;
	BRD_TraceParam	*pTP = (BRD_TraceParam*)args;

	return GetTraceParam( pModule, pTP->traceId, pTP->sizeb, &(pTP->sizebReal), pTP->pParam );
}

//***************************************************************************************
int CAdcSrv::CtrlGetPages(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
#ifdef _WIN32
	CModule* pModule = (CModule*)pDev;
	PBRD_PropertyList pList = (PBRD_PropertyList)args;
//	ULONG dlg_mode = *(PULONG)args;

	if(cmd == BRDctrl_ADC_GETDLGPAGES)
	{
		// Open Library
		PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
		pList->hLib = LoadLibrary(pSrvCfg->DlgDllName);
		if(pList->hLib <= (HINSTANCE)HINSTANCE_ERROR)
			return BRDerr_BAD_DEVICE_VITAL_DATA;

		INFO_InitPages_Type* pDlgFunc; 
		pDlgFunc = (INFO_InitPages_Type*)GetProcAddress(pList->hLib, "INFO_InitPages");
		if(!pDlgFunc)
		{
			FreeLibrary(pList->hLib);
			return BRDerr_BAD_DEVICE_VITAL_DATA;
		}
		PVOID pInfo = GetInfoForDialog(pModule);

	//	int num_pages = (pDlgFunc)(pDev, this, dlg_mode, pInfo);
		ULONG num_dlg;
//		int num_pages = (pDlgFunc)(pDev, pSrvCfg->pPropDlg, this, &num_dlg, pInfo);
		int num_pages = (pDlgFunc)(pDev, m_pPropDlg, this, &num_dlg, pInfo);
		pList->PagesCnt = num_pages;
		pList->NumDlg = num_dlg;

		FreeInfoForDialog(pInfo);
	}
	//if(cmd == BRDctrl_ADC_ENDPAGESDLG)
	//{
	//	INFO_DeletePages_Type* pDlgFunc; 
	//	pDlgFunc = (INFO_DeletePages_Type*)GetProcAddress(pList->hLib, _BRDC("INFO_DeletePages"));
	//	(pDlgFunc)(pList->NumDlg);

	//	FreeLibrary(pList->hLib);
	//}

#else
	fprintf(stderr, "%s, %d: - %s not implemented\n", __FILE__, __LINE__, __FUNCTION__);
	return -ENOSYS;
#endif
	return BRDerr_OK;
}

//***************************************************************************************
//int CAdcSrv::CtrlDlgProperty(
//							void		*pDev,		// InfoSM or InfoBM
//							void		*pServData,	// Specific Service Data
//							ULONG		cmd,		// Command Code (from BRD_ctrl())
//							void		*args 		// Command Arguments (from BRD_ctrl())
//							)
//{
//	CModule* pModule = (CModule*)pDev;
//	ULONG dlg_mode = *(PULONG)args;
//
//	// Open Library
//	PADCSRV_CFG pSrvCfg = (PADCSRV_CFG)m_pConfig;
//	HINSTANCE hLib = LoadLibrary(pSrvCfg->DlgDllName);
//	if(hLib <= (HINSTANCE)HINSTANCE_ERROR)
//		return BRDerr_BAD_DEVICE_VITAL_DATA;
//
//	INFO_DlgProp_Type* pDlgPropFunc; 
//	pDlgPropFunc = (INFO_DlgProp_Type*)GetProcAddress(hLib, _BRDC("INFO_DialogProperty"));
//	if(!pDlgPropFunc)
//	{
//		FreeLibrary(hLib);
//		return BRDerr_BAD_DEVICE_VITAL_DATA;
//	}
//	PVOID pInfo = GetInfoForDialog(pModule);
//	if(dlg_mode)
//	{
//		if(cmd == BRDctrl_ADC_GETPROPDLG)
//			dlg_mode |= 0x80000000;
//		int nResponse = (pDlgPropFunc)(pDev, this, dlg_mode, pInfo);
//		if (nResponse == IDOK)
//		{
//			// TODO: Place code here to handle when the dialog is
//			//  dismissed with OK
//			SetPropertyFromDialog(pDev, pInfo);
//		}
//		else if (nResponse == IDCANCEL)
//		{
//			// TODO: Place code here to handle when the dialog is
//			//  dismissed with Cancel
//		}
//		FreeLibrary(hLib);
//	}
//	else
//	{
//		if(cmd == BRDctrl_ADC_GETPROPDLG)
//			dlg_mode |= 0x80000000;
//		(pDlgPropFunc)(pDev, this, dlg_mode, pInfo);
//	}
//
//	FreeInfoForDialog(pInfo);
//
//	return BRDerr_OK;
//}

//***************************************************************************************
int CAdcSrv::CtrlIniFile(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_IniFile pFile = (PBRD_IniFile)args;
	if(BRDctrl_ADC_WRITEINIFILE == cmd)
		Status = SaveProperties(pModule, pFile->fileName, pFile->sectionName);
	else
		Status = SetProperties(pModule, pFile->fileName, pFile->sectionName);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlCfg(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	Status = GetCfg((PBRD_AdcCfg)args);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlChanMask(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_ADC_SETCHANMASK == cmd)
	{
		ULONG mask = *(ULONG*)args;
		Status = SetChanMask(pModule, mask);
	}
	else
	{
		ULONG mask;
		Status = GetChanMask(pModule, mask);
		*(ULONG*)args = mask;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlFormat(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADM2IFnr_FORMAT;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_FORMAT pFormat = (PADM2IF_FORMAT)&param.val;
	if(BRDctrl_ADC_SETFORMAT == cmd)
	{
		pFormat->ByBits.Pack = *(ULONG*)args;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	}
	else
		//*(ULONG*)args = pFormat->ByBits.Pack;
		GetFormat( pModule, (U32*)args );
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlCode(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_ADC_SETCODE == cmd)
	{
		ULONG type = *(ULONG*)args;
		Status = SetCode(pModule, type);
	}
	else
	{
		ULONG type;
		Status = GetCode(pModule, type);
		*(ULONG*)args = type;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlGain(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
//	if(pValChan->chan < 0 || pValChan->chan >= BRD_CHANCNT)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	int Status = BRDerr_OK;
	if(BRDctrl_ADC_SETGAIN == cmd)
		Status = SetGain(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetGain(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlInpRange(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
//	if(pValChan->chan < 0 || pValChan->chan >= BRD_CHANCNT)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	int Status = BRDerr_OK;
	if(BRDctrl_ADC_SETINPRANGE == cmd)
		Status = SetInpRange(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetInpRange(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlBias(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
//	if(pValChan->chan < 0 || pValChan->chan >= BRD_CHANCNT)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	if(BRDctrl_ADC_SETBIAS == cmd)
		Status = SetBias(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetBias(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlRate(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
					)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG ClkSrc;
	Status = GetClkSource(pModule, ClkSrc);
	double ClkValue;
	Status = GetClkValue(pModule, ClkSrc, ClkValue);
	double Rate = *(double*)args;
	if(BRDctrl_ADC_SETRATE == cmd)
	{
		Status = SetRate(pModule, Rate, ClkSrc, ClkValue);
		*(double*)args = Rate;
	}
	else
	{
		Status = GetRate(pModule, Rate, ClkValue);
		*(double*)args = Rate;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlClkMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ClkMode pAdcClk = (PBRD_ClkMode)args;
	if(BRDctrl_ADC_SETCLKMODE == cmd)
	{
		Status = SetClkSource(pModule, pAdcClk->src);
		if(BRDerr_OK != Status)
			return Status;
		Status = SetClkValue(pModule, pAdcClk->src, pAdcClk->value);
	}
	else
	{
		ULONG src = pAdcClk->src;
		Status = GetClkSource(pModule, src);
		pAdcClk->src = src;
		Status = GetClkValue(pModule, pAdcClk->src, pAdcClk->value);
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlSyncMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_SyncMode pSyncMode = (PBRD_SyncMode)args;
	if(BRDctrl_ADC_SETSYNCMODE == cmd)
	{
		Status = SetClkSource(pModule, pSyncMode->clkSrc);
		if(BRDerr_OK != Status)
			return Status;
		Status = SetClkValue(pModule, pSyncMode->clkSrc, pSyncMode->clkValue);
		if(BRDerr_OK == Status)
			Status = SetRate(pModule, pSyncMode->rate, pSyncMode->clkSrc, pSyncMode->clkValue);
	}
	else
	{
		ULONG src = pSyncMode->clkSrc;
		Status = GetClkSource(pModule, src);
		pSyncMode->clkSrc = src;
		Status = GetClkValue(pModule, pSyncMode->clkSrc, pSyncMode->clkValue);
		Status = GetRate(pModule, pSyncMode->rate, pSyncMode->clkValue);
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlMaster(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
					   )
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG MasterMode = *(ULONG*)args;
	if(BRDctrl_ADC_SETMASTER == cmd)
		Status = SetMaster(pModule, MasterMode);
	else
	{
		Status = GetMaster(pModule, MasterMode);
		*(ULONG*)args = MasterMode;
	}
	return Status;
}

//***************************************************************************************
//int CAdcSrv::CtrlMaster(
//						void		*pDev,		// InfoSM or InfoBM
//						void		*pServData,	// Specific Service Data
//						ULONG		cmd,		// Command Code (from BRD_ctrl())
//						void		*args 		// Command Arguments (from BRD_ctrl())
//					   )
//{
//	CModule* pModule = (CModule*)pDev;
//	ULONG* pMasterMode = (ULONG*)args;
//	DEVS_CMD_Reg param;
//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
//	param.idxMain = m_index;
//	param.reg = ADM2IFnr_MODE0;
//	if(BRDctrl_ADC_SETMASTER == cmd)
//	{
//		param.tetr = m_MainTetrNum;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		pMode0->ByBits.Master = *pMasterMode >> 1;
//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//		param.tetr = m_AdcTetrNum;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		pMode0->ByBits.Master = *pMasterMode & 0x1;
//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//	}
//	else
//	{
//		param.tetr = m_AdcTetrNum;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		*pMasterMode = pMode0->ByBits.Master;
//		param.tetr = m_MainTetrNum;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		*pMasterMode |= (pMode0->ByBits.Master << 1);
//	}
//	return BRDerr_OK;
//}

//***************************************************************************************
int CAdcSrv::CtrlStartMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
//	PBRD_AdcStartMode pAdcStartMode = (PBRD_AdcStartMode)args;
	if(BRDctrl_ADC_SETSTARTMODE == cmd)
		Status = SetStartMode(pModule, args);
	else
        Status = GetStartMode(pModule, args);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlTitleMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pTitleMode = (PBRD_EnVal)args;
	if(BRDctrl_ADC_SETTITLEMODE == cmd)
		Status = SetTitleMode(pModule, pTitleMode);
	else
        Status = GetTitleMode(pModule, pTitleMode);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlTitleData(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PULONG pTitleData = (PULONG)args;
	if(BRDctrl_ADC_SETTITLEDATA == cmd)
		Status = SetTitleData(pModule, pTitleData);
	else
        Status = GetTitleData(pModule, pTitleData);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlPretrigMode(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_PretrigMode pPretrigMode = (PBRD_PretrigMode)args;
	if(BRDctrl_ADC_SETPRETRIGMODE == cmd)
		Status = SetPretrigMode(pModule, pPretrigMode);
	else
        Status = GetPretrigMode(pModule, pPretrigMode);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlCnt(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
	switch(cmd)
	{
	case BRDctrl_ADC_SETSTDELAY:
		SetCnt(pModule, 0, pEnVal);
		break;
	case BRDctrl_ADC_GETSTDELAY:
		GetCnt(pModule, 0, pEnVal);
		break;
	case BRDctrl_ADC_SETACQDATA:
		SetCnt(pModule, 1, pEnVal);
		break;
	case BRDctrl_ADC_GETACQDATA:
		GetCnt(pModule, 1, pEnVal);
		break;
	case BRDctrl_ADC_SETSKIPDATA:
		SetCnt(pModule, 2, pEnVal);
		break;
	case BRDctrl_ADC_GETSKIPDATA:
		GetCnt(pModule, 2, pEnVal);
		break;
	}
	return BRDerr_OK;
}

////***************************************************************************************
//int CAdcSrv::CtrlAcqData(
//					void		*pDev,		// InfoSM or InfoBM
//					void		*pServData,	// Specific Service Data
//					ULONG		cmd,		// Command Code (from BRD_ctrl())
//					void		*args 		// Command Arguments (from BRD_ctrl())
//				   )
//{
//	CModule* pModule = (CModule*)pDev;
//	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
//	DEVS_CMD_Reg param;
//	param.idxMain = m_index;
//	param.tetr = m_AdcTetrNum;
//	param.reg = ADM2IFnr_MODE0;
//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
//	if(BRDctrl_ADC_SETACQDATA == cmd)
//	{
//		pMode0->ByBits.Cnt1En = pEnVal->enable;
//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//		param.reg = ADM2IFnr_CNT1;
//		param.val = pEnVal->value;
//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//	}
//	else
//	{
//		pEnVal->enable = pMode0->ByBits.Cnt1En;
//		param.reg = ADM2IFnr_CNT1;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		pEnVal->value = param.val;
//	}
//	return BRDerr_OK;
//}
//
////***************************************************************************************
//int CAdcSrv::CtrlSkipData(
//					void		*pDev,		// InfoSM or InfoBM
//					void		*pServData,	// Specific Service Data
//					ULONG		cmd,		// Command Code (from BRD_ctrl())
//					void		*args 		// Command Arguments (from BRD_ctrl())
//				   )
//{
//	CModule* pModule = (CModule*)pDev;
//	PBRD_EnVal pEnVal = (PBRD_EnVal)args;
//	DEVS_CMD_Reg param;
//	param.idxMain = m_index;
//	param.tetr = m_AdcTetrNum;
//	param.reg = ADM2IFnr_MODE0;
//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
//	if(BRDctrl_ADC_SETSKIPDATA == cmd)
//	{
//		pMode0->ByBits.Cnt2En = pEnVal->enable;
//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//		param.reg = ADM2IFnr_CNT2;
//		param.val = pEnVal->value;
//		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
//	}
//	else
//	{
//		pEnVal->enable = pMode0->ByBits.Cnt2En;
//		param.reg = ADM2IFnr_CNT2;
//		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//		pEnVal->value = param.val;
//	}
//	return BRDerr_OK;
//}
//

/*
// ***************************************************************************************
int CAdcSrv::CtrlIsBitsOverflow(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//PADC_STATUS pStatus = (PADC_STATUS)&param.val;
	//ULONG data = pStatus->ByBits.OverAdc0;
	//data |= (pStatus->ByBits.OverAdc1 << 1);
    // *(PULONG)args = data;
	param.reg = ADCnr_OVERFLOW;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	*(PULONG)args = param.val;
	return BRDerr_OK;
}
*/

//***************************************************************************************
int CAdcSrv::CtrlIsBitsOverflow(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
								)
{
	CModule* pModule = (CModule*)pDev;
	ULONG OverBits = 0;
	IsBitsOverflow(pModule, OverBits);
	*(ULONG*)args = OverBits;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlDcCoupling(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
//	if(pValChan->chan < 0 || pValChan->chan >= BRD_CHANCNT)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	ULONG value = (ULONG)pValChan->value;
	if(BRDctrl_ADC_SETDCCOUPLING == cmd)
		Status = SetDcCoupling(pModule, value, pValChan->chan);
	else
		Status = GetDcCoupling(pModule, value, pValChan->chan);
	pValChan->value = value;
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlInpFilter(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int				status = BRDerr_CMD_UNSUPPORTED;
	CModule			*pModule = (CModule*)pDev;
	BRD_ValChan		*pValChan = (BRD_ValChan*)args;
	ADCSRV_CFG		*pAdcSrvCfg = (ADCSRV_CFG*)m_pConfig;

	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;

	ULONG value = (ULONG)pValChan->value;

	if( BRDctrl_ADC_SETINPFILTER == cmd )
		status = SetInpFilter(pModule, value, pValChan->chan);
	else
		status = GetInpFilter(pModule, value, pValChan->chan);
	pValChan->value = value;

	return status;
}

//***************************************************************************************
int CAdcSrv::CtrlInpResist(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	//PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	// закомментировал, так как для FM212x1G канал 1024 означает сопротивление внешнего старта
	//if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
	//	return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	ULONG value = (ULONG)pValChan->value;
	if(BRDctrl_ADC_SETINPRESIST == cmd)
		Status = SetInpResist(pModule, value, pValChan->chan);
	else
		Status = GetInpResist(pModule, value, pValChan->chan);
	pValChan->value = value;
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlInpSrc(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	if(BRDctrl_ADC_SETINPSRC == cmd)
		Status = SetInpSrc(pModule, args);
	else
		Status = GetInpSrc(pModule, args);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlInpMux(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	//CModule* pModule = (CModule*)pDev;
	//PBRD_AdcInpMux pInpMux = (PBRD_AdcInpMux)args;
	//if(BRDctrl_ADC_SETINPMUX == cmd)
	//	Status = SetInpMux(pModule, pInpMux);
	//else
	//	Status = GetInpMux(pModule, pInpMux);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlDifMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	//CModule* pModule = (CModule*)pDev;
	//ULONG mode = *(PULONG)args;
	//if(BRDctrl_ADC_SETDIFMODE == cmd)
	//	Status = SetDifMode(pModule, mode);
	//else
	//	Status = GetDifMode(pModule, mode);
	//*(PULONG)args = mode;
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlSampleHold(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	//CModule* pModule = (CModule*)pDev;
	//ULONG mode = *(PULONG)args;
	//if(BRDctrl_ADC_SETSAMPLEHOLD == cmd)
	//	Status = SetSampleHold(pModule, mode);
	//else
	//	Status = GetSampleHold(pModule, mode);
	//*(PULONG)args = mode;
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlTestMode(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(PULONG)args;
	if(BRDctrl_ADC_SETTESTMODE == cmd)
		Status = SetTestMode(pModule, mode);
	else
		Status = GetTestMode(pModule, mode);
	*(PULONG)args = mode;
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlTestSeq(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG mode = *(PULONG)args;
	if(BRDctrl_ADC_SETTESTSEQ == cmd)
		Status = SetTestSeq(pModule, mode);
	else
		Status = GetTestSeq(pModule, mode);
	*(PULONG)args = mode;
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlDelay(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_StdDelay delay = (PBRD_StdDelay)args;
	if(BRDctrl_ADC_WRDELAY == cmd)
		Status = StdDelay(pModule, 1, delay);
	else
		if(BRDctrl_ADC_RSTDELAY == cmd)
			Status = StdDelay(pModule, 2, delay);
		else
			Status = StdDelay(pModule, 0, delay);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlSpecific(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_AdcSpec pSpec = (PBRD_AdcSpec)args;
	if(BRDctrl_ADC_SETSPECIFIC == cmd)
		Status = SetSpecific(pModule, pSpec);
	else
		Status = GetSpecific(pModule, pSpec);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlClkPhase(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
//	if(pValChan->chan < 0 || pValChan->chan >= BRD_CHANCNT)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	if(BRDctrl_ADC_SETCLKPHASE == cmd)
		Status = SetClkPhase(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetClkPhase(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlGainTuning(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	PBRD_ValChan pValChan = (PBRD_ValChan)args;
	PADCSRV_CFG pAdcSrvCfg = (PADCSRV_CFG)m_pConfig;
	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
//	if(pValChan->chan < 0 || pValChan->chan >= BRD_CHANCNT)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	if(BRDctrl_ADC_SETGAINTUNING == cmd)
		Status = SetGainTuning(pModule, pValChan->value, pValChan->chan);
	else
		Status = GetGainTuning(pModule, pValChan->value, pValChan->chan);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlTarget(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG target = *(ULONG*)args;
	if(BRDctrl_ADC_SETTARGET == cmd)
	{
		Status = SetTarget(pModule, target);
	}
	else
	{
		Status = GetTarget(pModule, target);
		*(ULONG*)args = target;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlGetPretrigEvent(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
							   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG EventStart;
	GetPretrigEvent(pModule, EventStart);
	*(ULONG*)args = EventStart;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlGetPreEventPrec(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
							   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_PREVPREC;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	*(ULONG*)args = param.val & 0xffff;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlGetBlkCnt(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
	param.reg = ADCnr_BLOCKCNTL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG counter = param.val & 0xffff;
	param.reg = ADCnr_BLOCKCNTH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	counter |= (param.val & 0xffff) << 16;
	*(ULONG*)args = counter;
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlDblClk(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG dbl_mode = *(ULONG*)args;
	if(BRDctrl_ADC_SETDBLCLK == cmd)
	{
		Status = SetDblClk(pModule, dbl_mode);
	}
	else
	{
		Status = GetDblClk(pModule, dbl_mode);
		*(ULONG*)args = dbl_mode;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlClkLocation(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG src_loc = *(ULONG*)args;
	if(BRDctrl_ADC_SETCLKLOCATION == cmd)
	{
		Status = SetClkLocation(pModule, src_loc);
	}
	else
	{
		Status = GetClkLocation(pModule, src_loc);
		*(ULONG*)args = src_loc;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlClkThr(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	double ClkThr = *(double*)args;
	if(BRDctrl_ADC_SETCLKTHR == cmd)
	{
		Status = SetClkThr(pModule, ClkThr);
		*(double*)args = ClkThr;
	}
	else
	{
		Status = GetClkThr(pModule, ClkThr);
		*(double*)args = ClkThr;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlExtClkThr(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	double ExtClkThr = *(double*)args;
	if(BRDctrl_ADC_SETEXTCLKTHR == cmd)
	{
		Status = SetExtClkThr(pModule, ExtClkThr);
		*(double*)args = ExtClkThr;
	}
	else
	{
		Status = GetClkThr(pModule, ExtClkThr);
		*(double*)args = ExtClkThr;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlSelfClbr(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	Status = SelfClbr(pModule);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlClkInv(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	int Status = BRDerr_CMD_UNSUPPORTED;
	CModule* pModule = (CModule*)pDev;
	ULONG ClkInv = *(ULONG*)args;
	if(BRDctrl_ADC_SETCLKINV == cmd)
	{
		Status = SetClkInv(pModule, ClkInv);
	}
	else
	{
		Status = GetClkInv(pModule, ClkInv);
		*(ULONG*)args = ClkInv;
	}
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlMu(
						   void		*pDev,		// InfoSM or InfoBM
						   void		*pServData,	// Specific Service Data
						   ULONG	cmd,		// Command Code (from BRD_ctrl())
						   void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	int				status;
	CModule			*pModule = (CModule*)pDev;

	if( BRDctrl_ADC_SETMU == cmd )
		status = SetMu(pModule, args);
	else
		status = GetMu(pModule, args);

	return status;
}

//***************************************************************************************
int CAdcSrv::CtrlStartSlave(
						   void		*pDev,		// InfoSM or InfoBM
						   void		*pServData,	// Specific Service Data
						   ULONG	cmd,		// Command Code (from BRD_ctrl())
						   void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	int				status;
	CModule			*pModule = (CModule*)pDev;
	ULONG			nStartSlave = *(ULONG*)args;

	if( BRDctrl_ADC_SETSTARTSLAVE == cmd )
		status = SetStartSlave( pModule, nStartSlave );
	else
		status = GetStartSlave( pModule, nStartSlave );
	*(ULONG*)args = nStartSlave;

	return status;
}

//***************************************************************************************
int CAdcSrv::CtrlClockSlave(
						   void		*pDev,		// InfoSM or InfoBM
						   void		*pServData,	// Specific Service Data
						   ULONG	cmd,		// Command Code (from BRD_ctrl())
						   void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	int				status;
	CModule			*pModule = (CModule*)pDev;
	ULONG			nClockSlave = *(ULONG*)args;

	if( BRDctrl_ADC_SETCLOCKSLAVE == cmd )
		status = SetClockSlave( pModule, nClockSlave );
	else
		status = GetClockSlave( pModule, nClockSlave );
	*(ULONG*)args = nClockSlave;

	return status;
}

//***************************************************************************************
int CAdcSrv::CtrlChanOrder(
						   void		*pDev,		// InfoSM or InfoBM
						   void		*pServData,	// Specific Service Data
						   ULONG	cmd,		// Command Code (from BRD_ctrl())
						   void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	int				status;
	CModule			*pModule = (CModule*)pDev;
	BRD_ItemArray	*pItemArray = (BRD_ItemArray*)args;

	status = GetChanOrder( pModule, pItemArray );

	return status;
}

//***************************************************************************************
int CAdcSrv::CtrlClrBitsOverflow(
								void		*pDev,		// InfoSM or InfoBM
								void		*pServData,	// Specific Service Data
								ULONG		cmd,		// Command Code (from BRD_ctrl())
								void		*args 		// Command Arguments (from BRD_ctrl())
							   )
{
	CModule* pModule = (CModule*)pDev;
	ULONG flags = *(PULONG)args;
	ClrBitsOverflow(pModule, flags);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlFifoReset (
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	FifoReset(pModule);
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlEnable (
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	ULONG enable = *(PULONG)args;
	AdcEnable(pModule, enable);
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_AdcTetrNum;
	//param.reg = ADM2IFnr_MODE0;
	//pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	//pMode0->ByBits.Start = enable;
	//pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//if(!pMode0->ByBits.Master)
	//{ // Master/Slave
	//	param.tetr = m_MainTetrNum;
	//	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	//	pMode0->ByBits.Start = enable;
	//	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	//}
	return BRDerr_OK;
}

//***************************************************************************************
int CAdcSrv::CtrlPrepareStart( void *pDev, void *pServData, ULONG cmd, void *args )
{
	CModule* pModule = (CModule*)pDev;
	return PrepareStart( pModule, args );
}

//***************************************************************************************
int CAdcSrv::CtrlIsComplex( void *pDev, void *pServData, ULONG cmd, void *args )
{
	CModule* pModule = (CModule*)pDev;
	return IsComplex( pModule, (U32*)args );
}

//***************************************************************************************
int CAdcSrv::CtrlFc( void *pDev, void *pServData, ULONG cmd, void *args )
{
	int			Status;
	CModule		*pModule = (CModule*)pDev;
	BRD_ValChan	*pValChan = (BRD_ValChan*)args;
	ADCSRV_CFG	*pAdcSrvCfg = (ADCSRV_CFG*)m_pConfig;

	if(pValChan->chan < 0 || pValChan->chan >= pAdcSrvCfg->MaxChan)
		return BRDerr_ADC_ILLEGAL_CHANNEL | BRDerr_WARN;
	if(BRDctrl_ADC_SETFC == cmd)
		Status = SetFc(pModule, &(pValChan->value), pValChan->chan);
	else
		Status = GetFc(pModule, &(pValChan->value), pValChan->chan);
	return Status;
}

//***************************************************************************************
int CAdcSrv::CtrlFifoStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	//CModule* pModule = (CModule*)pDev;
	//DEVS_CMD_Reg param;
	//param.idxMain = m_index;
	//param.tetr = m_AdcTetrNum;
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	//ULONG data = pStatus->AsWhole;
	//*(PULONG)args = data;
	//return BRDerr_OK;
	CModule* pModule = (CModule*)pDev;
	return GetFifoStatus(pModule,(U32*)args);
}

//***************************************************************************************
int CAdcSrv::CtrlGetData(
						void		*pDev,		// InfoSM or InfoBM
						void		*pServData,	// Specific Service Data
						ULONG		cmd,		// Command Code (from BRD_ctrl())
						void		*args 		// Command Arguments (from BRD_ctrl())
						)
{
	CModule* pModule = (CModule*)pDev;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_AdcTetrNum;
/*	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.FifoRes = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pMode0->ByBits.FifoRes = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
*/
	//PADM2IF_STATUS pStatus;
	//do
	//{
	//	param.reg = ADM2IFnr_STATUS;
	//	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//	pStatus = (PADM2IF_STATUS)&param.val;
	//} while(pStatus->ByBits.HalfFull);

	param.reg = ADM2IFnr_DATA;
	PBRD_DataBuf pBuf = (PBRD_DataBuf)args;
	param.pBuf = pBuf->pData;
	param.bytes = pBuf->size;
	pModule->RegCtrl(DEVScmd_REGREADSDIR, &param);

	//PULONG buf = (PULONG)pBuf->pData;
	//for(ULONG i = 0; i < pBuf->size / sizeof(ULONG); i++)
	//{
	//	pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//	buf[i] = param.val;
	//}
	
	//param.reg = ADM2IFnr_STATUS;
	//pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	//pStatus = (PADM2IF_STATUS)&param.val;

	return BRDerr_OK;
}

// ***************** End of file AdcCtrl.cpp ***********************
