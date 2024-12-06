/*
 ***************** File DdsSrv.cpp ************
 *
 * BRD Driver for DDS service on Base
 *
 * (C) InSys by Sklyarov A. Mar 2007
 *
 ******************************************************
*/

#include "module.h"
#include "ddssrv.h"

#define	CURRFILE "DDSSRV"

//***************************************************************************************
int CDdsSrv::SetClkMode(CModule* pModule, PBRD_ClkMode pClkMode)
{
	PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG )m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;


	if(pClkMode->src==1)	// internal
	{ 
		pControl1->ByBits.RefClkSrc = 1;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

		pClkMode->value=pDdsSrvCfg->BaseRefClk;
		return BRDerr_OK;
	}
	if(pClkMode->src==0)	//external
	{ 
		pControl1->ByBits.RefClkSrc = 0;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		pDdsSrvCfg->ExtRefClk=(U32)pClkMode->value;
		return BRDerr_OK;
	}
	return BRDerr_BAD_PARAMETER; 
}

//***************************************************************************************
int CDdsSrv::GetClkMode(CModule* pModule, PBRD_ClkMode pClkMode)
{
	PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG )m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	if(pControl1->ByBits.RefClkSrc==1)	// internal
	{ 
		pClkMode->src = 1;
		pClkMode->value=pDdsSrvCfg->BaseRefClk;
	}
	else
	{
		pClkMode->src = 0;			//external
		pClkMode->value=pDdsSrvCfg->ExtRefClk;
	}
	return BRDerr_OK;
}


