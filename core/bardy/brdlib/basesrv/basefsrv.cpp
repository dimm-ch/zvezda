/*
 ***************** File BasefSrv.cpp ***********************
 *
 * BRD Driver for BASE of FMC-modules service
 *
 * (C) InSys by Dorokhin A. Mar 2012
 *
 *********************************************************
*/

#include "module.h"
#include "basefsrv.h"

#define	CURRFILE _BRDC("BASEFSRV")

static CMD_Info WRITESWITCH_CMD	= { BRDctrl_BASEF_SETSWITCH,	0, 0, 0, NULL};
static CMD_Info READSWITCH_CMD	= { BRDctrl_BASEF_GETSWITCH,	1, 0, 0, NULL};
static CMD_Info ONSWITCH_CMD	= { BRDctrl_BASEF_SWITCHONOFF,	0, 0, 0, NULL};
static CMD_Info READSTAT_CMD	= { BRDctrl_BASEF_GETONOFF,		1, 0, 0, NULL};
static CMD_Info SETCLKMODE_CMD	= { BRDctrl_BASEF_SETCLKMODE,	0, 0, 0, NULL};
static CMD_Info GETCLKMODE_CMD	= { BRDctrl_BASEF_GETCLKMODE,	1, 0, 0, NULL};
static CMD_Info SWITCHDEV_CMD	= { BRDctrl_BASEF_SWITCHDEV,	0, 0, 0, NULL};

//***************************************************************************************
CBasefSrv::CBasefSrv(int idx, int srv_num, CModule* pModule, PBASEFSRV_CFG pCfg) :
	CService(idx, _BRDC("BASEFMC"), srv_num, pModule, pCfg, sizeof(BASEFSRV_CFG))
{
	m_attribute = 
			BRDserv_ATTR_DIRECTION_IN |
			BRDserv_ATTR_DIRECTION_OUT;	//|
			//BRDserv_ATTR_EXCLUSIVE_ONLY;

	Init(&WRITESWITCH_CMD, (CmdEntry)&CBasefSrv::CtrlWriteSwitch);
	Init(&READSWITCH_CMD, (CmdEntry)&CBasefSrv::CtrlReadSwitch);
	Init(&ONSWITCH_CMD, (CmdEntry)&CBasefSrv::CtrlOnOffSwitch);
	Init(&READSTAT_CMD, (CmdEntry)&CBasefSrv::CtrlReadStatus);
	Init(&SWITCHDEV_CMD, (CmdEntry)&CBasefSrv::CtrlSwitchDev);

	Init(&SETCLKMODE_CMD, (CmdEntry)&CBasefSrv::CtrlClkMode);
	Init(&GETCLKMODE_CMD, (CmdEntry)&CBasefSrv::CtrlClkMode);
}

