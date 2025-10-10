// clang-format off
//********************************************************
// exam_edac.cpp
//
// Программа EXAM_EDAC для управления ЦАПами на субмодулях
//
// (C) ИнСис, Эккоре, Март 2014
//
//=********************************************************

#include	<stdio.h>
#include	<ctype.h>
#include	<math.h>
#include	<stdexcept>

#include	"../total.h"
#include	"gipcy.h"

#include "../../work2/common/dev_util.h"
#include "total.h"

//
// Globals
//

//BRDCHAR		g_iniFileName[MAX_PATH] = _BRDC("exam_edac.ini");
//BRDCHAR		g_sFullName[MAX_PATH];

//int			g_lid = -1;
//S32			g_nDevNum = 0;
//BRD_Handle	g_hDev[MAX_DEVICE];
//S32			g_nDacNum = 0;
//TDacParam	g_aDac[MAX_DAC];
//BRD_Handle	g_idx[MAX_DAC];			// Рассортированные номера ЦАПов для MASTER/SLAVE
/*
double		g_dSamplingRate;
double		g_dSignalFreq;
U32			g_dSignalType;
FILE		*g_pFileDataBin;
*/
//
// Params from ini-file
//
/*
BRDCHAR		g_sServiceName[256];	// без номера службы
BRDCHAR		g_sPldFileName[MAX_PATH];
int			g_nWorkMode;
int			g_nIsAlwaysWriteSdram;
SBIG		g_nSamplesPerChannel;
int			g_nIsSystemMemory;
int			g_nSdramWriteBufSize;
int			g_nIsAlwaysLoadPld;
int			g_nMasterSlave;
int			g_nDmaBufFactor;
int			g_nIsDebugMarker;
int			g_nCycle;
int			g_nQuickQuit;

int			g_nIsNew;
S32			g_nMsTimeout = 5000;
*/
//------------------------------------------------------------------------


//=******************* DisplayErrorDac ***********************
//=********************************************************
S32 DisplayErrorDac( S32 status, const char* func_name, const BRDCHAR* cmd_str )
{
	U32			real_status = BRD_errext(status);
	BRDCHAR		msg[255];

	switch(real_status)
	{
		case BRDerr_OK:
			BRDC_sprintf(msg, _BRDC("%hs - %s: BRDerr_OK\n"), func_name, cmd_str);
			break;
		case BRDerr_BAD_MODE:
			BRDC_sprintf(msg, _BRDC("%hs - %s: BRDerr_BAD_MODE\n"), func_name, cmd_str);
			break;
		case BRDerr_INSUFFICIENT_SERVICES:
			BRDC_sprintf(msg, _BRDC("%hs - %s: BRDerr_INSUFFICIENT_SERVICES\n"), func_name, cmd_str);
			break;
		case BRDerr_BAD_PARAMETER:
			BRDC_sprintf(msg, _BRDC("%hs - %s: BRDerr_BAD_PARAMETER\n"), func_name, cmd_str);
			break;
		case BRDerr_BUFFER_TOO_SMALL:
			BRDC_sprintf(msg, _BRDC("%hs - %s: BRDerr_BUFFER_TOO_SMALL\n"), func_name, cmd_str);
			break;
		case BRDerr_NOT_ACTION:
			BRDC_sprintf(msg, _BRDC("%hs - %s: BRDerr_NOT_ACTION\n"), func_name, cmd_str);
			break;
		default:
			BRDC_sprintf(msg, _BRDC("%hs - %s: Unknown error, status = %8X\n"), func_name, cmd_str, real_status);
			break;
	}
	BRDC_printf( msg );

	return 0;
}

