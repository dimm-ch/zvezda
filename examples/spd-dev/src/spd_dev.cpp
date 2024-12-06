// spd_dev.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "brd.h"
#include "ctrlreg.h"

void SpdReg(BRD_Handle hReg,  BRDCHAR* fname);
void SpdRegCpp(  BRDCHAR* fname );

int BRDC_main(int argc, BRDCHAR* argv[])
{
	BRDCHAR fname[64];

	BRDC_strcpy(fname, _BRDC("spd_dev.ini") );

	bool cppflag = false;

	for( int i=1; i<argc; i++ )
	{
		BRDCHAR cc = argv[i][1];

		if( argv[i][0] != '-' )
		{
			BRDC_strcpy(fname, argv[i] );
			continue;
		}
		else if( BRDC_strcmp( &argv[i][1], _BRDC("cpp") )==0 )
		{
			cppflag = true;
		}
	}

	if( cppflag )
	{
		SpdRegCpp( fname );
		return 0;
	}

	S32		status;
	BRD_displayMode(BRDdm_VISIBLE | BRDdm_CONSOLE);

	S32	DevNum;
	status = BRD_init(_BRDC("brd.ini"), &DevNum);
	if(!BRD_errcmp(status, BRDerr_OK))
	{
		BRDC_printf( _BRDC("ERROR: BARDY Initialization = 0x%X\n"), status );
		BRDC_printf( _BRDC("Press any key for leaving program...\n"));
		_getch();
		return -1;
	}

	// получить список LID (каждая запись соответствует устройству)
	BRD_LidList lidList;
	lidList.item = 10;
	lidList.pLID = new U32[10];
	status = BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);


	BRD_Info	info;
	info.size = sizeof(info);
	BRD_Handle handle[10];

	for(ULONG i = 0; i < lidList.itemReal; i++)
	{
		BRD_getInfo( lidList.pLID[i], &info );
		BRDC_printf(_BRDC("Board: %s, DevID = 0x%x, RevID = 0x%x, Bus = %d, Dev = %d, Slot = %d.\n"),
						info.name, info.boardType >> 16, info.boardType & 0xff, info.bus, info.dev, info.slot);
		U32 open_mode;
//		handle[i] = BRD_open(lidList.pLID[i], BRDopen_EXCLUSIVE, &open_mode);
//		if(handle[i] > 0 && BRDopen_EXCLUSIVE == open_mode)
		handle[i] = BRD_open(lidList.pLID[i], BRDopen_SHARED, &open_mode);
		if(handle[i] > 0 && BRDopen_SHARED == open_mode)
		{
			U32 mode = BRDcapt_SHARED;
			BRD_Handle hReg = BRD_capture(handle[i], 0, &mode, _BRDC("REG0"), 10000);
			if(hReg <= 0)
			{
				BRDC_printf(_BRDC("REG NOT capture (%X)\n"), hReg);
				break;
			}
			if(mode == BRDcapt_SHARED)
				BRDC_printf(_BRDC("REG Capture mode: SHARED (%X)\n"), hReg);
			else
				break;

			SpdReg(hReg, fname);

			status = BRD_release(hReg, 0);

			status = BRD_close(handle[i]);
			printf("BRD_close: OK\n");
		}
	}
	status = BRD_cleanup();
	BRDC_printf(_BRDC("BRD_cleanup: OK\n"));
	BRDC_printf(_BRDC("Press any key for leaving program...\n"));
	_getch();

	return 0;
}
