#include "fm415s_test.h"
#include <string>
#include <cstring>
#include <sstream>
#include <chrono>
using namespace std;
using namespace std::chrono;
using namespace std::chrono_literals;
static volatile int exit_flag = 0;

void show_help(const std::string& program)
{
	fprintf(stderr, "usage: %s <options>\n", program.c_str());
	fprintf(stderr, "Options description:\n");
	fprintf(stderr, "-b <lid> - LID of 1 board\n");
	fprintf(stderr, "-r <lid> - LID of 2 board\n");	
	fprintf(stderr, "-h - Print this message\n");
}

void signal_handler(int signo) {
	signo = signo;
	exit_flag = 1;
	fprintf(stderr, "\n");
}

template<typename T>
T fromString(const std::string& s)
{
	std::istringstream iss(s);
	T res;
	if( strstr(s.c_str(), "0x") ) {
		iss >> std::hex >> res;
	}
	else {
		iss >> res;
	}
	return res;
}

bool is_option(int argc, char** argv, const char* name)
{
	for( int ii = 1; ii < argc; ii++ ) {
		if( strcmp(argv[ii], name) == 0 ) {
			return true;
		}
	}
	return false;
}

template<typename T> T get_from_cmdline(int argc, char** argv, const char* name, T defValue)
{	
	T res(defValue);
	for( int ii = 1; ii < argc - 1; ii++ ) {
		if( strcmp(argv[ii], name) == 0 ) {
			std::string val = argv[ii + 1];
			res = fromString<T>(val);
		}
	}
	return res;
}