//***************************************************************************************
int CBasefSrv::CtrlIsAvailable(
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

	m_isAvailable = 0;

	if(m_MainTetrNum != -1)
	{
		DEVS_CMD_Reg param;
		param.idxMain = m_index;
		param.tetr = m_MainTetrNum;
		param.reg = ADM2IFnr_IDMOD;
		pModule->RegCtrl(DEVScmd_REGREADIND, &param);
		if(param.val >= 17)
			m_isAvailable = 1;
	}
	pServAvailable->isAvailable = m_isAvailable;
	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlCapture(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
							)
{
	CModule* pModule = (CModule*)pDev;
	if(m_isAvailable)
	{
		PBASEFSRV_CFG pSrvCfg = (PBASEFSRV_CFG)m_pConfig;
		if(!pSrvCfg->isAlreadyInit)
		{
			pSrvCfg->isAlreadyInit = 1;
			pSrvCfg->SwitchDevId = BASEFsd_CLOCK;
			pSrvCfg->dGenFxtal = 0.0;
			if(pSrvCfg->Gen0Type)
			{	
				DEVS_CMD_Reg param;
				param.idxMain = m_index;
				param.tetr = m_MainTetrNum;
				param.reg = MAINnr_CMPMUX;
				pModule->RegCtrl(DEVScmd_REGREADIND, &param);
				PMAIN_MUX pMux = (PMAIN_MUX)&param.val;
				ULONG prev = pMux->ByBits.GenEn;
				pMux->ByBits.GenEn = 1;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

				writeSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, 135, 0x80); // Reset
				//SpdWrite( pModule, SPDdev_GEN, 135, 0x80 );		
#if defined(__IPC_WIN__) || defined(__IPC_LINUX__)
				IPC_delay(100);
#else
				Sleep(100);
#endif
				ULONG	regAdr, regData[20];

				// Считать регистры Si570/Si571
				for( regAdr=7; regAdr<=12; regAdr++ )
					readSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, regAdr, &regData[regAdr]);
					//SpdRead( pModule, SPDdev_GEN, regAdr, &regData[regAdr] );

				// Рассчитать частоту кварца
				SI571_CalcFxtal( &(pSrvCfg->dGenFxtal), (double)(pSrvCfg->RefGen0), (U32*)regData );
				//printf( "After reset Si571 regs 7-12: %x, %x, %x, %x, %x, %x\n", regData[7], regData[8], regData[9], regData[10], regData[11], regData[12] );
				//printf( ">> XTAL = %f kHz\n", pSrvCfg->dGenFxtal/1000.0 );
				//printf( ">> GREF = %f kHz\n", ((double)(pSrvCfg->RefGen0))/1000.0 );

				pMux->ByBits.GenEn = prev;
				pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
			}
		}
	}
	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlSwitchDev(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	PBASEFSRV_CFG pBasefCfg = (PBASEFSRV_CFG)m_pConfig;
	U32 Dev	= *(U32*)args;
	if(Dev != BASEFsd_CLOCK && Dev != BASEFsd_SYNC)
		return BRDerr_BAD_PARAMETER;
	pBasefCfg->SwitchDevId = Dev;
	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlOnOffSwitch(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Switch pIoSwitch = (PBRD_Switch)args;
	PBASEFSRV_CFG pBasefCfg = (PBASEFSRV_CFG)m_pConfig;
	
	ULONG reg = (pIoSwitch->out << 3) + 0xC0; // TX Configuration register
	ULONG on_off = pIoSwitch->val << 5;
	writeSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, reg, on_off);

	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlReadStatus(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Switch pIoSwitch = (PBRD_Switch)args;
	PBASEFSRV_CFG pBasefCfg = (PBASEFSRV_CFG)m_pConfig;
	
	ULONG reg = (pIoSwitch->out << 3) + 0xC0; // TX Configuration register
	ULONG on_off = 0;
	readSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, reg, &on_off);
	pIoSwitch->val = on_off >> 5;

	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlWriteSwitch(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Switch pIoSwitch = (PBRD_Switch)args;
	PBASEFSRV_CFG pBasefCfg = (PBASEFSRV_CFG)m_pConfig;
	
	ULONG val = (pIoSwitch->val << 4) + pIoSwitch->out;
	writeSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, 0x40, val); // XPT Configuration register
	ULONG olc1_reg = (pIoSwitch->out << 3) + 0xC1; // TX Output Level Control 1 register
	writeSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, olc1_reg, 0xE6);
	ULONG olc0_reg = (pIoSwitch->out << 3) + 0xC2; // TX Output Level Control 0 register
	writeSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, olc0_reg, 0x04);
	writeSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, 0x41, 1); // XPT Update register

	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlReadSwitch(
							void		*pDev,		// InfoSM or InfoBM
							void		*pServData,	// Specific Service Data
							ULONG		cmd,		// Command Code (from BRD_ctrl())
							void		*args 		// Command Arguments (from BRD_ctrl())
						   )
{
	CModule* pModule = (CModule*)pDev;
	PBRD_Switch pIoSwitch = (PBRD_Switch)args;
	PBASEFSRV_CFG pBasefCfg = (PBASEFSRV_CFG)m_pConfig;

	ULONG reg = pIoSwitch->out + 0x50; // XPT Status register
	ULONG val = 0;
	readSpdDev(pModule, pBasefCfg->SwitchDevId, pBasefCfg->AdrSwitch, 0, reg, &val);
	pIoSwitch->val = val;

	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::writeSpdDev(CModule* pModule, ULONG dev, ULONG num, ULONG synchr, ULONG reg, ULONG val)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;

	// ожидаем готовности тетрады
	param.reg = ADM2IFnr_STATUS;
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	do	{
		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	} while(!pStatus->ByBits.CmdRdy);

	// выбираем устройство
	param.reg = MAIN17nr_SPDDEVICE; // 0x203
	param.val = dev;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// записываем адрес
	param.reg = MAIN17nr_SPDADDR; // 0x205
	param.val = reg;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// записываем данные
	param.reg = MAIN17nr_SPDDATAL; // 0x206  
	param.val = val;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// посылаем команду записи
	param.val = (synchr << 12) | (num << 4) | 0x2; // write
	param.reg = MAIN17nr_SPDCTRL; // 0x204
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// ожидаем готовности тетрады
	param.reg = ADM2IFnr_STATUS;
	pStatus = (PADM2IF_STATUS)&param.val;
	do	{
		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	} while(!pStatus->ByBits.CmdRdy);

	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::readSpdDev(CModule* pModule, ULONG dev, ULONG num, ULONG synchr, ULONG reg, ULONG* pVal)
{
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;

	// ожидаем готовности тетрады
	param.reg = ADM2IFnr_STATUS;
	PADM2IF_STATUS pStatus = (PADM2IF_STATUS)&param.val;
	do	{
		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	} while(!pStatus->ByBits.CmdRdy);

	// выбираем устройство
	param.reg = MAIN17nr_SPDDEVICE; // 0x203
	param.val = dev;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// записываем адрес
	param.reg = MAIN17nr_SPDADDR; // 0x205
	param.val = reg;
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// посылаем команду чтения
	param.val = (synchr << 12) | (num << 4) | 0x1; // read
	param.reg = MAIN17nr_SPDCTRL; // 0x204
	pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);

	// ожидаем готовности тетрады
	param.reg = ADM2IFnr_STATUS;
	pStatus = (PADM2IF_STATUS)&param.val;
	do	{
		pModule->RegCtrl(DEVScmd_REGREADDIR, &param);
	} while(!pStatus->ByBits.CmdRdy);

	// считываем данные
	param.reg = MAIN17nr_SPDDATAL; // 0x206;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	*pVal = param.val;

	return BRDerr_OK;
}