//=******************** CaptureAllDac *********************
//=********************************************************
S32 CaptureAllDac(int lid, U32 modeCapture )
{
	S32			err;
	S32			ii;
	U32			iDev = 0;
	BRD_Info	rInfo;

	ReleaseAllDac(lid);
	// Открыть устройство, если не открыто
	BRD_Handle hDev = DevicesLid[lid].device.handle();		
	if( hDev<=0)
	{
		hDev = DevicesLid[lid].device.reopen(lid);
		if(! DevicesLid[lid].device.isOpen()) {
			printf( "<ERR> CaptureAllDac: device d'nt opened\n");		
			return -1;
		}
	}
	ParamsDACs& p = DevicesLid[lid].paramsDac;

	// Вывести на экран информацию об устройстве
	rInfo.size = sizeof(rInfo);
	BRD_getInfo(lid, &rInfo );
	if(rInfo.busType == BRDbus_ETHERNET)
		BRDC_printf(_BRDC("<SRV> Device %s, LID = %d\n  DevID = 0x%x, RevID = 0x%x, IP %u.%u.%u.%u, Port %u, PID = %d.\n"),
		rInfo.name,
		lid,
		rInfo.boardType >> 16,
		rInfo.boardType & 0xff,
		(UCHAR)rInfo.bus,
		(UCHAR)(rInfo.bus >> 8),
		(UCHAR)(rInfo.bus >> 16),
		(UCHAR)(rInfo.bus >> 24),
		rInfo.dev,
		rInfo.pid );
	else
		BRDC_printf(_BRDC("<SRV> Device %s, LID = %d\n    DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, Slot = %d, PID = %d.\n"),
		rInfo.name,
		lid,
		rInfo.boardType >> 16,
		rInfo.boardType & 0xff,
		rInfo.bus,
		rInfo.dev,
		rInfo.slot,
		rInfo.pid );
		//
		// Захватить все подходящие службы на данном устройстве
		//
		U32	mode = modeCapture; //BRDcapt_EXCLUSIVE;
		U32	nItemReal, iSrv;
		BRD_ServList srvList[MAX_SERVICE_ON_DEVICE];
		S32	len = (S32)BRDC_strlen( p.g_sServiceName );

		err = BRD_serviceList( hDev, 0, srvList, MAX_SERVICE_ON_DEVICE, &nItemReal);
		for( ii = 0; ii < (S32)nItemReal; ii++)
			BRDC_printf( _BRDC("        Srv %d:  %15s, Attr = %X.\n"),ii, srvList[ii].name, srvList[ii].attr);

		
		for( iSrv = 0; iSrv < nItemReal; iSrv++ )
		{
			if( BRDC_strnicmp(  srvList[iSrv].name, p.g_sServiceName, len) )
				continue;
			if( (srvList[iSrv].name[len] < '0') ||
				(srvList[iSrv].name[len] > '9') ||
				(srvList[iSrv].name[len+1] != '\0') )
				continue;

            BRDC_printf( _BRDC( "<SRV> Try capture %s serv. in %s mode ...\n"), 
						p.g_sServiceName, getStrCaptureModeService(mode).c_str() );

			DacDevice dac;
			dac.servDac.setMode(mode);
			dac.servDac.capture(hDev,  srvList[iSrv].name, 10000);
			if(dac.servDac.isCaptured())
			{
				printf(" - %s\n", dac.servDac.strMode().c_str());
				dac.paramDac.handle = dac.servDac.handle();
				dac.paramDac.dSignalFreq = p.g_dSignalFreq;
				dac.paramDac.signalType  = p.g_dSignalType;
				BRDC_sprintf( dac.paramDac.sSection, _BRDC("device%d_%s"), iDev++, srvList[iSrv].name);
				DevicesLid[lid].dac.push_back(dac);				
				BRDC_printf( _BRDC( "<SRV> Capture %s serv. in %s mode (section:%s)\n"), srvList[iSrv].name, 
							getStrCaptureModeService(mode).c_str(),
							dac.paramDac.sSection );
			}
			else
				printf("<ERR> capture is fail: %s \n", dac.servDac.error().c_str());
			

		}


    BRDC_printf( _BRDC( "\n" ));

	return 0;
}

//=******************** ReleaseAllDac *********************
//=********************************************************
S32 ReleaseAllDac(int lid)
{
	S32			err = 1;
	if(lid < 0 || lid >= MAX_LID_DEVICES)
		return err;
		
	for( auto& dac : DevicesLid[lid].dac ) {
		if(DevicesLid[lid].device.isOpen()) {
			dac.servDac.release();
			dac.paramDac.handle = 0;
		}
	}
	DevicesLid[lid].dac.clear();

	return 0;
}

