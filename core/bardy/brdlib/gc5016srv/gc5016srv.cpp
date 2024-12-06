/*
 ***************** File Gc5016Srv.cpp ************
 *
 * BRD Driver for GC5016 service on ADMDDC5016
 *
 * (C) InSys by Sklyarov A. Nov. 2006
 *
 ******************************************************
*/

#include "module.h"
#include "gc5016srv.h"

#define	CURRFILE "GC5016SRV"

//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::CtrlRelease(
								void			*pDev,		// InfoSM or InfoBM
								void			*pServData,	// Specific Service Data
								ULONG			cmd,		// Command Code (from BRD_ctrl())
								void			*args 		// Command Arguments (from BRD_ctrl())
								)
{
//	CModule* pModule = (CModule*)pDev;
	return BRDerr_CMD_UNSUPPORTED; 
}

//***************************************************************************************
void CGc5016Srv::GetGcTetrNum(CModule* pModule)
{
	m_GcTetrNum = GetTetrNum(pModule, m_index,  DDC5016_TETR_ID );
	if(m_GcTetrNum==(-1))m_GcTetrNum = GetTetrNum(pModule, m_index,  DDC416_TETR_ID );
	if(m_GcTetrNum==(-1))m_GcTetrNum = GetTetrNum(pModule, m_index,  DDC214_TETR_ID );
	if(m_GcTetrNum==(-1))m_GcTetrNum = GetTetrNum(pModule, m_index,  DDCDR16_TETR_ID );

}
//***************************************************************************************
//***************************************************************************************

int CGc5016Srv::GetCfg(PBRD_GC5016Cfg pCfg)
{
	PGC5016SRV_CFG pSrvCfg = (PGC5016SRV_CFG)m_pConfig;
	pCfg->SubType = pSrvCfg->SubType;
	pCfg->SubVer = pSrvCfg->SubVer;
	pCfg->AdmConst = m_AdmConst.AsWhole;
	return BRDerr_OK;
}

//***************************************************************************************