//***************************************************************************************
int CDdsSrv::SetOutFreq(CModule* pModule, double  *OutFreq ,int *ScaleCP,int *dividerM)
{
	double RefClk;
	PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG )m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	if(pControl1->ByBits.RefClkSrc==1)	
	{ 
		RefClk=pDdsSrvCfg->BaseRefClk; // internal
	}else
	{
		RefClk=pDdsSrvCfg->ExtRefClk; //external
	}
	int	clkSel,resDiv0,div0,resDiv1,div1;
	if(m_DdsVer)
		SetCfgParamAD9956_V2(&CfgAD9956, RefClk, OutFreq,&resDiv0,&div0,&resDiv1,&div1,ScaleCP,dividerM);
	else
		SetCfgParamAD9956(&CfgAD9956, RefClk, OutFreq,&clkSel,&resDiv0,&div0,&resDiv1,&div1,ScaleCP,dividerM);
	int res=ProgAD9956(pModule,&CfgAD9956);
	if(!m_DdsVer)
		pControl1->ByBits.ClkSel=clkSel;
	pControl1->ByBits.ResDiv0=resDiv0;
	pControl1->ByBits.Div0=div0;	
	pControl1->ByBits.ResDiv1=resDiv1;
	pControl1->ByBits.Div1=div1;	

	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);


	return res;
}
//***************************************************************************************
int	CDdsSrv::SetCfgParamAD9956(PAD9956_CFG pcfg, double refClk, double *outFreq, int*clkSel, int *resDiv0, int *div0,int *resDiv1, int *div1,int *scale,int *divM)
{
	__int64 FTW;	
	int R;
	double CompClk=refClk/(*divM);
	double Fsys;
	if(refClk>=12500000)
	{
		pcfg->CtrlCFR2.ByBytes.Byte2=0x48;
		R=4;

	}else
	{
		pcfg->CtrlCFR2.ByBytes.Byte2=0x68;
		R=8;
	}
	pcfg->CtrlCFR1.ByBytes.Byte0=0x10;
	pcfg->CtrlCFR1.ByBytes.Byte1=0x0;
	pcfg->CtrlCFR1.ByBytes.Byte2=0x0;
	pcfg->CtrlCFR1.ByBytes.Byte3=0x0;

	pcfg->CtrlCFR2.ByBytes.Byte0=*scale-1;
	pcfg->CtrlCFR2.ByBytes.Byte3=0x92;
	pcfg->CtrlCFR2.ByBytes.Byte4=0x0;
	if((int)*outFreq<=50000000) // <=50 MHz
	{
	if(refClk>=12500000)
	{
		pcfg->CtrlCFR2.ByBytes.Byte2=0x48;
//		pcfg->CtrlCFR2.ByBytes.Byte2=0x11; //for VCO=400 MHz !!!
		R=4;
	}else
	{
		pcfg->CtrlCFR2.ByBytes.Byte2=0x68;
		R=8;
	}
		pcfg->CtrlCFR2.ByBytes.Byte1=(*divM-1);
		pcfg->CtrlCFR2.ByBytes.Byte1 |=0xF0;
		FTW = ROUND64(((*outFreq) * pow(2.0, 44)*(*divM)) / refClk);
		*outFreq = double(FTW * refClk /(( pow(2.0, 44)*(*divM))));
		*clkSel=0;
		*resDiv0=0;
		*div0=7;
		*resDiv1=0;
		*div1=0;
	}
	if((int)*outFreq>50000000) // >50 MHz
	{
		pcfg->CtrlCFR2.ByBytes.Byte1=(*divM-1);
		R=4;
		pcfg->CtrlCFR2.ByBytes.Byte2=0x48;
//		pcfg->CtrlCFR2.ByBytes.Byte2=0x11; //for VCO=400 MHz !!!

		if((int)*outFreq<=100000000  ) // >50..100 MHz
		{
			Fsys=(*outFreq)*16/R;								// D=16
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/16*R;
			*clkSel=1;
			*resDiv0=1;
			*div0=7;
			*resDiv1=1;
			*div1=0;

		}
		if((int)*outFreq<=200000000 && (int)*outFreq >100000000) // >100-200 MHz
		{
			Fsys=(*outFreq)*8/R;								// D=8
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/8*R;
			*clkSel=1;
			*resDiv0=1;
			*div0=6;
			*resDiv1=1;
			*div1=4;
		}
		if((int)*outFreq<=400000000 && (int)*outFreq >200000000) // >200...400 MHz
		{
			Fsys=(*outFreq)*4/R;								// D=4
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/4*R;
			*clkSel=1;
			*resDiv0=1;
			*div0=5;
			*resDiv1=1;
			*div1=5;
		}
		if((int)*outFreq<=800000000 && (int)*outFreq >400000000) // >400...800 MHz
		{
			Fsys=(*outFreq)*2/R;								// D=2
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/2*R;
			*clkSel=1;
			*resDiv0=1;
			*div0=4;
			*resDiv1=1;
			*div1=6;
		}
		if((int)*outFreq<=1600000000 && (int)*outFreq >800000000) // >800...1600 MHz
		{
			Fsys=(*outFreq)/R;								// D=1
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )*R;
			*clkSel=1;
			*resDiv0=1;
			*div0=0;
			*resDiv1=1;
			*div1=7;
		}
	}
	pcfg->CtrlPCR0.Byte0 = BYTE(FTW & 0xff);
	pcfg->CtrlPCR0.Byte1 = BYTE((FTW >> 8) & 0xff);
	pcfg->CtrlPCR0.Byte2 = BYTE((FTW >> 16) & 0xff);
	pcfg->CtrlPCR0.Byte3 = BYTE((FTW >> 24) & 0xff);
	pcfg->CtrlPCR0.Byte4 = BYTE((FTW >> 32) & 0xff);
	pcfg->CtrlPCR0.Byte5 = BYTE((FTW >> 40) & 0xff);
	pcfg->CtrlPCR0.Byte7 = 0;
	pcfg->CtrlPCR0.Byte6 = 0;

	return BRDerr_OK;
}