int BRDC_main(int argc, char *argv[])
{
	U32 apLidList[32] = { 0 }, nItemReal=0, nMode = BRDcapt_EXCLUSIVE, nTetrNum=0xFF, nMainTetrNum=0xFF;
	U32 nTimeout = 1000;
	S32 nStatus = 0, nDevNum = 0, bLid[2] = { -1 };
	BRD_Handle hBrd[2], h415[2] = { -1 };
	BRD_ServList aServices[32];
	BRD_Info rInfo = { sizeof(rInfo) };
	IPC_initKeyboard();
	signal(SIGINT, signal_handler);
	IPC_TIMEVAL tStart, tStop;
		
	g_nTestType = 0;
	g_nLid[0] = 0xFFFFFFFF;
	g_nLid[1] = 0xFFFFFFFF;
	g_isTStudio = false;
	BRDC_strcpy(g_sIniFileName, _BRDC("fm415s_test.ini"));
	g_nLid[0] = get_from_cmdline(argc, argv, "-b", -1);
	BRDC_printf(_BRDC("LID1:%d\n"), g_nLid[0]);
	g_nLid[1] = get_from_cmdline(argc, argv, "-r", -1);
	BRDC_printf(_BRDC("LID2:%d\n"), g_nLid[1]);
	U32 is_help = is_option(argc, argv, "-h");
	if( is_help )
	{
		show_help(argv[0]);
		return(0);
	}
	if (ParseIniFile(g_sIniFileName) < 0)
	{
		BRDC_printf(_BRDC("ERROR: no ini file %s !!\n"), g_sIniFileName);
		return TEST_ERR_NO_INI;
	}

	nStatus = BRD_init(_BRDC("brd.ini"), &nDevNum);
	if (nStatus < 0)
	{
		BRDC_printf(_BRDC("ERROR: error in brd.ini 0x%08X\n"), nStatus);
		return TEST_ERR_BRD_INI;
	}
	nStatus = BRD_lidList(apLidList, 32, &nItemReal);
	if (nStatus < 0)
	{
		BRDC_printf(_BRDC("ERROR: error in BRD_lidList=0x%08X\n"), nStatus);
		IPC_cleanupKeyboard();
		BRD_cleanup();
		return TEST_ERR_LIDLIST;
	}
	if (nItemReal < 2)
	{
		BRDC_printf(_BRDC("ERROR: need 2 LIDs, found %d\n"), nItemReal);
		IPC_cleanupKeyboard();
		BRD_cleanup();
		return TEST_ERR_NO_LIDS;
	}
	if (nItemReal > 32)
		nItemReal = 32;
	
	if (g_nLid[0] != 0xFFFFFFFF)
	{
		//for (U32 i = 0; i < nItemReal; i++)
		//	if (apLidList[i] == g_nLid)
		//		bLid = apLidList[i];
		//if (bLid == 0xFFFFFFFF)
		//{
		//	BRD_cleanup();
		//	IPC_cleanupKeyboard();
		//	BRDC_printf(_BRDC("ERROR: LID=%d not found\n"), bLid);
		//	return TEST_ERR_LID_NOT_FOUND;
		//}
		bLid[0] = g_nLid[0];
	}
	else
		bLid[0] = apLidList[0];
	if( g_nLid[1] != 0xFFFFFFFF )
	{
		bLid[1] = g_nLid[1];
	}
	else
		bLid[1] = apLidList[1];
	if( bLid[0] == bLid[1] )
	{
		BRDC_printf(_BRDC("ERROR: need different LIDs\n"));
		return TEST_ERR_NO_LIDS;
	}
	for( int i = 0; i < 2; i++ )
	{
		hBrd[i] = BRD_open(bLid[i], nMode, &nMode);
		if( hBrd[i] < 0 )
		{
			BRDC_printf(_BRDC("ERROR: error in BRD_open=0x%08X\n"), nStatus);
			IPC_cleanupKeyboard();
			BRD_cleanup();
			return TEST_ERR_OPEN;
		}
		else
		{
			BRD_getInfo(bLid[i], &rInfo);
			BRDC_printf(_BRDC("LID%d %s (0x%04X) pid=%d mode=%d\n\n"), bLid[i], rInfo.name, rInfo.boardType, rInfo.pid, nMode);
		}
		if( nMode == 2 )
		{
			BRDC_printf(_BRDC("ERROR: LID%d opened in SPY mode\n"), bLid[i]);
			return TEST_ERR_OPEN;
		}
		nStatus = BRD_serviceList(hBrd[i], 0, aServices, 32, &nItemReal);
		if( nStatus < 0 )
		{
			BRDC_printf(_BRDC("ERROR: error in BRD_serviceList=0x%08X\n"), nStatus);
			IPC_cleanupKeyboard();
			BRD_cleanup();
			return TEST_ERR_SERVICELIST;
		}
		if( nItemReal < 1 )
		{
			BRDC_printf(_BRDC("ERROR: no services\n"));
			IPC_cleanupKeyboard();
			BRD_cleanup();
			return TEST_ERR_NO_SERVICES;
		}
		if( nItemReal > 32 )
			nItemReal = 32;
		for( U32 ii = 0; ii < nItemReal; ii++ )
		{
			BRDC_printf(_BRDC("Service %d: %s ATTR=0x%X\n"), ii, aServices[ii].name, aServices[ii].attr);
			if( !BRDC_strcmp(aServices[ii].name, _BRDC("FM415S0")) )
			{
				h415[i] = BRD_capture(hBrd[i], 0, &nMode, aServices[ii].name, 10000);
				//break;
			}
		}
		if( h415[i] < 0 )
		{
			BRDC_printf(_BRDC("ERROR: FM415S service not found\n"));
			IPC_cleanupKeyboard();
			BRD_cleanup();
			return TEST_ERR_NO_415S;
		}
	}
	BRDC_printf(_BRDC("\n"));
	for( int i = 0; i < 2; i++ )
	{
		double rate = g_dRate;
		BRDC_printf(_BRDC("LID%d:set si571 rate %f\n"),bLid[i], rate);
		nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETCLKVALUE, &rate);
		if( nStatus < 0 )
			BRDC_printf(_BRDC("ERROR: set clock value error 0x%X rate=%f Hz\n"), nStatus, rate);
		else
		{
			double dRate = 0.0;
			BRD_ctrl(h415[i], 0, BRDctrl_FM415S_GETCLKVALUE, &dRate);
			BRDC_printf(_BRDC("LID%d:set si571 rate %f\n"), bLid[i], dRate);
		}

		U32 clk_mode = 1;
		nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETCLKMODE, &clk_mode);
	}
	IPC_delay(100);
	for( int i = 0; i < 2; i++ )
	{
		U32 nChanMask = g_nChanMask;
		nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETCHANMASK, &nChanMask);
		if( nStatus < 0 )
		{
			U32 nChanMask2 = 0;
			nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_GETCHANMASK, &nChanMask2);
			BRDC_printf(_BRDC("ERROR: channel mask 0x%X not set.\nChannel mask 0x%X\n"), nChanMask, nChanMask2);
			g_nChanMask = nChanMask;
		}
		else
		{
			nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_GETCHANMASK, &nChanMask);
			g_nChanMask = nChanMask;
		}
		if( (g_nChanMask & 0xFF) == 0 )
		{
			BRDC_printf(_BRDC("ERROR: no channels set to active\n"));
			IPC_cleanupKeyboard();
			BRD_cleanup();
			return TEST_ERR_CHAN_MASK;
		}
		BRDC_printf(_BRDC("Channel mask 0x%X\n"), g_nChanMask);
	}
	
	//BRDC_printf(_BRDC("set decimation 0x%X\n"), g_nDecim);
	//for( int i = 0; i < 2; i++ )
	//{
	//	nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETDECIM, &g_nDecim);
	//	if( nStatus < 0 )
	//		BRDC_printf(_BRDC("ERROR: set decimation error 0x%X\n"), nStatus);
	//	else
	//	{
	//		U32 nDecim = 0;
	//		nStatus = BRD_ctrl(h415[i], 0, BRDctrl_FM415S_GETDECIM, &nDecim);
	//		BRDC_printf(_BRDC("set decimation 0x%X\n"), nDecim);
	//	}
	//}
	for( int i = 0; i < 2; i++ )
	{
		FM415S_TESTMODE rTest = { 0 };
		switch( g_nTestType )
		{
		case 0://256 �������� ������������������
		default:
			rTest.isGenEnable = 1;
			BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETTESTMODE, &rTest);
			break;
		case 1://64 �������
			rTest.isGenEnable = 1;
			rTest.isCntEnable = 1;
			BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETTESTMODE, &rTest);
			break;
		case 2://��� �����
			BRD_ctrl(h415[i], 0, BRDctrl_FM415S_SETTESTMODE, &rTest);
			break;
		}
	}

	for( int i = 0; i < 2; i++ )
		BRD_ctrl(h415[i], 0, BRDctrl_FM415S_PREPARE, NULL);
	//printf("wait %d sec\n", nTimeout);
	IPC_delay(nTimeout*2);

	FM415S_GETSTATUS rStat[2] = { 0 };
	bool isBreak = true;
	for (int i = 0; i < 100; i++)
	{	
		isBreak = true;
		for( int ii = 0; ii < 2; ii++ )
		{
			BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_GETSTATUS, &(rStat[ii]));

			for( int j = 0; j < 4; j++ )
			{
				if( (g_nChanMask >> j) & 1 )
				{
					if( (rStat[ii].isPLL_Lock[j] != 1) || (rStat[ii].isLaneUp[j] != 1) )
						isBreak = false;
				}
			}
		}
		if (isBreak)
			break;
		BRDC_printf(_BRDC("retry #%d wait %d ms\r"), i + 1, nTimeout / 2);
		IPC_delay(nTimeout/2);		
	}
	BRDC_printf(_BRDC("\n"));
	if (!isBreak)
	{
		for( int ii = 0; ii < 2; ii++ )
		{
			printf("LID%d\n", bLid[ii]);
			for( int i = 0; i < 4; i++ )
			{
				if( (g_nChanMask >> i) & 1 )
				{
					if( rStat[ii].isPLL_Lock[i] != 1 )
						BRDC_printf(_BRDC("ERROR: no PLL lock for channel %d\n"), i);
					if( rStat[ii].isLaneUp[i] != 1 )
						BRDC_printf(_BRDC("ERROR: no lane for channel %d\n"), i);
				}
			}
		}
	}	
	for( int i = 0; i < 2; i++ )
		BRD_ctrl(h415[i], 0, BRDctrl_FM415S_START, NULL);
	printf("start\n");
	IPC_getTime(&tStart);
	FM415S_GETRXTXSTATUS rRxTx = { 0 };
	auto start_prog = high_resolution_clock::now();
	while (!IPC_kbhit())
	{
		auto current_cycle = high_resolution_clock::now();
       	auto duration = duration_cast<seconds>(current_cycle - start_prog); 
		printf("[%lld]",duration);
		for( int ii = 0; ii < 2; ii++ )
		{
			printf("LID%d:", bLid[ii]);
			BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_GETRXTXSTATUS, &(rRxTx));
			for( int i = 0; i < 4; i++ )
				if( ((g_nChanMask >> i) & 1) != 0 )
					printf("c%d:w%02X;r%02X;err[0x%X] ",i, rRxTx.nBlockWrite[i]&0xFF, rRxTx.nBlockRead[i]&0xFF, rRxTx.nBlockErr[i]);
				
			BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_GETSTATUS, &(rStat[ii]));
			for( int j = 0; j < 4; j++ )
			{
				if( (g_nChanMask >> j) & 1 )
				{
					if (rStat[ii].isLaneUp[j] != 1)
					{
						printf("\nError: lost LaneUp in chan%d LID%d\n", j, bLid[ii]);
						BRDextn_FMCPOWER power = { 0 };
						BRD_extension(hBrd[0], 0, BRDextn_SET_FMCPOWER, &power);
						BRD_extension(hBrd[1], 0, BRDextn_SET_FMCPOWER, &power);
						BRD_close(hBrd[0]);
						BRD_close(hBrd[1]);
						IPC_cleanupKeyboard();
						BRD_cleanup();
						return 1;
						break;
					}
				}
			}
		}
		printf("\r");
		IPC_delay(250);
	}
	printf("\n");
	for( int ii = 0; ii < 2; ii++ )
	{
		BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_GETRXTXSTATUS, &(rRxTx));
		for( int i = 0; i < 4; i++ )
			if( ((g_nChanMask >> i) & 1) != 0 )
			{
				if( rRxTx.nBlockErr[i] != 0 )
				{
					BRDC_printf(_BRDC("ERROR: LID%d channel %d\n"),bLid[ii], i);
					FM415S_GETTESTRESULT rTestRes = { 0 };
					U32 nErrors = 0;
					rTestRes.nChan = i;
					BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_GETTESTRESULT, &rTestRes);
					BRDC_printf(_BRDC("Total errors %d\n"), rTestRes.nTotalError);
					if( rTestRes.nTotalError != 0 )
					{
						if( rTestRes.nTotalError > 32 )
							nErrors = 32;
						else
							nErrors = rTestRes.nTotalError;
						for( U32 j = 0; j < nErrors; j++ )
						{
							if( rTestRes.lReadWord[j] != rTestRes.lExpectWord[j] )
								BRDC_printf(_BRDC("block %d index %d read 0x%llX expected 0x%llX xor 0x%llX\n"), rTestRes.nBlock[j], rTestRes.nIndex[j], rTestRes.lReadWord[j], rTestRes.lExpectWord[j], rTestRes.lReadWord[j] ^ rTestRes.lExpectWord[j]);
						}
					}
				}
			}
	}
	//BRDC_printf(_BRDC("\n"));
	for( int ii = 0; ii < 2; ii++ )
		BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_STOP, 0);
	IPC_getTime(&tStop);
	double dTime = IPC_getDiffTime(&tStart, &tStop);
	double dSpeed = 0.0;
	for( int ii = 0; ii < 2; ii++ )
	{
		BRD_ctrl(h415[ii], 0, BRDctrl_FM415S_GETRXTXSTATUS, &(rRxTx));
		for( int i = 0; i < 4; i++ )
			if( ((g_nChanMask >> i) & 1) != 0 )
			{
				if( rRxTx.nBlockRead[i] != 0 )
				{
					dSpeed = rRxTx.nBlockRead[i] / (dTime / 1000.0);
					BRDC_printf(_BRDC("LID %d, Channel %d read %f block/sec\n"),bLid[ii], i, dSpeed);
				}
				if( rRxTx.nBlockWrite[i] != 0 )
				{
					dSpeed = rRxTx.nBlockWrite[i] / (dTime / 1000.0);
					BRDC_printf(_BRDC("LID %d, Channel %d write %f block/sec\n"),bLid[ii], i, dSpeed);
				}
				if( (rRxTx.nBlockRead[i] == 0) && (rRxTx.nBlockWrite[i] == 0) )
					BRDC_printf(_BRDC("ERROR: no data in LID %d channel %d\n"), bLid[ii], i);
			}
	}
	BRDextn_FMCPOWER power = { 0 };
	BRD_extension(hBrd[0], 0, BRDextn_SET_FMCPOWER, &power);
	BRD_extension(hBrd[1], 0, BRDextn_SET_FMCPOWER, &power);
	BRD_close(hBrd[0]);
	BRD_close(hBrd[1]);
	IPC_cleanupKeyboard();
	BRD_cleanup();
    return 0;
}