//***************************************************************************************
int CBasefSrv::CtrlClkMode(
					void		*pDev,		// InfoSM or InfoBM
					void		*pServData,	// Specific Service Data
					ULONG		cmd,		// Command Code (from BRD_ctrl())
					void		*args 		// Command Arguments (from BRD_ctrl())
				   )
{
	CModule* pModule = (CModule*)pDev;
	PBASEFSRV_CFG pBasefCfg = (PBASEFSRV_CFG)m_pConfig;
	PBRD_BasefClkMode clk_mode = (PBRD_BasefClkMode)args;
	DEVS_CMD_Reg param;
	param.idxMain = m_index;
	param.tetr = m_MainTetrNum;

	param.reg = MAINnr_CMPMUX;
	pModule->RegCtrl(DEVScmd_REGREADIND, &param);
	PMAIN_MUX pMux = (PMAIN_MUX)&param.val;

	if (BRDctrl_BASEF_SETCLKMODE == cmd)
	{
		pMux->ByBits.GenEn = clk_mode->genEn;
		pMux->ByBits.CmpEn = clk_mode->clkInCmpEn;
		pModule->RegCtrl(DEVScmd_REGWRITEIND, &param);
		if (!clk_mode->clkValue)
		{	// если задано НУЛЕВОЕ значение частоты
			clk_mode->clkValue = (REAL64)pBasefCfg->RefGen0; // возвращаем частоту, записанную в ППЗУ
		}
		else
		{	// если задано НЕНУЛЕВОЕ значение частоты
			if (pBasefCfg->Gen0Type)
			{ // если установлен программируемый внутренний генератор
				Si571SetClkVal(pModule, &clk_mode->clkValue); // программируем внутренний генератор
			}
			else
			{ // если установлен НЕпрограммируемый внутренний генератор
				if (clk_mode->clkValue != (REAL64)pBasefCfg->RefGen0)
				{
					clk_mode->clkValue = (REAL64)pBasefCfg->RefGen0; // возвращаем частоту, записанную в ППЗУ
					return BRDerr_PARAMETER_CHANGED;
				}
			}
		}
	}
	else
	{
		clk_mode->genEn = pMux->ByBits.GenEn;
		clk_mode->clkInCmpEn = pMux->ByBits.CmpEn;
		if (pBasefCfg->Gen0Type)
			Si571GetClkVal(pModule, &clk_mode->clkValue);
		else
			clk_mode->clkValue = pBasefCfg->RefGen0;
	}
	return BRDerr_OK;
}