//***************************************************************************************
int	CDdsSrv::SetCfgParamAD9956_V2(PAD9956_CFG pcfg, double refClk, double *outFreq, int *resDiv0, int *div0,int *resDiv1, int *div1,int *scale,int *divM)
{
	__int64 FTW;	
	int R;
	double CompClk=refClk/(*divM);
	double Fsys;
	pcfg->CtrlCFR1.ByBytes.Byte0=0x10;
	pcfg->CtrlCFR1.ByBytes.Byte1=0x40;
	pcfg->CtrlCFR1.ByBytes.Byte2=0x0;
	pcfg->CtrlCFR1.ByBytes.Byte3=0x0;

	pcfg->CtrlCFR2.ByBytes.Byte0=*scale-1;
	pcfg->CtrlCFR2.ByBytes.Byte3=0x92;
	pcfg->CtrlCFR2.ByBytes.Byte4=0x0;

	if((U32)*outFreq < 50000000) // <50 MHz
		return BRDerr_BAD_PARAMETER;

	if((U32)*outFreq >= 50000000) // >=50 MHz
	{
		pcfg->CtrlCFR2.ByBytes.Byte1=(*divM-1);
		R=8;
		pcfg->CtrlCFR2.ByBytes.Byte2=0x68;
//		pcfg->CtrlCFR2.ByBytes.Byte2=0x11; //for VCO=400 MHz !!!

		if((U32)*outFreq <= 100000000) // 50..100 MHz
		{
			Fsys=(*outFreq)*32/R;								// D=32
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/32*R;
			*resDiv0=1;
			*div0=5; // D=32
			*resDiv1=1;
			*div1=0; // 1

		}
		if((U32)*outFreq > 100000000 && (U32)*outFreq <= 200000000) // >100..-200 MHz
		{
			Fsys=(*outFreq)*16/R;								// D=16
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/16*R;
			*resDiv0=1;
			*div0=4;	// D=16
			*resDiv1=1;
			*div1=4; // 2
		}
		if((U32)*outFreq > 200000000 && (U32)*outFreq <= 400000000) // >200..400 MHz
		{
			Fsys=(*outFreq)*8/R;								// D=8
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/8*R;
			*resDiv0=1;
			*div0=3;	// D=8
			*resDiv1=1;
			*div1=5; // 4
		}
		if((U32)*outFreq > 400000000 && (U32)*outFreq <= 800000000) // >400..800 MHz
		{
			Fsys=(*outFreq)*4/R;								// D=4
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/4*R;
			*resDiv0=1;
			*div0=2;	// D=4
			*resDiv1=1;
			*div1=6; // 8
		}
		if((U32)*outFreq > 800000000 && (U32)*outFreq <= 1600000000) // >800..1600 MHz
		{
			Fsys=(*outFreq)*2/R;								// D=2
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )/2*R;
			*resDiv0=1;
			*div0=1;	// D=2
			*resDiv1=1;
			*div1=7; // 16
		}
		if((U32)*outFreq > 1600000000 && (U32)*outFreq<=3200000000) // >1600..3200 MHz
		{
			Fsys=(*outFreq)/R;								// D=1
			FTW = ROUND64((CompClk* pow(2.0, 48)) / Fsys); 
			*outFreq = double(CompClk*pow(2.0, 48) /FTW )*R;
			*resDiv0=1;
			*div0=0;	// D=1
			*resDiv1=1;
			*div1=7; // 16
		}
	}
	pcfg->CtrlPCR0.Byte0 = BYTE(FTW & 0xff);
	pcfg->CtrlPCR0.Byte1 = BYTE((FTW >> 8) & 0xff);
	pcfg->CtrlPCR0.Byte2 = BYTE((FTW >> 16) & 0xff);
	pcfg->CtrlPCR0.Byte3 = BYTE((FTW >> 24) & 0xff);
	pcfg->CtrlPCR0.Byte4 = BYTE((FTW >> 32) & 0xff);
	pcfg->CtrlPCR0.Byte5 = BYTE((FTW >> 40) & 0xff);
	pcfg->CtrlPCR0.Byte7 = 0;
	pcfg->CtrlPCR0.Byte6 = 0;

	return BRDerr_OK;
}

//***************************************************************************************
int	CDdsSrv::ProgAD9956(CModule* pModule,PAD9956_CFG pcfg)
{
	AD9956_CFG cfg;
	//DisableDds( pModule);
	//EnableDds( pModule);
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.UpdateDds=0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); // I/O UPDATE =0

	param.reg =	DDSnr_INST;							// Write CFR1
	param.val =	0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.reg =	DDSnr_WRITEDATA;
	param.val =	pcfg->CtrlCFR1.ByBytes.Byte3;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR1.ByBytes.Byte2;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR1.ByBytes.Byte1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR1.ByBytes.Byte0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 