S32 ParseIniFile(BRDCHAR * filename)
{
	S32		err;	
	BRDCHAR sBuffer[1024], sFullName[MAX_PATH], *sTmp;

	err = IPC_getFullPath(g_sIniFileName, sFullName);
	if (0 > err)
	{
		BRDC_printf(_BRDC("ERROR: Can't find ini-file '%s'\n\n"), g_sIniFileName);
		return TEST_ERR_NO_INI;
	}
		
	IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("TestType"), _BRDC("0"), sBuffer, sizeof(sBuffer), sFullName);
	g_nTestType = BRDC_strtol(sBuffer, &sTmp, 0);
	IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("ChannelMask"), _BRDC("0xFF"), sBuffer, sizeof(sBuffer), sFullName);
	g_nChanMask = BRDC_strtol(sBuffer, &sTmp, 0);
	IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("Decim"), _BRDC("0"), sBuffer, sizeof(sBuffer), sFullName);
	g_nDecim = BRDC_strtol(sBuffer, &sTmp, 0);
	IPC_getPrivateProfileString(_BRDC("Option"), _BRDC("GenFreq"), _BRDC("162500000.0"), sBuffer, sizeof(sBuffer), sFullName);
	g_dRate = BRDC_strtod(sBuffer, &sTmp);
	return 0;
}