//=*********************** SetAllDac **********************
//=********************************************************
S32 SetAllDac( int lid )
{
	S32			err;	
	BRD_DacCfg	rDacCfg;
	ParamsDACs& p = DevicesLid[lid].paramsDac;

	for(auto& dev : DevicesLid[lid].dac )
	{
		if(!dev.servDac.isCaptured()) {
			printf("<ERR> SetAllDac: service not captured!");
			return -1;
		}
		BRD_Handle hdlSrv = dev.getService();
		//
		// Определить размер FIFO в байтах, количество каналов
		//
		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETCFG, &rDacCfg );
		if(err < 0) {
			printf("<ERR> SetAllDac::BRD_ctrl(BRDctrl_DAC_GETCFG)");
			return err;
		}
		BRDC_printf(_BRDC("<SRV> DAC Config: FIFO size = %d kBytes\n"), rDacCfg.FifoSize / 1024);
		BRDC_printf(_BRDC("<SRV> DAC Config: chanMaxNum = %d \n"), rDacCfg.MaxChan);
		dev.paramDac.nFifoSizeb = rDacCfg.FifoSize;
		dev.paramDac.chanMaxNum = rDacCfg.MaxChan;
		if( dev.paramDac.chanMaxNum > MAX_CHAN )
			dev.paramDac.chanMaxNum = MAX_CHAN;

		//
		// Задать параметры работы ЦАП с помощью ini-файла
		//
		BRD_IniFile rIniFile;
		
		lstrcpy( rIniFile.fileName, p.g_sFullName );
		lstrcpy( rIniFile.sectionName, dev.paramDac.sSection );
		BRDC_printf(_BRDC("<SRV> DAC_READINIFILE: \n - name: %s\n - section : %s \n"), rIniFile.fileName,
				rIniFile.sectionName);
		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_READINIFILE, &rIniFile );
		
		if (BRD_errext(err) != BRDerr_OK) {
			printf("<ERR> BRDctrl_DAC_READINIFILE: err=0x%X, handle=0%X, rIniFile.fileName=%s, rIniFile.sectionName=%s \n", 
				err, hdlSrv, rIniFile.fileName, rIniFile.sectionName);
			throw std::invalid_argument("Side-Driver rejection");
		}

		//
		// получить формат данных и размер буфера
		//
		U32	format = 0;

		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETFORMAT, &format);
		dev.paramDac.sampleSizeb = format ? format : sizeof(short);

		//
		// Получить источник и значение тактовой частоты можно отдельной функцией
		//
		BRD_SyncMode	rSyncMode;
		U32				nInterpFactor;

		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETINTERP, &nInterpFactor );
		if( !BRD_errcmp(err, BRDerr_OK) )
		{
			nInterpFactor = 1;
			DisplayErrorDac( err, __FUNCTION__, _BRDC("BRDctrl_DAC_GETINTERP") );
		}
		BRDC_printf(_BRDC("BRDctrl_DAC_GETINTERP: Kint = %d\n"), nInterpFactor );

		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETSYNCMODE, &rSyncMode );
		if( BRD_errcmp(err, BRDerr_OK) )
		{
			BRDC_printf(_BRDC("BRDctrl_DAC_GETSYNCMODE: source = %d, value = %.2f MHz, "), rSyncMode.clkSrc, rSyncMode.clkValue/1000000);
			if( rSyncMode.rate < 1000000 )
				BRDC_printf(_BRDC("sample rate = %.2f kHz\n"), rSyncMode.rate/1000);
			else
				BRDC_printf(_BRDC("sample rate = %.2f MHz\n"), rSyncMode.rate/1000000);
			dev.paramDac.dSamplingRate = rSyncMode.rate;
		}
		else
			DisplayErrorDac( err, __FUNCTION__, _BRDC("BRDctrl_DAC_GETSYNCMODE") );

		//
		// получить параметры стартовой синхронизации
		//
		// команда BRDctrl_ADC_GETSTARTMODE может получать 2 разные структуры:
		// 1) BRD_StartMode размером 24 байта или
		// 2) BRD_DacStartMode размером 44 байта
		// для определения какую из них использует данная служба
		// применяем трюк с массивом )))
		//
		U08 aStartStruct[sizeof(BRD_DacStartMode)]; // наибольшая из структур имеет размер 44 байт
		memset( aStartStruct, 0x5A, sizeof(BRD_DacStartMode) );

		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETSTARTMODE, &aStartStruct );
		if( BRD_errcmp(err, BRDerr_OK) )
		{
			if( aStartStruct[1+sizeof(BRD_StartMode)] == 0x5A )
			{
				BRD_StartMode *pStart = (BRD_StartMode*)aStartStruct;
				BRDC_printf( _BRDC("BRDctrl_DAC_GETSTARTMODE: start source = %d\n"), pStart->startSrc );
			}
			else
			{
				BRD_DacStartMode *pStart = (BRD_DacStartMode*)aStartStruct;;
				BRDC_printf( _BRDC("BRDctrl_DAC_GETSTARTMODE: start source base = %d, subunit = %d\n"), pStart->stndStart.startSrc, pStart->src );
			}
		}
		else
			DisplayErrorDac(err, __FUNCTION__, _BRDC("BRDctrl_DAC_GETSTARTMODE"));

		//
		// Скорректировать режим РЕСТАРТ для WorkMode=3 и 4
		//
		if( (p.g_nWorkMode==3) || (p.g_nWorkMode==4) )
		{
			memset( aStartStruct, 0x5A, sizeof(BRD_DacStartMode) );
			err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETSTARTMODE, &aStartStruct );
			if( BRD_errcmp(err, BRDerr_OK) )
			{
				if( aStartStruct[1+sizeof(BRD_StartMode)] == 0x5A ) {
					
					BRD_StartMode *pStart = (BRD_StartMode*)aStartStruct;
					pStart->reStartMode = 1;
				}
				else {
					
					BRD_DacStartMode *pStart = (BRD_DacStartMode*)aStartStruct;;
					pStart->stndStart.reStartMode = 1;
				}
				err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_SETSTARTMODE, &aStartStruct );
				
			}
		}	
		// Установить значения компараторов
		//
		BRD_CmpSC rCmpSc;
		rCmpSc.src= 0;
		rCmpSc.thr[0] = dev.paramDac.aThdac[0];
		rCmpSc.thr[1] = dev.paramDac.aThdac[1];

		err = BRD_ctrl( hdlSrv, 0, BRDctrl_CMPSC_SET, &rCmpSc );

		//
		// Получить маску выбраных каналов. Определить количество выбраных каналов.
		//
		U32 chanMask = 0;

		err = BRD_ctrl( hdlSrv, 0, BRDctrl_DAC_GETCHANMASK, &chanMask );
		if(BRD_errcmp(err, BRDerr_OK))
			BRDC_printf(_BRDC("BRDctrl_DAC_GETCHANMASK: chan_mask = %0X\n"), chanMask);
		else
			DisplayErrorDac(err, __FUNCTION__, _BRDC("BRDctrl_DAC_GETCHANMASK"));


		S32			jj;
		S32			chanNum  = 0;

		for( jj = 0; jj < dev.paramDac.chanMaxNum; jj++ )
			chanNum += (chanMask >> jj) & 0x1;

		dev.paramDac.chanMask = chanMask;
		dev.paramDac.chanNum  = chanNum;

		//
		// Управление коммутатором
		//
		SetSwitch(DevicesLid[lid].device.handle(), dev.paramDac);
	}

	return 0;
}