//	BRDC_printf(_BRDC("Pause after writing CFR1\n"));
//	IPC_getch();

	param.reg =	DDSnr_INST;							// Write CFR2
	param.val =	1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.reg =	DDSnr_WRITEDATA;
	param.val =	pcfg->CtrlCFR2.ByBytes.Byte4;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR2.ByBytes.Byte3;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR2.ByBytes.Byte2;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR2.ByBytes.Byte1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlCFR2.ByBytes.Byte0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 

//	BRDC_printf(_BRDC("Pause after writing CFR2\n"));
//	IPC_getch();

	param.reg =	DDSnr_INST;							// Write PCR0
	param.val =	6;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.reg =	DDSnr_WRITEDATA;
	param.val =	pcfg->CtrlPCR0.Byte7;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte6;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte5;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte4;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte3;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte2;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.val =	pcfg->CtrlPCR0.Byte0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 

//	BRDC_printf(_BRDC("Pause after writing PCR0\n"));
//	IPC_getch();

	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pControl1->ByBits.UpdateDds=1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); // I/O UPDATE =1
	pControl1->ByBits.UpdateDds=0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); // I/O UPDATE =0

	ReadRegsAD9956( pModule,&cfg);
	return CompareRegsAD9956(&cfg, pcfg);
}

//***************************************************************************************
int	CDdsSrv::CompareRegsAD9956(PAD9956_CFG pcfg1,PAD9956_CFG pcfg2)
{
	int status = BRDerr_OK;
	if (pcfg1->CtrlCFR1.ByBytes.Byte0 != pcfg2->CtrlCFR1.ByBytes.Byte0) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR1.ByBytes.Byte1!=pcfg2->CtrlCFR1.ByBytes.Byte1) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR1.ByBytes.Byte2!=pcfg2->CtrlCFR1.ByBytes.Byte2) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR1.ByBytes.Byte3!=pcfg2->CtrlCFR1.ByBytes.Byte3) status = BRDerr_DDS_INVALID_WRRDREG;

	if(pcfg1->CtrlCFR2.ByBytes.Byte0!=pcfg2->CtrlCFR2.ByBytes.Byte0) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR2.ByBytes.Byte1!=pcfg2->CtrlCFR2.ByBytes.Byte1) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR2.ByBytes.Byte2!=pcfg2->CtrlCFR2.ByBytes.Byte2) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR2.ByBytes.Byte3!=pcfg2->CtrlCFR2.ByBytes.Byte3) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlCFR2.ByBytes.Byte4!=pcfg2->CtrlCFR2.ByBytes.Byte4) status = BRDerr_DDS_INVALID_WRRDREG;

	if(pcfg1->CtrlPCR0.Byte0!=pcfg2->CtrlPCR0.Byte0) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte1!=pcfg2->CtrlPCR0.Byte1) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte2!=pcfg2->CtrlPCR0.Byte2) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte3!=pcfg2->CtrlPCR0.Byte3) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte4!=pcfg2->CtrlPCR0.Byte4) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte5!=pcfg2->CtrlPCR0.Byte5) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte6!=pcfg2->CtrlPCR0.Byte6) status = BRDerr_DDS_INVALID_WRRDREG;
	if(pcfg1->CtrlPCR0.Byte7!=pcfg2->CtrlPCR0.Byte7) status = BRDerr_DDS_INVALID_WRRDREG;

	if (BRDerr_DDS_INVALID_WRRDREG == status)
	{
		BRDC_printf(_BRDC("DDS REGISTER CtrlCFR1: WR %02X%02X%02X%02X -> RD %02X%02X%02X%02X\n"),
			pcfg2->CtrlCFR1.ByBytes.Byte3, pcfg2->CtrlCFR1.ByBytes.Byte2, pcfg2->CtrlCFR1.ByBytes.Byte1, pcfg2->CtrlCFR1.ByBytes.Byte0,
			pcfg1->CtrlCFR1.ByBytes.Byte3, pcfg1->CtrlCFR1.ByBytes.Byte2, pcfg1->CtrlCFR1.ByBytes.Byte1, pcfg1->CtrlCFR1.ByBytes.Byte0);

		BRDC_printf(_BRDC("DDS REGISTER CtrlCFR2: WR %02X %02X%02X%02X%02X -> RD %02X %02X%02X%02X%02X\n"),
			pcfg2->CtrlCFR2.ByBytes.Byte4, pcfg2->CtrlCFR2.ByBytes.Byte3, pcfg2->CtrlCFR2.ByBytes.Byte2, pcfg2->CtrlCFR2.ByBytes.Byte1, pcfg2->CtrlCFR2.ByBytes.Byte0,
			pcfg1->CtrlCFR2.ByBytes.Byte4, pcfg1->CtrlCFR2.ByBytes.Byte3, pcfg1->CtrlCFR2.ByBytes.Byte2, pcfg1->CtrlCFR2.ByBytes.Byte1, pcfg1->CtrlCFR2.ByBytes.Byte0);

		BRDC_printf(_BRDC("DDS REGISTER CtrlPCR0: WR %02X%02X%02X%02X %02X%02X%02X%02X -> RD %02X%02X%02X%02X %02X%02X%02X%02X\n"),
			pcfg2->CtrlPCR0.Byte7, pcfg2->CtrlPCR0.Byte6, pcfg2->CtrlPCR0.Byte5, pcfg2->CtrlPCR0.Byte4, pcfg2->CtrlPCR0.Byte3, pcfg2->CtrlPCR0.Byte2, pcfg2->CtrlPCR0.Byte1, pcfg2->CtrlPCR0.Byte0,
			pcfg1->CtrlPCR0.Byte7, pcfg1->CtrlPCR0.Byte6, pcfg1->CtrlPCR0.Byte5, pcfg1->CtrlPCR0.Byte4, pcfg1->CtrlPCR0.Byte3, pcfg1->CtrlPCR0.Byte2, pcfg1->CtrlPCR0.Byte1, pcfg1->CtrlPCR0.Byte0);
	}

	return BRDerr_OK;

}
//***************************************************************************************
int	CDdsSrv::ReadRegsAD9956(CModule* pModule,PAD9956_CFG pcfg)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.UpdateDds=0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); // I/O UPDATE =0

	param.reg =	DDSnr_INST;							// Read CFR1
	param.val =	0x80;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.reg =	DDSnr_READDATA;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR1.ByBytes.Byte3=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR1.ByBytes.Byte2=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR1.ByBytes.Byte1=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR1.ByBytes.Byte0=param.val;