//***************************************************************************************
int CGc5016Srv::WriteGcReg(CModule* pModule, void *args)
{
	PBRD_GC5016SetReg ptr=(PBRD_GC5016SetReg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_ADDR;
	param.val=(2<<8) | 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = ptr->page;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// set page
	
	param.reg = GC5016nr_ADDR;
	param.val= (ptr->reg<<8) | 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = ptr->data;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// write val to reg

    IPC_delay(1);
	return BRDerr_OK;

}
//***************************************************************************************
int CGc5016Srv::ReadGcReg(CModule* pModule,  void *args)
{
	PBRD_GC5016SetReg ptr=(PBRD_GC5016SetReg)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_ADDR;
	param.val=(2<<8) | 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = ptr->page;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// set page

	param.reg = GC5016nr_ADDR;
	param.val= (ptr->reg<<8) | 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = GC5016nr_READCOMMAND;	// set read mode
	param.val= 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg = GC5016nr_READDATA;	
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ptr->data=param.val&0xffff;

	return BRDerr_OK;
}

//***************************************************************************************

int CGc5016Srv::SetAdcGain(CModule* pModule, void *args)
{
	PBRD_GC5016AdcSetGain AdcGain=(PBRD_GC5016AdcSetGain)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_ADCGAIN;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PGC_ADCGAIN pAdcGain = (PGC_ADCGAIN)&param.val;
	if((AdcGain->maskAdc&1)>0)
		pAdcGain->ByBits.Gain0=AdcGain->gainAdc;
	if((AdcGain->maskAdc&2)>0)
		pAdcGain->ByBits.Gain1=AdcGain->gainAdc;
	if((AdcGain->maskAdc&4)>0)
		pAdcGain->ByBits.Gain2=AdcGain->gainAdc;
	if((AdcGain->maskAdc&8)>0)
		pAdcGain->ByBits.Gain3=AdcGain->gainAdc;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;

}
//***************************************************************************************
int CGc5016Srv::GetAdcOver(CModule* pModule,void *args)
{
	U32 *ptr=(U32*)args; 
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_ADCOVER;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	*ptr=param.val;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;

}
//***************************************************************************************
int CGc5016Srv::AdcEnable(CModule* pModule, void *args)
{

	U32 *ptr=(U32*)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_ADCENABLE;
	param.val = *ptr;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetDdcFc(CModule* pModule, void *args)
{
	
	PBRD_GC5016SetFc pDdcSetFc=(PBRD_GC5016SetFc)args;
	U32	 frPage,addr, chMask = pDdcSetFc->maskChans,ddcmode,chM;
	__int64	codefreq =0;
	unsigned int codeDphase;
	unsigned int codephase;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_DDCMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ddcmode=param.val;
	if(ddcmode>0)	// SplitIQ, DoubleRate
	{
		chM=0;
		if(chMask&1)chM=0x3;
		if(chMask&2)chM |=0xc;
	}else
	{
		chM=chMask;
	}
    codefreq=CalcCodeNcoFreq(ddcmode, pDdcSetFc->Fc,&codeDphase );
	for(int i=0; i<4;i++)
	{
		if( (chM >> i) & 1 )
		{
			if(i<2)frPage=0x80;
			else frPage=0xa0;
		
			param.reg = GC5016nr_ADDR;
			param.val=(2<<8) | 1;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val = frPage;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// set page

			if((i&1)==0)addr = (0x12<<8) | 1;
			else addr = (0x1a<<8) | 1;

			param.reg = GC5016nr_ADDR;
			param.val=addr;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val =(U32)( codefreq&0xffff);						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 lsb

			param.reg = GC5016nr_ADDR;
			param.val=addr+(1<<8);
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val =(U32)( (codefreq>>16)&0xffff);						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 mid

			param.reg = GC5016nr_ADDR;
			param.val=addr+(2<<8);
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val = (U32)((codefreq>>32)&0xffff);						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 msb

			if(ddcmode==2)// DoubleRate
			{
				if((i%2)==0)
				{
					addr = (0x11<<8) | 1;
					codephase=0;
					param.reg = GC5016nr_ADDR;
					param.val=addr;
					pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
					param.reg = GC5016nr_WRITEDATA;
					param.val = (U32)(codephase & 0xffff);						
					pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 lsb
				}
				if((i%2)==1)
				{
					addr = (0x19<<8) | 1;
/*
					param.reg = GC5016nr_ADDR;
					param.val= addr;
					pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

					param.reg = GC5016nr_READCOMMAND;	// set read mode
					param.val= 0;
					pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

					param.reg = GC5016nr_READDATA;	
					pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//					codephase=param.val&0xffff;

					codephase +=codeDphase;
*/
					codephase=codeDphase;
					param.reg = GC5016nr_ADDR;
					param.val=addr;
					pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
					param.reg = GC5016nr_WRITEDATA;
					param.val = (U32)(codephase & 0xffff);						
					pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 lsb
				}		
			}
		}
	}
	return BRDerr_OK;

}
//*************************************************************************************
int CGc5016Srv::SetDdcFcExt(CModule* pModule, void *args)
{
	
	PBRD_GC5016SetFc pDdcSetFc=(PBRD_GC5016SetFc)args;
	U32	 frPage,addr,maskDdc,chMask,chanMask = pDdcSetFc->maskChans,ddcmode,chM;
	__int64	codefreq =0;
	unsigned int codeDphase;
	unsigned int codephase;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_DDCMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ddcmode=param.val;
	for(int j=0;j<4;j++)
	{
	 if((((chanMask>>(4*j))&0xf)!=0))
	 {
		chMask = (chanMask>>(4*j))&0xf;
		maskDdc=1<<j;
		if(ddcmode>0)	// SplitIQ, DoubleRate
		{
			chM=0;
			if(chMask&1)chM=0x3;
			if(chMask&2)chM |=0xc;
		}else
		{
			chM=chMask;
		}
		codefreq=CalcCodeNcoFreq(ddcmode, pDdcSetFc->Fc,&codeDphase );
		for(int i=0; i<4;i++)
		{
			if( (chM >> i) & 1 )
			{
				if(i<2)frPage=0x80;
				else frPage=0xa0;
		
				param.reg = GC5016nr_ADDR;
				param.val=(2<<8) | maskDdc;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				param.reg = GC5016nr_WRITEDATA;
				param.val = frPage;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// set page

				if((i&1)==0)addr = (0x12<<8) | maskDdc;
				else addr = (0x1a<<8) | maskDdc;

				param.reg = GC5016nr_ADDR;
				param.val=addr;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				param.reg = GC5016nr_WRITEDATA;
				param.val =(U32)( codefreq&0xffff);						
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 lsb

				param.reg = GC5016nr_ADDR;
				param.val=addr+(1<<8);
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				param.reg = GC5016nr_WRITEDATA;
				param.val =(U32)( (codefreq>>16)&0xffff);						
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 mid

				param.reg = GC5016nr_ADDR;
				param.val=addr+(2<<8);
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
				param.reg = GC5016nr_WRITEDATA;
				param.val = (U32)((codefreq>>32)&0xffff);						
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 msb

				if(ddcmode==2)// DoubleRate
				{
					if((i%2)==1)
					{
						addr = (0x19<<8) | maskDdc;
						param.reg = GC5016nr_ADDR;
						param.val= addr;
						pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

						param.reg = GC5016nr_READCOMMAND;	// set read mode
						param.val= 0;
						pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

						param.reg = GC5016nr_READDATA;	
						pModule->RegCtrl(DEVScmd_REGREADIND, &param);
						codephase=param.val&0xffff;

						codephase +=codeDphase;
						param.reg = GC5016nr_ADDR;
						param.val=addr;
						pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
						param.reg = GC5016nr_WRITEDATA;
						param.val = (U32)(codephase & 0xffff);						
						pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	// 16 lsb
					}		
				}
			}
		}
	 }
	}
	return BRDerr_OK;

}
//*************************************************************************************
int CGc5016Srv::SetAdmMode(CModule* pModule,void *args)
{
	U32			*ptr = (U32*)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_ADMMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
//	param.val |= (*ptr)&1;

--	param.val = (*ptr)&1;
	param.val = (*ptr)&3;  // new 5.3.2011
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;

}
//*************************************************************************************
int CGc5016Srv::SetDdcMode(CModule* pModule,void *args)
{
	U32			*ptr = (U32*)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_DDCMODE;
	param.val = (*ptr)&3;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;

}
//*************************************************************************************
//=*********************************************************************************************************
__int64 CGc5016Srv::CalcCodeNcoFreq(int ddcmode,U32 freq, unsigned int *codeDphase)
{
      __int64 code=0x1000000000000ULL;
      unsigned int mph=0x8000;
//	  long double SystemClock=200000000;
	  long double fh  =(-1)*(long double)freq;
	  unsigned int hexDph=0;
      long double dph;
	  if(ddcmode==2)// DoubleRate
	  {
			fh *=2;

            dph=fh/(long double)m_SystemClock;
            hexDph=dph *mph;

	  }
	  fh /= (long double)m_SystemClock;
//	  fh /= SystemClock;
  	  code=code*fh;
	  *codeDphase=hexDph;
	  return code;
}

//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::SetChanMask(CModule* pModule, ULONG mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	pChan1->ByBits.ChanSel = mask;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::GetChanMask(CModule* pModule, ULONG& mask)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_CHAN1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_CHAN1 pChan1 = (PADM2IF_CHAN1)&param.val;
	mask = pChan1->ByBits.ChanSel;

	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::GetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStrMode = (PADM2IF_STMODE)&param.val;
		pStartMode->startSrc	= pStrMode->ByBits.SrcStart;
		pStartMode->trigStopSrc	= pStrMode->ByBits.SrcStop;
		pStartMode->startInv	= pStrMode->ByBits.InvStart;
		pStartMode->stopInv		= pStrMode->ByBits.InvStop;
		pStartMode->trigOn		= pStrMode->ByBits.TrigStart;
		pStartMode->reStartMode = pStrMode->ByBits.Restart;
	}else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_STMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			PADM2IF_STMODE pStrMode = (PADM2IF_STMODE)&param.val;
			pStartMode->startSrc	= pStrMode->ByBits.SrcStart;
			pStartMode->trigStopSrc	= pStrMode->ByBits.SrcStop;
			pStartMode->startInv	= pStrMode->ByBits.InvStart;
			pStartMode->stopInv		= pStrMode->ByBits.InvStop;
			pStartMode->trigOn		= pStrMode->ByBits.TrigStart;
			pStartMode->reStartMode = pStrMode->ByBits.Restart;
		}else
			return BRDerr_NOT_ACTION; // функция в режиме Slave не выполнима
	}

	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::SetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	if(pMode0->ByBits.Master)
	{ // Single
		param.reg = ADM2IFnr_STMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		PADM2IF_STMODE pStrMode = (PADM2IF_STMODE)&param.val;
		pStrMode->ByBits.SrcStart  = pStartMode->startSrc;
		pStrMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
		pStrMode->ByBits.InvStart  = pStartMode->startInv;
		pStrMode->ByBits.InvStop   = pStartMode->stopInv;
		pStrMode->ByBits.TrigStart = pStartMode->trigOn;
		pStrMode->ByBits.Restart = pStartMode->reStartMode;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		return BRDerr_OK;
	}else
	{ // Master/Slave
		param.tetr = m_MainTetrNum;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(pMode0->ByBits.Master)
		{ // Master
			param.reg = ADM2IFnr_STMODE;
			pModule->RegCtrl(DEVScmd_REGREADIND, &param);
			PADM2IF_STMODE pStrMode = (PADM2IF_STMODE)&param.val;
			pStrMode->ByBits.SrcStart  = pStartMode->startSrc;
			pStrMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
			pStrMode->ByBits.InvStart  = pStartMode->startInv;
			pStrMode->ByBits.InvStop   = pStartMode->stopInv;
			pStrMode->ByBits.TrigStart = pStartMode->trigOn;
			pStrMode->ByBits.Restart = pStartMode->reStartMode;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			return BRDerr_OK;
		}else
			return BRDerr_NOT_ACTION; // функция в режиме Slave не выполнима
	}

}
//=**************************************************************************************