//=******************** SetSwitch *****************************
//=************************************************************
S32 SetSwitch(BRD_Handle handle, TDacParam& param)
{
	//BRD_Handle	handle   = DevicesLid[lid].device.handle; //g_hDev[idxDac];
	S32			switchIn = param.switchIn; //g_aDac[idxDac].switchIn;
	S32			switchOut= param.switchOut; //g_aDac[idxDac].switchOut;

	BRDC_printf(_BRDC( "SetSwitch( %d, %d )\n"), switchIn, switchOut );
	if( (switchIn<0) || (switchOut<0) )
		return 1;

	//
	// Управление коммутатором на базовом модуле
	//
	S32 err = 0;
	U32 mode = BRDcapt_SHARED;
	BRD_Handle hSrv = BRD_capture( handle, 0, &mode, _BRDC("BASEFMC0"), 10000 );

	if(hSrv <= 0)
	{
		BRDC_printf(_BRDC("WARNING: BASEFMC0 NOT capture (%X)\n"), hSrv);
		return -1;
	}
	if(mode == BRDcapt_EXCLUSIVE) BRDC_printf(_BRDC("BASEFMC Capture mode: EXCLUSIVE (%X)\n"), hSrv);
	if(mode == BRDcapt_SHARED) BRDC_printf(_BRDC("BASEFMC Capture mode: SHARED (%X)\n"), hSrv);
	if(mode == BRDcapt_SPY)	BRDC_printf(_BRDC("BASEFMC Capture mode: SPY (%X)\n"), hSrv);

	BRD_Switch sw;
	sw.out = switchOut; // на выход SwitchOut (FPGA_CLK0)
	sw.val = switchIn;  // подаем вход SwitchIn (FMC_CLK0)
	err = BRD_ctrl(hSrv, 0, BRDctrl_BASEF_SETSWITCH, &sw);

	sw.out = switchOut; // выход SwitchOut (FPGA_CLK0)
	sw.val = 1;         // включаем (1 - on)
	err = BRD_ctrl(hSrv, 0, BRDctrl_BASEF_SWITCHONOFF, &sw);

	if(BRD_errcmp(err, BRDerr_OK))
		BRDC_printf(_BRDC( "SetSwitch: %d --> %d\n"), switchIn, switchOut );

	err = BRD_release(hSrv, 0);

	return 0;
}

//******************* SetMasterSlave **********************
//*********************************************************
S32	SetMasterSlave(int lid)
{
	S32				err;
	int				ii, jj;
	U32				master[MAX_DAC];

	ParamsDACs& p = DevicesLid[lid].paramsDac;
	if(DevicesLid[lid].dac.size() == 0) {
		printf("<ERR> SetMasterSlave: service d'nt captured ");
		return -1;
	}
	//
	// Проверить номер ЦАП, выбранного в качестве МАСТЕРА
	//
	int g_nDacNum = DevicesLid[lid].dac.size();
	if( p.g_nMasterSlave >= g_nDacNum )
	if( g_nDacNum > 1 )
	{
		BRDC_printf(_BRDC("WARNING: Wrong Master Dac No (%d). Must be less than %d.\n"), p.g_nMasterSlave, g_nDacNum );
		p.g_nMasterSlave = -1;
	}

	//
	// Рассортировать дескрипторы ЦАПов так, чтобы первым шел МАСТЕР
	//
	if( (p.g_nMasterSlave>=0) && (g_nDacNum>1) )
	{
		//
		// Режим MASTER/SLAVE
		//
		p.g_idx[0] = p.g_nMasterSlave;
		jj = 1;

		for( ii=0; ii<g_nDacNum; ii++ )
			if( ii != p.g_nMasterSlave )
				p.g_idx[jj++] = ii;

		for( ii=0; ii<g_nDacNum; ii++ )
			master[ii] = 0;
		master[0] = 2;
	}
	else
	{
		//
		// Режим SINGLE
		//
		for( ii=0; ii<g_nDacNum; ii++ )
			p.g_idx[ii] = ii;
		for( ii=0; ii<g_nDacNum; ii++ )
			master[ii] = 1;
	}

	//
	// Установить режим МАСТЕР/СЛЕЙВ или СИНГЛ для всех ЦАПов
	//
	ii =0;
	
	for(auto& param : DevicesLid[lid].dac) // ii=0; ii<g_nDacNum; ii++ )
	{
		err = BRD_ctrl( param.paramDac.handle, 0, BRDctrl_DAC_SETMASTER, &master[ii]);
		if( 0 <= err )
            BRDC_printf( _BRDC("BRDctrl_DAC_SETMASTER:  DAC%d  master = %d\n"), ii, master[ii] );
		else
			DisplayErrorDac( err, __FUNCTION__, _BRDC("BRDctrl_DAC_SETMASTER") );
		ii++;
	}

	return 0;
}



//********************** CalcSignal ***********************
//*********************************************************
S32 CalcSignal(int lid, void *pvBuf, SBIG nSamples, TDacParam& dac, int cnt )
{
	S32				err;
	int				ii;
	double			twiddle;

	ParamsDACs& p = DevicesLid[lid].paramsDac;
	//
	// Заполнить сигнал из файла
	//
	if( p.g_pFileDataBin )
		return FillSignalFromFile(lid, pvBuf, nSamples, dac, cnt );

	//
	// Если первый буфер, запомнить начальную фазу
	//
	if( cnt==0 )
		for( ii=0; ii<MAX_CHAN; ii++ )
			dac.aPhaseKee[ii] = dac.aPhase[ii];


	twiddle = PI2 * dac.dSignalFreq / dac.dSamplingRate;

	err = CalcSignalToBuf(lid, pvBuf, nSamples,
							dac.signalType,
							dac.sampleSizeb,
							dac.chanMask,
							dac.chanMaxNum,
							twiddle,
							dac.aAmpl,
							dac.aPhaseKee
							);

	return err;
}

//******************** CalcSignalToBuf ********************
//*********************************************************
S32 CalcSignalToBuf(int lid, void *pvBuf, SBIG nSamples, S32 signalType,
					 S32 sampleWidth, S32 chanMask, S32 chanMaxNum,
					 double twiddle, double *aAmpl, double *aPhase )
{
	int			ii;
	S32			chanNum;
	S32			aChanNo[64];

	//
	// Определить количество и номера выбранных каналов
	//
	chanNum = 0;
	for( ii=0; ii<chanMaxNum; ii++ )
		if( chanMask & (1<<ii) )
			aChanNo[chanNum++] = ii;

	//
	// Сформировать сигнал в каждом выбранном канале
	//
	for( ii=0; ii<chanNum; ii++ )
		CalcSignalToChan(lid, pvBuf, nSamples, signalType, sampleWidth, ii, chanNum,
		twiddle, aAmpl[aChanNo[ii]], &(aPhase[aChanNo[ii]]) );

	return 0;
}