//	BRDC_printf(_BRDC("Pause after reading CFR1\n"));
//	IPC_getch();

	param.reg =	DDSnr_INST;							// Read CFR2
	param.val =	0x81;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.reg =	DDSnr_READDATA;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR2.ByBytes.Byte4=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR2.ByBytes.Byte3=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR2.ByBytes.Byte2=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR2.ByBytes.Byte1=param.val;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlCFR2.ByBytes.Byte0=param.val;

//	BRDC_printf(_BRDC("Pause after reading CFR2\n"));
//	IPC_getch();

	param.reg =	DDSnr_INST;							// Read PCR0
	param.val =	0x86;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param); 
	param.reg =	DDSnr_READDATA;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte7=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte6=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte5=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte4=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte3=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte2=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte1=param.val&0xff;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param); 
	pcfg->CtrlPCR0.Byte0=param.val&0xff;

//	BRDC_printf(_BRDC("Pause after reading PCR0\n"));
//	IPC_getch();

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::GetOutFreq(CModule* pModule, double  *OutFreq,int *ScaleCP,int *dividerM)
{
	double RefClk;
	double CompClk;
	int	divM;
	__int64 FTW;	
	AD9956_CFG cfg;

	PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG )m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	if(pControl1->ByBits.RefClkSrc==1)	
	{ 
		RefClk=pDdsSrvCfg->BaseRefClk; // internal
	}else
	{
		RefClk=pDdsSrvCfg->ExtRefClk; //external
	}
//-----------------------------------------------
	ReadRegsAD9956( pModule,&cfg);

	FTW = (__int64)cfg.CtrlPCR0.Byte0;
	FTW |= ((__int64)cfg.CtrlPCR0.Byte1 << 8);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte2 << 16);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte3 << 24);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte4 << 32);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte5 << 40);
	*ScaleCP=cfg.CtrlCFR2.ByBytes.Byte0+1;
	divM=(cfg.CtrlCFR2.ByBytes.Byte1 & 0xf) +1;
	CompClk=RefClk/divM;
	*dividerM=divM;