int CGc5016Srv::SetClkMode(CModule* pModule, PBRD_ClkMode pClkMode)
{
	PGC5016SRV_CFG pDdcSrvCfg= (PGC5016SRV_CFG)m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_FSRC;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);

	if(pClkMode->src==0)	// internal
	{ 
		param.val = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		pClkMode->value = pDdcSrvCfg->SubIntClk;
		m_SystemClock=pClkMode->value;
	//	pDdcSrvCfg->SubRefGen=pClkMode->value;
		return BRDerr_OK;
	}
	if(pClkMode->src==1)	//external
	{ 
		param.val = 0x81;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		pDdcSrvCfg->SubExtClk=(U32)pClkMode->value;
		m_SystemClock=(U32)pClkMode->value;
		return BRDerr_OK;
	}
	if(pDdcSrvCfg->SubType == ADMDDC214x400M && (pDdcSrvCfg->SubVer == 2 || pDdcSrvCfg->SubVer == 32))
	{
		int ret = BRDerr_OK;
		switch(pClkMode->src){
			case 2:		pClkMode->value = 150000000; break;
			case 3:		pClkMode->value = 200000000; break;
			case 4:		pClkMode->value = 240000000; break;
			case 5:		pClkMode->value = 300000000; break;
			case 6:		pClkMode->value = 400000000; break;
			default:	pClkMode->value = 100000000; ret = BRDerr_BAD_PARAMETER;break;
		}

		m_SystemClock=pClkMode->value;
		param.val = pClkMode->src;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		return ret;

	}else
	{	
		param.val = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		pClkMode->value = pDdcSrvCfg->SubIntClk;
		m_SystemClock=pClkMode->value;
		return BRDerr_BAD_PARAMETER;
	}
}