template<typename T>
void fillBufferForDac(int lid, void *pvBuf, SBIG nSamples, S32 signalType, S32 chanIdx, S32 chanNum, 
		double& phase, double twiddle, double ampl, const T valc)
{
	T *pBuf = (T*)pvBuf;
	ParamsDACs& p = DevicesLid[lid].paramsDac;

	pBuf += chanIdx;
	for(SBIG ii=0; ii<nSamples; ii++ )
	{
		if( signalType == 2 ) // меандр
			*pBuf = (T)(( *pBuf < 0.0 ) ? -ampl : +ampl);
		else if(signalType == 3) // const ampl
			*pBuf = (T)(ampl);
		else // синус
			*pBuf  = (T)floor(ampl * sin( phase ) + 0.5);
		pBuf  += chanNum;
		phase += twiddle;
	}
	if( p.g_nIsDebugMarker )
	{
		pBuf = (T*)pvBuf;
		pBuf[ chanIdx + (nSamples-1)*chanNum ] = valc;
	}

}

//******************** CalcSignalToChan *******************
//*********************************************************
/*
S32 CalcSignalToChan(int lid, void *pvBuf, SBIG nSamples, S32 signalType,
					  S32 sampleWidth, S32 chanIdx, S32 chanNum,
					  double twiddle, double ampl, double *pPhase )
{
	double phase = *pPhase;
	//
	// Проверить аргументы
	//
	if( ampl < 0.0 )
		switch( sampleWidth )
		{
			case 1: ampl = 128/2; break;
			case 2: ampl = 32767/2; break;
			case 4: ampl = 2147483647/2; break;
		}

	//
	// Отсчеты сигнала имеют размер 1 байт
	//
	if( sampleWidth == 1 )
		fillBufferForDac<S08>(lid,  pvBuf, nSamples, signalType, chanIdx, chanNum, phase, twiddle, ampl, 0x7f);		
	//
	// Отсчеты сигнала имеют размер 2 байта
	//
	if( sampleWidth == 2 )
		fillBufferForDac<S16>(lid, pvBuf, nSamples, signalType, chanIdx, chanNum, phase, twiddle, ampl, 0x7fff);		
	//
	// Отсчеты сигнала имеют размер 4 байта
	//
	if( sampleWidth == 4 )
		fillBufferForDac<S32>(lid, pvBuf, nSamples, signalType, chanIdx, chanNum, phase, twiddle, ampl, 0x7fffffff);		

	*pPhase = fmod( phase, PI2 );

	return 0;
}*/

//******************** CalcSignalToChan *******************
//*********************************************************
S32 CalcSignalToChan(int lid, void *pvBuf, SBIG nSamples, S32 signalType,
					  S32 sampleWidth, S32 chanIdx, S32 chanNum,
					  double twiddle, double ampl, double *pPhase )
{
	SBIG		ii;
	double		phase = *pPhase;
	ParamsDACs& p = DevicesLid[lid].paramsDac;
	//
	// Проверить аргументы
	//
	if( ampl < 0.0 )
		switch( sampleWidth )
		{
			case 1: ampl = 128/2; break;
			case 2: ampl = 32767/2; break;
			case 4: ampl = 2147483647/2; break;
		}

	//
	// Отсчеты сигнала имеют размер 1 байт
	//
	if( sampleWidth == 1 )
	{
		S08		*pBuf = (S08*)pvBuf;

		if( ampl < 0.0 )
			ampl = 64.0;

		pBuf += chanIdx;
		for( ii=0; ii<nSamples; ii++ )
		{
			*pBuf  = (S08)floor(ampl * sin( phase ) + 0.5);
			if( signalType == 2 )
				*pBuf = (S08)(( *pBuf < 0.0 ) ? -ampl : +ampl);
			pBuf  += chanNum;
			phase += twiddle;
		}
		if( p.g_nIsDebugMarker )
		{
			pBuf = (S08*)pvBuf;
			pBuf[ chanIdx + (nSamples-1)*chanNum ] = 0x7f;
		}
	}

	//
	// Отсчеты сигнала имеют размер 2 байта
	//
	if( sampleWidth == 2 )
	{
		S16		*pBuf = (S16*)pvBuf;

		if( ampl < 0.0 )
			ampl = 16384.0;

		pBuf += chanIdx;
		for( ii=0; ii<nSamples; ii++ )
		{
			*pBuf  = (S16)floor(ampl * sin( phase ) + 0.5);
			if( signalType == 2 )
				*pBuf = (S16)(( *pBuf < 0.0 ) ? -ampl : +ampl);
			pBuf  += chanNum;
			phase += twiddle;
		}
		if( p.g_nIsDebugMarker )
		{
			pBuf = (S16*)pvBuf;
			pBuf[ chanIdx + (nSamples-1)*chanNum ] = 0x7fff;
		}
	}

	//
	// Отсчеты сигнала имеют размер 4 байта
	//
	if( sampleWidth == 4 )
	{
		S32		*pBuf = (S32*)pvBuf;

		if( ampl < 0.0 )
			ampl = 1024.0 * 1024.0 * 1024.0;

		pBuf += chanIdx;
		for( ii=0; ii<nSamples; ii++ )
		{
			*pBuf  = (S32)floor(ampl * sin( phase ) + 0.5);
			if( signalType == 2 )
				*pBuf = (S32)(( *pBuf < 0.0 ) ? -ampl : +ampl);
			pBuf  += chanNum;
			phase += twiddle;
		}
		if( p.g_nIsDebugMarker )
		{
			pBuf = (S32*)pvBuf;
			pBuf[ chanIdx + (nSamples-1)*chanNum ] = 0x7fffffff;
		}
	}

	*pPhase = fmod( phase, PI2 );

	return 0;
}