//-----------------------------------------------
	if(pControl1->ByBits.ClkSel==0)
	{
		*OutFreq = double(FTW * RefClk / (pow(2.0, 44)*divM)); //  <=50 MHz

	}
	if(pControl1->ByBits.ClkSel==1)
	{
		if(pControl1->ByBits.Div0 ==7)	// D=16,  >50..100 MHz
				*OutFreq = double(CompClk*pow(2.0, 46) /FTW );
		if(pControl1->ByBits.Div0 ==6)	// D=8,   >100-200 MHz
				*OutFreq = double(CompClk*pow(2.0, 47) /FTW );
		if(pControl1->ByBits.Div0 ==5)	// D=4,   >200...400 MHz
				*OutFreq = double(CompClk*pow(2.0, 48) /FTW );
		if(pControl1->ByBits.Div0 ==4)	// D=2,   >400...800 MHz
				*OutFreq = double(CompClk*pow(2.0, 49) /FTW );
		if(pControl1->ByBits.Div0 ==0)	// D=1,   >800...1600 MHz
				*OutFreq = double(CompClk*pow(2.0, 50) /FTW );

	}
	return BRDerr_OK;

}

//***************************************************************************************
int CDdsSrv::GetOutFreq_V2(CModule* pModule, double  *OutFreq,int *ScaleCP,int *dividerM)
{
	double RefClk;
	double CompClk;
	int	divM;
	__int64 FTW;	
	AD9956_CFG cfg;

	PDDSSRV_CFG pDdsSrvCfg = (PDDSSRV_CFG )m_pConfig;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	if(pControl1->ByBits.RefClkSrc==1)	
	{ 
		RefClk=pDdsSrvCfg->BaseRefClk; // internal
	}else
	{
		RefClk=pDdsSrvCfg->ExtRefClk; //external
	}
//-----------------------------------------------
	ReadRegsAD9956( pModule,&cfg);

	FTW = (__int64)cfg.CtrlPCR0.Byte0;
	FTW |= ((__int64)cfg.CtrlPCR0.Byte1 << 8);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte2 << 16);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte3 << 24);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte4 << 32);
	FTW |= ((__int64)cfg.CtrlPCR0.Byte5 << 40);
	*ScaleCP=cfg.CtrlCFR2.ByBytes.Byte0+1;
	divM=(cfg.CtrlCFR2.ByBytes.Byte1 & 0xf) +1;
	CompClk=RefClk/divM;
	*dividerM=divM;