//=**************************************************************************************
int CGc5016Srv::GetClkMode(CModule* pModule, PBRD_ClkMode pClkMode)
{
	PGC5016SRV_CFG pDdcSrvCfg= (PGC5016SRV_CFG)m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_FSRC;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	if(param.val==0x81)			// external
	{ 
		pClkMode->src = 1;		
		pClkMode->value=pDdcSrvCfg->SubExtClk;
	}else						  //internal
	{
		pClkMode->src = param.val;
		switch(pClkMode->src){
			case 0:		pClkMode->value = pDdcSrvCfg->SubIntClk; break;
			case 2:		pClkMode->value = 150000000; break;
			case 3:		pClkMode->value = 200000000; break;
			case 4:		pClkMode->value = 240000000; break;
			case 5:		pClkMode->value = 300000000; break;
			case 6:		pClkMode->value = 400000000; break;
			default:	pClkMode->value = 100000000; break;
		}
	}
		return BRDerr_OK;
}

//=**************************************************************************************
int CGc5016Srv::SetDdcSync(CModule* pModule,void* args)
{
	PBRD_GC5016SetSync DdcSetSync=(PBRD_GC5016SetSync)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_DDCSYNC;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PGC_DDCSYNC pDdcSync = (PGC_DDCSYNC)&param.val;

	pDdcSync->ByBits.SyncMode=DdcSetSync->syncMode;
	pDdcSync->ByBits.Slave=DdcSetSync->slave;
	pDdcSync->ByBits.ProgSync=0;
	pDdcSync->ByBits.ResDiv=0;

	pDdcSync->ByBits.ResDiv=DdcSetSync->resDiv;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	pDdcSync->ByBits.ResDiv=0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
    IPC_delay(1);
	pDdcSync->ByBits.ProgSync=DdcSetSync->progSync;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::ProgramDdc(CModule* pModule,void* args)
{
	PBRD_GC5016Program pDdcProgram=(PBRD_GC5016Program)args;
	U32	addr;
	U32	nItems = pDdcProgram->nItems;
	U32	*pData = pDdcProgram->pData;
	int	nPage = 0xFF, nCurPage = 0xFF;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

    for(int i=0; i<(int)nItems; i++ )
	{
		nPage = (pData[i]>>24) & 0xFF;
		if( nCurPage!=nPage )
		{
			nCurPage = nPage;
			addr = ((0x2)<<8)  | 1;					// Register 2 (address page) 

			param.reg = GC5016nr_ADDR;
			param.val=addr;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val = nCurPage;						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	
		}
		addr = ((pData[i]>>8)&0xFF00) | 1;
		param.reg = GC5016nr_ADDR;
		param.val=addr;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = GC5016nr_WRITEDATA;
		param.val = pData[i]&0xFFFF;						
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	}
	if( 0 > VerifyProgDdc(pModule,pData, nItems ) )
				return BRDerr_ERROR;
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::VerifyProgDdc(CModule* pModule,U32 *regs,int nItems)
{

	U32		addr, reg;
	U32		nPage=0xFF, nCurPage=0xFF;
	PGC5016SRV_CFG pDdcSrvCfg = (PGC5016SRV_CFG)m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	for( int i=0; i<nItems; i++ )
	{
		nPage = (regs[i] >> 24) & 0xFF;
		if( nCurPage!=nPage )
		{
			nCurPage = nPage;
			addr = ((0x2)<<8)  | 1;					// Register 2 (address page) 

			param.reg = GC5016nr_ADDR;
			param.val=addr;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val = nCurPage;						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	
		}
		addr = ((regs[i]>>8)&0xFF00) | 1; 
		reg = (addr>>8)& 0xFF;
		param.reg = GC5016nr_ADDR;
		param.val=addr;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = GC5016nr_READCOMMAND;	// set read mode
		param.val= 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = GC5016nr_READDATA;	
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		
		param.val &=0xffff;
		if(reg==0)param.val &=0xfffc; 
		if(reg==2)continue; 
		if( ((nPage==0x13) || (nPage==0x33) || (nPage==0x53) || (nPage==0x73)) && ((reg==0x1c )|| (reg==0x1a ) || (reg==0x1b )))continue; 
		if( ((nPage==0x80) || (nPage==0xa0)) && ((reg==0x10) || (reg==0x18)))continue; 
		if((nPage==0x10) || (nPage==0x30)|| (nPage==0x50)|| (nPage==0x70))continue; 

		if(param.val!=(regs[i]&0xffff))
		{
			if(pDdcSrvCfg->SubType != ADMDDC5016)return BRDerr_ERROR;

		}
	}

	return BRDerr_OK; 

}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::ProgramDdcExt(CModule* pModule,void* args)
{
	PBRD_GC5016ProgramExt pDdcProgramExt=(PBRD_GC5016ProgramExt)args;
	U32	addr;
	U32	maskDdc = 0xf & pDdcProgramExt->maskDdc;
	U32	nItems = pDdcProgramExt->nItems;
	U32	*pData = pDdcProgramExt->pData;
	int	nPage = 0xFF, nCurPage = 0xFF;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

    for(int i=0; i<(int)nItems; i++ )
	{
		nPage = (pData[i]>>24) & 0xFF;
		if( nCurPage!=nPage )
		{
			nCurPage = nPage;
			addr = ((0x2)<<8)  | maskDdc;					// Register 2 (address page) 

			param.reg = GC5016nr_ADDR;
			param.val=addr;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val = nCurPage;						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	
		}
		addr = ((pData[i]>>8)&0xFF00) | maskDdc;
		param.reg = GC5016nr_ADDR;
		param.val=addr;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = GC5016nr_WRITEDATA;
		param.val = pData[i]&0xFFFF;						
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	}
	U32 mask = 0;
	U32 errmask = 0;
	for(int i=0;i<4;i++)
	{
		mask = maskDdc&(1<<i);	
		if(mask!=0)
		{
			if(0>VerifyProgDdcExt(pModule,pData, nItems, mask ))errmask |=mask;
		}

	}
	pDdcProgramExt->errMask=errmask;
	if( 0 > errmask)return BRDerr_ERROR;
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::VerifyProgDdcExt(CModule* pModule,U32 *regs,int nItems,U32 maskDdc)
{

	U32		addr, reg;
	U32		nPage=0xFF, nCurPage=0xFF;
	PGC5016SRV_CFG pDdcSrvCfg = (PGC5016SRV_CFG)m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	for( int i=0; i<nItems; i++ )
	{
		nPage = (regs[i] >> 24) & 0xFF;
		if( nCurPage!=nPage )
		{
			nCurPage = nPage;
			addr = ((0x2)<<8)  | maskDdc;					// Register 2 (address page) 

			param.reg = GC5016nr_ADDR;
			param.val=addr;
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			param.reg = GC5016nr_WRITEDATA;
			param.val = nCurPage;						
			pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	
		}
		addr = ((regs[i]>>8)&0xFF00) | maskDdc; 
		reg = (addr>>8)& 0xFF;
		param.reg = GC5016nr_ADDR;
		param.val=addr;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = GC5016nr_READCOMMAND;	// set read mode
		param.val= 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		param.reg = GC5016nr_READDATA;	
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		
		param.val &=0xffff;
		if(reg==0)param.val &=0xfffc; 
		if(reg==2)continue; 
		if( ((nPage==0x13) || (nPage==0x33) || (nPage==0x53) || (nPage==0x73)) && ((reg==0x1c )|| (reg==0x1a ) || (reg==0x1b )))continue; 
		if( ((nPage==0x80) || (nPage==0xa0)) && ((reg==0x10) || (reg==0x18)))continue; 
		if((nPage==0x10) || (nPage==0x30)|| (nPage==0x50)|| (nPage==0x70))continue; 

		if(param.val!=(regs[i]&0xffff))
		{
			if(pDdcSrvCfg->SubType != ADMDDC5016)return BRDerr_ERROR;
//			return BRDerr_ERROR;

		}
	}

	return BRDerr_OK; 

}
//***************************************************************************************

//***************************************************************************************

int CGc5016Srv::StartDdc(CModule* pModule)
{
	PGC5016SRV_CFG pDdcSrvCfg = (PGC5016SRV_CFG)m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	param.reg = GC5016nr_ADDR;
	param.val=0x1;		
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = 0x100;				// Unreset DDC				
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	param.reg = GC5016nr_ADDR;
	param.val=0x301;		
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	if(pDdcSrvCfg->SubType == ADMDDC416x100M || pDdcSrvCfg->SubType == ADMDDC214x400M)
		param.val = 0x1ff;
	else
		param.val = 0x1c0;					// Enable Output DDC				
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	return BRDerr_OK; 
}
//***************************************************************************************
int CGc5016Srv::StopDdc(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	param.reg = GC5016nr_ADDR;
	param.val=0x1;		
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = 0xfff0;					// Reset DDC				
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	return BRDerr_OK; 
}
//***************************************************************************************

int CGc5016Srv::StartDdcExt(CModule* pModule,U32 maskDdc)
{
	PGC5016SRV_CFG pDdcSrvCfg = (PGC5016SRV_CFG)m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	param.reg = GC5016nr_ADDR;
	param.val=maskDdc;		
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = 0x100;				// Unreset DDC				
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	param.reg = GC5016nr_ADDR;
	param.val=0x300 | maskDdc;		
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	if(pDdcSrvCfg->SubType == ADMDDC416x100M || pDdcSrvCfg->SubType == ADMDDC214x400M)
		param.val = 0x1ff;
	else
		param.val = 0x1c0;					// Enable Output DDC				
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	return BRDerr_OK; 
}
//***************************************************************************************
int CGc5016Srv::StopDdcExt(CModule* pModule,U32 maskDdc)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	param.reg = GC5016nr_ADDR;
	param.val= maskDdc;		
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_WRITEDATA;
	param.val = 0xfff0;					// Reset DDC				
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);	

	return BRDerr_OK; 
}
//***************************************************************************************
//***************************************************************************************
int CGc5016Srv::SetTarget(CModule* pModule, ULONG target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Out = target;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::GetTarget(CModule* pModule, ULONG& target)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	target = pMode1->ByBits.Out;
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetTestMode(CModule* pModule, ULONG mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	pMode1->ByBits.Test = mode;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::GetTestMode(CModule* pModule, ULONG& mode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = ADM2IFnr_MODE1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE1 pMode1 = (PADM2IF_MODE1)&param.val;
	mode = pMode1->ByBits.Test;
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
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
int CGc5016Srv::GetCnt(CModule* pModule, ULONG numreg, PBRD_EnVal pEnVal)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
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
int CGc5016Srv::SetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
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
int CGc5016Srv::GetTitleMode(CModule* pModule, PBRD_EnVal pTitleMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

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
//***************************************************************************************
int CGc5016Srv::SetTitleData(CModule* pModule, PULONG pTitleData)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_TLDATAL;
	param.val = (*pTitleData) & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	param.reg = GC5016nr_TLDATAH;
	param.val = ((*pTitleData) >> 16) & 0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}

//***************************************************************************************
int CGc5016Srv::GetTitleData(CModule* pModule, PULONG pTitleData)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg = GC5016nr_TLDATAL;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	ULONG tldata = param.val & 0xffff;
	param.reg = GC5016nr_TLDATAH;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	tldata |= param.val << 16;
	*pTitleData = tldata;
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetSpecific(CModule* pModule, PBRD_GC5016Spec pSpec)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	switch(pSpec->command)
	{
	case GC5016cmd_SUMENABLE:
		param.reg = GC5016nr_ADMMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		param.val |=2;
	    pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		break;
	case GC5016cmd_SUMDISABLE:
		param.reg = GC5016nr_ADMMODE;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		param.val &=~(2);
	    pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		break;
	case GC5016cmd_SUMCOUNT:
		param.reg = GC5016nr_SUMCNT;
		param.val =*((U32*)pSpec->arg);
	    pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		break;
	case GC5016cmd_SUMSHIFT:
		param.reg = GC5016nr_SUMSHIFT;
		param.val =*((U32*)pSpec->arg);
	    pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		break;
	}
	return BRDerr_OK;

}
//***************************************************************************************
int CGc5016Srv::SetFlt(CModule* pModule, PBRD_SetFlt pSetFlt)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	U32 *pData=pSetFlt->pData;
	U32 cont=0;
	PCONTFLT pCont=(PCONTFLT)&cont;
	int size = pSetFlt->sizeFlt;
	pCont->ByBits.SizeFlt = size;
	pCont->ByBits.EnableFlt = pSetFlt->enableFlt;
	param.val= cont;
	param.reg = GC5016nr_CONTFLT;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	pCont->ByBits.RstFlt = 1; // Reset address counter
	param.val= cont;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	pCont->ByBits.RstFlt = 0;
	param.val= cont;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	for(int i=0;i<size; i++)
	{
		param.val= pData[i] & 0xffff;
		param.reg = GC5016nr_RDATAFLT;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.val= (pData[i]>>16) & 0xffff;
		param.reg = GC5016nr_IDATAFLT;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	}

	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetFft(CModule* pModule, PBRD_SetFft pSetFft)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	U32 *pWind=pSetFft->pWind;
	U32 cont=0;
	PCONTFFT pCont=(PCONTFFT)&cont;
	pCont->ByBits.EnableCor = pSetFft->enableCor;
	pCont->ByBits.EnableFft = pSetFft->enableFft;
	pCont->ByBits.EnableOneChan = pSetFft->enableOneChan;
	param.val= cont;
	param.reg = GC5016nr_CONTFFT;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	pCont->ByBits.RstCor = 1; // Reset address counter
	param.val= cont;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	pCont->ByBits.RstCor = 0;
	param.val= cont;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	for(int i=0;i<16; i++)
	{
		param.val= pWind[i] & 0xffff;
		param.reg = GC5016nr_RWINDFFT;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		param.val= (pWind[i]>>16) & 0xffff;
		param.reg = GC5016nr_IWINDFFT;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	}

	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetFrame(CModule* pModule, PBRD_SetFrame pSetFrame)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;

	param.val= pSetFrame -> SkipSamplesString & 0xffff;
	param.reg = GC5016nr_SKIPSAMPLES;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.val= pSetFrame ->GetSamplesString & 0xffff;
	param.reg = GC5016nr_GETSAMPLES;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.val= (pSetFrame ->EnableHeaderString & 1) | ((pSetFrame ->EnableDelayStart & 1)<<1);
	param.reg = GC5016nr_HEADERENABLE;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.val= pSetFrame ->HeaderString;
	param.reg = GC5016nr_HEADERFRAME;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.val= pSetFrame ->DelayStart;
	param.reg = GC5016nr_DELAYSTART;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
}
//***************************************************************************************
int CGc5016Srv::SetNchans(CModule* pModule,void *args)
{
	U32 *ptr=(U32*)args; 
	DEVS_CMD_Reg param;

	switch((*ptr)&0x1f){
		case 2:		param.val = 0; break;
		case 4:		param.val = 1; break;
		case 6:		param.val = 2; break;
		case 8:		param.val = 3; break;
		case 10:	param.val = 4; break;
		case 12:	param.val = 5; break;
		case 14:	param.val = 6; break;
		case 16:	param.val = 7; break;
		default:	param.val = 0; break;
	}
	param.idxMain = m_index;
	param.tetr = m_GcTetrNum;
	param.reg =	GC5016nr_NUMCHANS;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;

}
//***************************************************************************************
int CGc5016Srv::GetSpecific(CModule* pModule, PBRD_GC5016Spec pSpec)
{


	return BRDerr_OK;
}
//***************************************************************************************