//=****************** FillSignalFromFile ******************
//=********************************************************
S32 FillSignalFromFile(int lid, void *pvBuf, SBIG nSamples, TDacParam& dac, int cnt )
{
	SBIG		bufSizeb;
	ParamsDACs& p = DevicesLid[lid].paramsDac;

	bufSizeb = nSamples
				* dac.chanNum
				* dac.sampleSizeb;
	//
	// Если первый буфер, позиционировать файл в начало
	//
	if( cnt==0 )
		fseek( p.g_pFileDataBin, 0, SEEK_SET );

	memset( pvBuf, 0, bufSizeb );
	fread( pvBuf, 1, bufSizeb, p.g_pFileDataBin );

	return 0;
}

//=******************** CorrectOutFreq ********************
//=********************************************************
S32 CorrectOutFreq(TDacParam& dac)
{
	double npi;			// Число периодов выходного синуса (пока не целое)
	double nnp;			// Число периодов выходного синуса (целое гарантировано)
	double delta;		// Поворачивающий фактор
	double freq;		// Скорректированная частота

	delta = PI2 * dac.dSignalFreq / dac.dSamplingRate;
	npi   = delta * (double)dac.samplesPerChannel / PI2;
	nnp   = floor(npi+0.5);
	delta = PI2 * nnp / (double)dac.samplesPerChannel;
	freq  = floor(delta * dac.dSamplingRate / PI2 + 0.5);

	dac.dSignalFreq  = freq;

	return 0;
}