//-----------------------------------------------
	{
		if(pControl1->ByBits.Div0 ==5)	// D=32,  >50..100 MHz
				*OutFreq = double(CompClk*pow(2.0, 46) /FTW );
		if(pControl1->ByBits.Div0 ==4)	// D=16,   >100-200 MHz
				*OutFreq = double(CompClk*pow(2.0, 47) /FTW );
		if(pControl1->ByBits.Div0 ==3)	// D=8,   >200...400 MHz
				*OutFreq = double(CompClk*pow(2.0, 48) /FTW );
		if(pControl1->ByBits.Div0 ==2)	// D=4,   >400...800 MHz
				*OutFreq = double(CompClk*pow(2.0, 49) /FTW );
		if(pControl1->ByBits.Div0 ==1)	// D=2,   >800...1600 MHz
				*OutFreq = double(CompClk*pow(2.0, 50) /FTW );
		if(pControl1->ByBits.Div0 ==0)	// D=1,   >1600...3200 MHz
				*OutFreq = double(CompClk*pow(2.0, 51) /FTW );

	}
	return BRDerr_OK;

}
//***************************************************************************************
int CDdsSrv::SetStbMode(CModule* pModule, PBRD_StbMode pStbMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	
	param.reg =	DDSnr_PULSE;
	param.val =	pStbMode->Length;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg =	DDSnr_DELAY;
	param.val =	pStbMode->Delay;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);


	param.reg =	DDSnr_PERIOD_L;
	param.val =	pStbMode->Period&0xffff;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg =	DDSnr_PERIOD_H;
	param.val =	((pStbMode->Period>>16)&0x7fff) | ((pStbMode->Polarity&1)<<15);
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	param.reg =	DDSnr_QUANTITY;
	param.val=pStbMode->Number&0x7fff;
	param.val |=(pStbMode->ContStb&1)<<15;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::GetStbMode(CModule* pModule, PBRD_StbMode pStbMode)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;

	param.reg =	DDSnr_PULSE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pStbMode->Length = param.val&0xffff;	

	param.reg =	DDSnr_DELAY;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pStbMode->Delay = param.val&0xffff;	

	param.reg =	DDSnr_PERIOD_L;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pStbMode->Period = param.val&0xffff;	

	param.reg =	DDSnr_PERIOD_H;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pStbMode->Period |=(param.val&0x7fff)<<16;	

	pStbMode->Polarity =(param.val&0x8000)>>16;	

	param.reg =	DDSnr_QUANTITY;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	pStbMode->ContStb =(param.val>>15)&1;
	pStbMode->Number = param.val&0x7fff;

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::StartStb(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = 1;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::StopStb(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg = ADM2IFnr_MODE0;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_MODE0 pMode0 = (PADM2IF_MODE0)&param.val;
	pMode0->ByBits.Start = 0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::EnableDds(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.ChipReset=0;
	pControl1->ByBits.ResDiv0=1;
	pControl1->ByBits.ResDiv1=1;
	pControl1->ByBits.UpdateDds=0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::DisableDds(CModule* pModule)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.ChipReset=1;
	pControl1->ByBits.ResDiv0=0;
	pControl1->ByBits.ResDiv1=0;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	
	return BRDerr_OK;
}
//***************************************************************************************
int CDdsSrv::SetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	// Single
	param.reg = ADM2IFnr_STMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_STMODE pStrMode = (PADM2IF_STMODE)&param.val;
	pStrMode->ByBits.SrcStart  = pStartMode->startSrc;
	pStrMode->ByBits.SrcStop   = pStartMode->trigStopSrc;
	pStrMode->ByBits.InvStart  = pStartMode->startInv;
	pStrMode->ByBits.InvStop   = pStartMode->stopInv;
	pStrMode->ByBits.TrigStart = pStartMode->trigOn;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
	return BRDerr_OK;
	
}
//=**************************************************************************************
int CDdsSrv::GetStartMode(CModule* pModule, PVOID pStMode)
{
	PBRD_StartMode pStartMode = (PBRD_StartMode)pStMode;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	// Single
	param.reg = ADM2IFnr_STMODE;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PADM2IF_STMODE pStrMode = (PADM2IF_STMODE)&param.val;
	pStartMode->startSrc	= pStrMode->ByBits.SrcStart;
	pStartMode->trigStopSrc	= pStrMode->ByBits.SrcStop;
	pStartMode->startInv	= pStrMode->ByBits.InvStart;
	pStartMode->stopInv		= pStrMode->ByBits.InvStop;
	pStartMode->trigOn		= pStrMode->ByBits.TrigStart;

	return BRDerr_OK;
}

//***************************************************************************************
int CDdsSrv::SetStartClk(CModule* pModule, U32* clksel)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.ClkSel = *clksel;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}
//***************************************************************************************
int CDdsSrv::GetStartClk(CModule* pModule, U32* clksel)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	*clksel = pControl1->ByBits.ClkSel;

	return BRDerr_OK;
}
//***************************************************************************************
int CDdsSrv::SetMuxOut(CModule* pModule, U32* output)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.MuxOut = *output;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	return BRDerr_OK;
}
//***************************************************************************************
int CDdsSrv::GetMuxOut(CModule* pModule, U32* output)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	*output = pControl1->ByBits.MuxOut;

	return BRDerr_OK;
}
//***************************************************************************************
int CDdsSrv::SetDdsPower(CModule* pModule, U32* power)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	pControl1->ByBits.DdsEn = *power;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	IPC_delay(100);

	return BRDerr_OK;
}
//***************************************************************************************
int CDdsSrv::GetDdsPower(CModule* pModule, U32* power)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_DdsTetrNum;
	param.reg =	DDSnr_CONTROL1;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PDDS_CONTROL1 pControl1 = (PDDS_CONTROL1)&param.val;
	*power = pControl1->ByBits.DdsEn;

	return BRDerr_OK;
}
//***************************************************************************************

// ***************** End of file DdsSrv.cpp ***********************