//***************************************************************************************
int	CBasefSrv::Si571SetClkVal( CModule* pModule, double *pRate )    
{
	PBASEFSRV_CFG pSrvCfg = (PBASEFSRV_CFG)m_pConfig;
	ULONG			regData[20];
	int			regAdr;
	int			status = BRDerr_OK;

	// Скорректировать частоту, если необходимо
	if( *pRate > (double)pSrvCfg->RefMaxGen0 )
	{
		*pRate = (double)pSrvCfg->RefMaxGen0;
		status = BRDerr_WARN;
	}

    SI571_SetRate( pRate, pSrvCfg->dGenFxtal, (U32*)regData );
	//if(ret < 0)
	//	printf( "Error SI571_SetRate\n");

	// Запрограммировать генератор
	writeSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, 137, 0x10); // Freeze DCO
	for( regAdr=7; regAdr<=18; regAdr++ )
		writeSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, regAdr, regData[regAdr]);
	writeSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, 137, 0x0); // Unfreeze DCO
	writeSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, 135, 0x40 ); // Assert the NewFreq bit
	//printf( "Write Si571 regs 7-12: %x, %x, %x, %x, %x, %x\n", regData[7], regData[8], regData[9], regData[10], regData[11], regData[12] );
	//printf( ">> XTAL = %f kHz\n", pSrvCfg->dGenFxtal/1000.0 );
	//printf( ">> Rate = %f kHz\n", *pRate/1000.0 );

	return status;	
}

//***************************************************************************************
int	CBasefSrv::Si571GetClkVal( CModule* pModule, double *pRate )
{
	PBASEFSRV_CFG pSrvCfg = (PBASEFSRV_CFG)m_pConfig;
	ULONG			regData[20];
    //ULONG		clkSrc;
	int			regAdr;

	*pRate = 0.0;

	// Считать регистры Si570/Si571
	for( regAdr=7; regAdr<=12; regAdr++ )
		readSpdDev(pModule, GENERATOR_DEVID, pSrvCfg->AdrGen0, 0, regAdr, &regData[regAdr]);

	SI571_GetRate( pRate, pSrvCfg->dGenFxtal, (U32*)regData );
	//printf( "Read Si571 regs 7-12: %x, %x, %x, %x, %x, %x\n", regData[7], regData[8], regData[9], regData[10], regData[11], regData[12] );

	return BRDerr_OK;	
}

// ***************** End of file BasefSrv.cpp ***********************