//=****************** ReadIniFileOption *******************
//=********************************************************
S32  ReadIniFileOption( const std::string fileIni, int lid )
{
	S32		err;
	S32		ii;
	BRDCHAR sBuffer[1024];

	ParamsDACs& p = DevicesLid[lid].paramsDac;

	err = IPC_getFullPath( fileIni.c_str(), p.g_sFullName );
	if( 0 > err )
	{
		BRDC_printf( _BRDC("ERROR: Can't find ini-file '%s'\n\n"), fileIni );
		return -1;
	}

	GetInifileString( _BRDC("Option"), _BRDC("ServiceName"), _BRDC("DACNAME"), p.g_sServiceName, sizeof(p.g_sServiceName), p.g_sFullName );
	GetInifileString( _BRDC("Option"), _BRDC("PldFileName"), _BRDC("PLDNAME"), p.g_sPldFileName, sizeof(p.g_sPldFileName), p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("IsNew"), _BRDC("1"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nIsNew = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("IsNew"), _BRDC("1"), p.g_nIsNew, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("DebugMarker"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nIsDebugMarker = BRDC_atoi( sBuffer );
	GetInifileInt( _BRDC("Option"), _BRDC("DebugMarker"), _BRDC("0"), p.g_nIsDebugMarker, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("Cycle"), _BRDC("1"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nCycle = BRDC_atoi( sBuffer );
	GetInifileInt( _BRDC("Option"), _BRDC("Cycle"), _BRDC("1"), p.g_nCycle , p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("QuickQuit"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nQuickQuit = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("QuickQuit"), _BRDC("0"), p.g_nQuickQuit , p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("WorkMode"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nWorkMode = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("WorkMode"), _BRDC("0"), p.g_nWorkMode, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("IsAlwaysWriteSdram"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nIsAlwaysWriteSdram = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("IsAlwaysWriteSdram"), _BRDC("0"), p.g_nIsAlwaysWriteSdram, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("SamplesPerChannel"), _BRDC("1024"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nSamplesPerChannel = BRDC_atoi64( sBuffer );
	GetInifileBig(_BRDC("Option"), _BRDC("SamplesPerChannel"), _BRDC("1024"), p.g_nSamplesPerChannel, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("IsSystemMemory"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nIsSystemMemory = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("IsSystemMemory"), _BRDC("0"), p.g_nIsSystemMemory, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("SdramWriteBufSize"), _BRDC("4"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nSdramWriteBufSize = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("SdramWriteBufSize"), _BRDC("4"), p.g_nSdramWriteBufSize, p.g_sFullName );
	p.g_nSdramWriteBufSize *= 1024;

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("IsAlwaysLoadPld"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nIsAlwaysLoadPld = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("IsAlwaysLoadPld"), _BRDC("0"), p.g_nIsAlwaysLoadPld, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("MasterSlave"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nMasterSlave = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("MasterSlave"), _BRDC("0"), p.g_nMasterSlave, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("DmaBufFactor"), _BRDC("4096"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_nDmaBufFactor = BRDC_atoi( sBuffer );
	GetInifileInt( _BRDC("Option"), _BRDC("DmaBufFactor"), _BRDC("4096"), p.g_nDmaBufFactor, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("SignalFreq"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_dSignalFreq = (double)BRDC_atoi( sBuffer );
	GetInifileFloat(_BRDC("Option"), _BRDC("SignalFreq"), _BRDC("0"), p.g_dSignalFreq, p.g_sFullName );


	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("SignalType"), _BRDC("0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
	//p.g_dSignalType  = BRDC_atoi( sBuffer );
	GetInifileInt(_BRDC("Option"), _BRDC("SignalType"), _BRDC("0"), (int&)p.g_dSignalType, p.g_sFullName );

	//IPC_getPrivateProfileString( _BRDC("Option"), _BRDC("SignalFile"), _BRDC(""), sBuffer, sizeof(sBuffer), p.g_sFullName );
	GetInifileString(_BRDC("Option"), _BRDC("SignalFile"), _BRDC(""), sBuffer, sizeof(sBuffer), p.g_sFullName );
	p.g_pFileDataBin = BRDC_fopen( sBuffer, _BRDC("rb") );
	if( '\0' != sBuffer[0] )
		if( NULL == p.g_pFileDataBin )
		{
            BRDC_printf( _BRDC( "\n\tERROR: Can't open file '%s'\n\n"), sBuffer );
			return -1;
		}

/*	for( auto& dev : DevicesLid[lid].dac)
	{
		dev.dSignalFreq = p.g_dSignalFreq;
		dev.signalType  = p.g_dSignalType;
		printf("signalFreq = %.2f ", p.g_dSignalFreq);
	}
*/
	//IPC_getPrivateProfileString( _BRDC( "Option" ), _BRDC( "TimeoutSec" ), _BRDC( "5" ), sBuffer, sizeof( sBuffer ), p.g_sFullName );
	GetInifileInt(_BRDC( "Option" ), _BRDC( "TimeoutSec" ), _BRDC( "5" ), p.g_nMsTimeout, p.g_sFullName );
	p.g_nMsTimeout *= 1000;

	return 0;
}

//=****************** ReadIniFileDevice *******************
//=********************************************************
S32  ReadIniFileDevice( int lid )
{
	int		 jj;
	BRDCHAR sBuffer[512];
	BRDCHAR sParamName[128];
	double	dAmpl;
	int rv;

	ParamsDACs& p = DevicesLid[lid].paramsDac;

	for( auto& dev : DevicesLid[lid].dac )
	{
		BRDC_printf("<SRV> Read ini-file >%s< from section >%s<\n", p.g_sFullName, dev.paramDac.sSection );		
		//IPC_getPrivateProfileString( dev.paramDac.sSection, _BRDC("Ampl"), _BRDC("-1"), sBuffer, sizeof(sBuffer), p.g_sFullName );
		//dAmpl = BRDC_atof( sBuffer );
		GetInifileFloat(dev.paramDac.sSection, _BRDC("Ampl"), _BRDC("-1"), dAmpl, p.g_sFullName );

		for( jj=0; jj<MAX_CHAN; jj++ )
		{
			BRDC_sprintf( sParamName, _BRDC("Ampl%d"), jj );
			//IPC_getPrivateProfileString( dev.paramDac.sSection, sParamName, _BRDC(""), sBuffer, sizeof(sBuffer), p.g_sFullName );
			//dev.paramDac.aAmpl[jj] = (*sBuffer) ? BRDC_atof(sBuffer) : dAmpl;
			rv = GetInifileFloat(dev.paramDac.sSection, sParamName, _BRDC(""), dev.paramDac.aAmpl[jj], p.g_sFullName );
			if(! rv)
				dev.paramDac.aAmpl[jj] = dAmpl;

			BRDC_sprintf( sParamName, _BRDC("Phase%d"), jj );
			//IPC_getPrivateProfileString( dev.paramDac.sSection, sParamName, _BRDC(""), sBuffer, sizeof(sBuffer), p.g_sFullName );
			//dev.paramDac.aPhase[jj] = (*sBuffer) ? BRDC_atof(sBuffer) : 0.0;
			rv = GetInifileFloat(dev.paramDac.sSection, sParamName, _BRDC(""), dev.paramDac.aPhase[jj], p.g_sFullName );
			if(! rv)
				dev.paramDac.aPhase[jj] = 0.0;			
			dev.paramDac.aPhase[jj] /= 360;
			dev.paramDac.aPhase[jj] *= PI2;
		}

		//IPC_getPrivateProfileString( dev.paramDac.sSection, _BRDC("ThresholdComp0"), _BRDC("0.0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
		//dev.paramDac.aThdac[0] = BRDC_atof(sBuffer);
		GetInifileFloat(dev.paramDac.sSection, _BRDC("ThresholdComp0"), _BRDC("0.0"), dev.paramDac.aThdac[0], p.g_sFullName );
		//IPC_getPrivateProfileString( dev.paramDac.sSection, _BRDC("ThresholdComp1"), _BRDC("0.0"), sBuffer, sizeof(sBuffer), p.g_sFullName );
		//dev.paramDac.aThdac[1] = BRDC_atof(sBuffer);
		GetInifileFloat(dev.paramDac.sSection, _BRDC("ThresholdComp1"), _BRDC("0.0"), dev.paramDac.aThdac[1], p.g_sFullName );

		//IPC_getPrivateProfileString( dev.paramDac.sSection, _BRDC("SwitchIn"), _BRDC("-1"), sBuffer, sizeof(sBuffer), p.g_sFullName );
		//dev.paramDac.switchIn = BRDC_atoi(sBuffer);
		GetInifileInt(dev.paramDac.sSection, _BRDC("SwitchIn"), _BRDC("-1"), dev.paramDac.switchIn, p.g_sFullName );
		//IPC_getPrivateProfileString( dev.paramDac.sSection, _BRDC("SwitchOut"), _BRDC("-1"), sBuffer, sizeof(sBuffer), p.g_sFullName );
		//dev.paramDac.switchOut = BRDC_atoi(sBuffer);
		GetInifileInt(dev.paramDac.sSection, _BRDC("SwitchOut"), _BRDC("-1"), dev.paramDac.switchOut, p.g_sFullName );

		GetInifileString( dev.paramDac.sSection, _BRDC("RegFileName"), _BRDC(""),
				dev.paramDac.sRegRwSpdFilename, sizeof(dev.paramDac.sRegRwSpdFilename), p.g_sFullName );
	}

	return 0;
}

//=****************** DisplayDacTraceText *****************
//=********************************************************
S32 DisplayDacTraceText( int loop, int lid )
{
	BRD_TraceText		rTraceText;
	BRDCHAR				lin[200] = _BRDC("oops");
	int					err;

	rTraceText.traceId = 1;
	rTraceText.sizeb = sizeof(lin);
	rTraceText.pText = lin;

	if( DevicesLid[lid].dac.size() <= 0)
		return -1;

	if(!DevicesLid[lid].dac[0].outText)
		return 0;

	err = BRD_ctrl( DevicesLid[lid].dac[0].getService(), 0, BRDctrl_GETTRACETEXT, &rTraceText );
	if( err<0 )
		BRDC_printf( _BRDC("LID:%d [%d]\r"), lid, loop );
	else
		BRDC_printf( _BRDC("LID:%d %d [%s]\r"), lid, loop, lin );

	return err;
}

//
// End of file
//

void ListParameters(int lid)
{
	ParamsDACs& p = DevicesLid[lid].paramsDac;
	printf("/n List DAC parameters : \n");
	//printf("g_iniFileName = %s\n",p.g_iniFileName);
	printf("g_sFullName = %s\n",p.g_sFullName);
	//printf("g_lid = %d\n",p.g_lid);
	//printf("g_nDevNum = %d\n",p.g_nDevNum);
	//printf("g_nDacNum = %d\n",p.g_nDacNum);
	printf("g_dSamplingRate = %f\n",p.g_dSamplingRate);
	printf("g_dSignalFreq = %f\n",p.g_dSignalFreq);
	printf("from ini-file : \n");
	printf("g_sServiceName = %s\n",p.g_sServiceName);
	printf("g_nWorkMode = %d\n",p.g_nWorkMode);
	printf("g_nIsAlwaysWriteSdram = %d\n",p.g_nIsAlwaysWriteSdram);
	printf("g_nSamplesPerChannel = %llu\n",p.g_nSamplesPerChannel);
	printf("g_nIsSystemMemory = %d\n",p.g_nIsSystemMemory);
	printf("g_nSdramWriteBufSize = %d\n",p.g_nSdramWriteBufSize);
	printf("g_nIsAlwaysLoadPld = %d\n",p.g_nIsAlwaysLoadPld);
	printf("g_nMasterSlave = %d\n",p.g_nMasterSlave);
	printf("g_nDmaBufFactor = %d\n",p.g_nDmaBufFactor);
	printf("g_nIsDebugMarker = %d\n",p.g_nIsDebugMarker );
	printf("g_nCycle = %d\n",p.g_nCycle );
	printf("g_nQuickQuit = %d\n",p.g_nQuickQuit );
	printf("g_nIsNew = %d\n",p.g_nMsTimeout );
	printf("g_sPldFileName = %s\n",p.g_sPldFileName);
	int i=0;
	for(auto dac : DevicesLid[x_lid].dac) {
	//for(size_t i=0; i<g_nDacNum; i++) {
		printf("DAC #%d : \n", i++);
		printf("  sSection = %s\n", dac.paramDac.sSection);
		printf("  sampleSizeb = %d\n", dac.paramDac.sampleSizeb);
		printf("  samplesPerChannel = %llX (0x%llX)\n", dac.paramDac.samplesPerChannel, dac.paramDac.samplesPerChannel);
		printf("  chanMask = %X\n", dac.paramDac.chanMask);
		printf("  chanNum = %d\n", dac.paramDac.chanNum);
		printf("  chanMaxNum = %d\n", dac.paramDac.chanMaxNum);
		printf("  aThdac[0] = %f\n", dac.paramDac.aThdac[0]);
		printf("  aThdac[1] = %f\n", dac.paramDac.aThdac[1]);
		for(size_t c=0; c<MAX_CHAN; c++) {
			printf("  aAmpl[%d] = %f", c, dac.paramDac.aAmpl[c]);
			printf("  aPhase[%d] = %f", c, dac.paramDac.aPhase[c]);
			printf("  aPhaseKee[%d] = %f\n", c, dac.paramDac.aPhaseKee[c]);
		}	
		printf("  signalType = %s\n", dac.paramDac.signalType==0?"sine":"meander");
		printf("  outBufSizeb = %llX\n", dac.paramDac.outBufSizeb);
		printf("  switchIn = %X\n", dac.paramDac.switchIn);
		printf("  switchOut = %X\n", dac.paramDac.switchOut);
		printf("  nFifoSizeb = %X\n", dac.paramDac.nFifoSizeb);
		printf("  dSamplingRate = %f\n", dac.paramDac.dSamplingRate);
		printf("  dSignalFreq = %f\n", dac.paramDac.dSignalFreq);
		printf("  sRegRwSpdFilename = %s\n", dac.paramDac.sRegRwSpdFilename);
		printf("  rBufAlloc = %d\n", dac.paramDac.rBufAlloc);
	//	printf("   = %d", dac.);
	}
	//printf(" = %d\n",  );
	printf("/n");
	//BRD_Handle	g_hDev[MAX_DEVICE];
	//TDacParam	g_aDac[MAX_DAC];
	//BRD_Handle	g_idx[MAX_DAC];			// Рассортированные номера ЦАПов для MASTER/SLAVE
}
