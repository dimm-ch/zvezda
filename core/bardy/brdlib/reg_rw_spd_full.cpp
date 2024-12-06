// ***************** File spd_reg.cpp ************

#ifndef SPDDEVAPP
#include "gipcy.h"
#include "module.h"
#include "realdelay.h"
#include "brdapi.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include	"brd.h"

#ifdef TCLAPP
//svn://srv/utility/tcl
#include	"tcl.h"
#endif

#ifdef SPDDEVAPP
#include "ctrlreg.h"

class CModule
{
public:

	BRD_Handle handle;

	virtual S32 RegCtrl( U32 cmd, void *pCmd )
	{
		return BRD_ctrl( handle, 0, cmd, pCmd );
	}
};

#define DEVS_CMD_Reg BRD_Reg 

#define DEVScmd_REGWRITEIND BRDctrl_REG_WRITEIND
#define DEVScmd_REGREADIND BRDctrl_REG_READIND
#define DEVScmd_REGWRITEDIR BRDctrl_REG_WRITEDIR
#define DEVScmd_REGREADDIR BRDctrl_REG_READDIR



void SpdReg( CModule *pModule,  BRDCHAR* fname );

void SpdReg(BRD_Handle hReg,  BRDCHAR* fname)
{
	CModule module;

	module.handle = hReg;

	SpdReg( &module , fname );
}

S32 RegRwSpd(BRD_Handle hDev, BRDCHAR* fname)
{
	//	U32 mode = ::BRDcapt_EXCLUSIVE; 
	U32 mode = ::BRDcapt_SHARED;

	BRD_Handle hReg = BRD_capture(hDev, 0, &mode, _BRDC("REG0"), 10000);
	if (hReg <= 0)
	{
		BRDC_printf(_BRDC("REG NOT capture (%X, %X (%d)\n"), hReg, hDev, mode);
		return -1;
	}
	if (mode == BRDcapt_SHARED)
		BRDC_printf(_BRDC("REG Capture mode: SHARED (%X)\n"), hReg);
	else
	{
		BRDC_printf(_BRDC("REG Capture mode: SPY (%X)\n"), hReg);
		BRD_release(hReg, 0);
		return -1;
	}

	SpdReg(hReg, fname );

	BRD_release(hReg, 0);

	return 0;
}

class CModuleCpp : public CModule
{
public:

	FILE *f;

	virtual S32 RegCtrl( U32 cmd, void *pCmd )
	{
		DEVS_CMD_Reg *pregdata = (DEVS_CMD_Reg*)pCmd;
				
		BRDCHAR* mode = NULL;

		bool spd = false;

		switch( cmd )
		{
		case DEVScmd_REGWRITEDIR:			
			mode = (BRDCHAR*)_BRDC("WD");
			break;		
		case DEVScmd_REGWRITEIND:
			mode = (BRDCHAR*)_BRDC("WI");
			break;		
		case BRDctrl_REG_WRITESPD:			
			mode = (BRDCHAR*)_BRDC("WSPD");
			spd = true;
			break;		
		case BRDctrl_REG_READSPD:
			mode = (BRDCHAR*)_BRDC("RSPD");
			spd = true;
			break;
		}

		if( mode == NULL )
			return 0;

		if( spd )
		{
			BRD_Spd *pregdata = (BRD_Spd*)pCmd;

			BRDC_fprintf( f, _BRDC("%s( 0x%x, 0x%x, 0x%x, 0x%x, 0x%x );\n"), mode, pregdata->tetr, pregdata->dev, pregdata->num , pregdata->reg, pregdata->val );
		}
		else
		{
			DEVS_CMD_Reg *pregdata = (DEVS_CMD_Reg*)pCmd;

			BRDC_fprintf( f, _BRDC("%s( 0x%x, 0x%x );\n"), mode, pregdata->reg, pregdata->val );
		}
		return 0;
	}
};


void SpdRegCpp(  BRDCHAR* fname )
{
	BRDCHAR outname[256];

	

	BRDC_strcpy( outname, fname );
	
	BRDCHAR * ext = BRDC_strstr( outname, _BRDC(".") );	
	
	if( ext )
		BRDC_strcpy( ext, _BRDC(".cpp") );
	
	CModuleCpp module;

	module.f = BRDC_fopen( outname, _BRDC("w") );

	SpdReg( &module , fname );
}

#endif

void GetKey( BRDCHAR *str, BRDCHAR *key, int defval, U32 &outval )
{
	BRDCHAR *pos = BRDC_strstr(str, key );

	outval = defval;
	
	if( pos == NULL )
		return;

	pos += BRDC_strlen( key );

	int retval = 0;

	int ret = BRDC_sscanf( pos, _BRDC("=%i"), &retval );

	if( ret == 1 )
		outval = retval;

}

#define PUSHVALIFNEED() if (strval[0] == _BRDC('$')) { *stack = val; stack++; }

void TOUPPERCASE(BRDCHAR* str)
{
	while (*str)
	{
		*str = BRDC_toupper(*str);
		str++;
	}
}

struct SPD_MY
{
	CModule *pModule;
	U32 trd, dev, fl_32bits, dev_baseadr, num, synchr;
};

#ifdef TCLAPP
int cmdRW(
	ClientData clientData,
	Tcl_Interp *interp,
	int argc,
	char *argv[]
)
{
	char *io = (char*)clientData;
	SPD_MY *my = (SPD_MY*)interp->data;
	DEVS_CMD_Reg regdata;

	char	 *endptr;
	int reg;

	int argi = 1;

	Tcl_GetInt(interp, argv[argi++], &regdata.tetr);

	Tcl_GetInt(interp, argv[argi++], &regdata.reg);

	//U32 val = strtoul(argv[2], &endptr, 0);



	U32 cmd = (io[1] == 'i') ? DEVScmd_REGREADIND : DEVScmd_REGREADDIR;

	if (io[0] == _BRDC('w') || io[0] == _BRDC('W'))
	{
		cmd = (io[1] == 'i') ? DEVScmd_REGWRITEIND : DEVScmd_REGWRITEDIR;
		Tcl_GetInt(interp, argv[argi++], (int*)&regdata.val);
	}

	my->pModule->RegCtrl(cmd, &regdata);

	//val = regdata.val;

	if (io[0] == _BRDC('r') || io[0] == _BRDC('R'))
	{
		char valbuf[32];

		itoa(regdata.val, valbuf, 10);

		Tcl_SetResult(interp, valbuf, NULL);
	}

	return TCL_OK;
}


int cmdSPD(
	ClientData clientData,
	Tcl_Interp *interp,
	int argc,
	char *argv[]
)
{

	SPD_MY *my = (SPD_MY*)interp->data;

	char * val;

	{
		val = Tcl_GetVar2(interp, argv[1], "DEV", TCL_GLOBAL_ONLY);
		Tcl_GetInt(interp, val, (int*)&my->dev);
	}


	{
		val = Tcl_GetVar2(interp, argv[1], "DEV", TCL_GLOBAL_ONLY);
		Tcl_GetInt(interp, val, (int*)&my->dev);
	}


	{
		val = Tcl_GetVar2(interp, argv[1], "DEVBASEADR", TCL_GLOBAL_ONLY);
		Tcl_GetInt(interp, val, (int*)&my->dev_baseadr);
	}


	{
		val = Tcl_GetVar2(interp, argv[1], "32BITS", TCL_GLOBAL_ONLY);
		Tcl_GetInt(interp, val, (int*)&my->fl_32bits);
	}


	{
		val = Tcl_GetVar2(interp, argv[1], "NUM", TCL_GLOBAL_ONLY);
		Tcl_GetInt(interp, val, (int*)&my->num);
	}


	{
		val = Tcl_GetVar2(interp, argv[1], "SYNCHR", TCL_GLOBAL_ONLY);
		Tcl_GetInt(interp, val, (int*)&my->synchr);
	}


	return TCL_OK;
}

int cmdRWSPD(
	ClientData clientData,
	Tcl_Interp *interp,
	int argc,
	char *argv[]
)
{

	SPD_MY *my = (SPD_MY*)interp->data;
	DEVS_CMD_Reg regdata;
	
	char	 *endptr;
	int reg; 

	int argi = 1;


	cmdSPD(clientData, interp, argc, argv);

	Tcl_GetInt(interp, argv[2], &regdata.reg);

	//U32 val = strtoul(argv[2], &endptr, 0);
	
	
	
	

	U32 cmd = BRDctrl_REG_READSPD;

	char *io = (char*)clientData;

	if (io[0] == _BRDC('w') || io[0] == _BRDC('W'))
	{
		cmd = BRDctrl_REG_WRITESPD; 
		Tcl_GetInt(interp, argv[argi++], (int*)&regdata.val);
	}

	BRD_Spd ctrl;

	ctrl.dev = my->dev;
	ctrl.mode = my->fl_32bits;
	ctrl.num = my->num;

	ctrl.sync = my->synchr;

	my->pModule->RegCtrl(cmd, &ctrl);

	if (io[0] == 'r' || io[0] == 'R')
	{
		char valbuf[32];

		itoa(regdata.val, valbuf, 10);

		Tcl_SetResult(interp, valbuf, NULL);
	}

	//val = regdata.val;

	

	return TCL_OK;
}

#endif

// Запись регистров из файла
void SpdReg( CModule *pModule,  BRDCHAR* fname )
{
	BRDC_printf(_BRDC("Programming SPD-registers from file %s\n"), fname);

	BRDCHAR fpath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, fpath);
	BRDC_strcat(fpath, _BRDC("\\") );
	BRDC_strcat(fpath, fname); 
	
	
	SPD_MY cfg = { pModule, 0xFFFF, 0, 0, 0, 0, 0 };

#if TCLAPP
	
	Tcl_Interp *interp = Tcl_CreateInterp();
	interp->data = &cfg;

	Tcl_CmdBuf buffer;

	Tcl_CreateCommand(interp, "_wd", cmdRW, (ClientData)"rd", NULL);
	Tcl_CreateCommand(interp, "_rd", cmdRW, (ClientData)"wd", NULL);

	Tcl_CreateCommand(interp, "_ri", cmdRW, (ClientData)"ri", NULL);
	Tcl_CreateCommand(interp, "_wi", cmdRW, (ClientData)"wd", NULL);

	Tcl_CreateCommand(interp, "_wspd", cmdRWSPD, (ClientData)"w", NULL);
	Tcl_CreateCommand(interp, "_rspd", cmdRWSPD, (ClientData)"r", NULL);
#endif

	FILE *fh = BRDC_fopen(fpath, _BRDC("rt"));
	if (fh == NULL)
	{
		BRDC_printf(_BRDC("Error by file open %s\n"), fpath);
		return;
	}

	BRDCHAR str[256];

	BRDCHAR* begptr;
	BRDCHAR* endptr;

	U32 reg, val;
	BRDCHAR oper[256];
	int ret;
	int fl_enum = 0;

	cfg.dev_baseadr = 0x203;

	U32 _stack[16];
	U32 *stack = _stack;

	do
	{
		BRDCHAR buf[256];

		if( BRDC_fgets(buf, 250, fh )==NULL )
			break;

		BRDC_strcpy(str, buf);

		if( str[0] == _BRDC(';') )
			continue;

		TOUPPERCASE(str);
		
		if(BRDC_strnicmp(str, _BRDC("REM"),3 ) == 0)
		{
			BRDC_printf( str + 4 );
			continue;
		}

		if( str[0] == _BRDC('[') )
		{
			
			ret = BRDC_sscanf( str, _BRDC("[TRD:%i_DEV:%i]"), &cfg.trd, &cfg.dev);
			if(1 == ret)
			{
				BRDC_printf( _BRDC("TRD=%i IND\\DIR\n"), cfg.trd );

				cfg.dev_baseadr = 0x203;
				cfg.num = 0;
				cfg.fl_32bits = -1;
				cfg.synchr = 0;

				continue;
			}
			else if(2 != ret)
			{
				BRDC_printf( _BRDC("Error by section %s\n"), str);
				continue;
			}


			begptr = str+1;
			int len = (int)BRDC_strlen(str);
			str[len-2] = 0;

			fl_enum = 0;
			
			while( BRDC_fgets( str, 250, fh ) )
			{
				TOUPPERCASE(str);

				if(BRDC_strstr(str, _BRDC("#ENUM") ))
				{
					fl_enum = 1;
					break;
				}
								
				GetKey( str, (BRDCHAR*)_BRDC("DEVBASEADR"), cfg.dev_baseadr, cfg.dev_baseadr );
				GetKey( str, (BRDCHAR*)_BRDC("32BITS"), cfg.fl_32bits, cfg.fl_32bits );
				GetKey( str, (BRDCHAR*)_BRDC("NUM"), cfg.num, cfg.num );
				GetKey( str, (BRDCHAR*)_BRDC("SYNCHR"), cfg.synchr, cfg.synchr );
				
				
			}
			
			BRDC_printf( _BRDC("TRD=%d DEV=%d DEVBASEADR=0x%X NUM=0x%X SYNCHR=%d (32BITS=%d)\n"), cfg.trd, cfg.dev, cfg.dev_baseadr, cfg.num, cfg.synchr, cfg.fl_32bits);
			
			continue;
		}
#if TCLAPP
		if (BRDC_strstr(str, _BRDC("#TCL")) )
		{
			char *cmd; 
			
			buffer = Tcl_CreateCmdBuf();
			
			while (BRDC_fgets(str, 250, fh))
			{

				if (BRDC_strstr(str, _BRDC("#END")))
				{
					break;
				}

				char tclstr[256];
				BRDC_bcstombs(tclstr, str, 256);

				cmd = Tcl_AssembleCmd(buffer, tclstr);

				if (cmd == NULL)
					continue;

				Tcl_Eval(interp, cmd, 0, (char **)NULL);
			}

			Tcl_DeleteCmdBuf(buffer);

			continue;
		}
#endif

		if(BRDC_strstr(str, _BRDC("#ENUM") ) && cfg.trd != 0xFFFF)
		{
			fl_enum = 1;
			continue;
		}
		if(BRDC_strstr(str, _BRDC("#END") ))
		{
			cfg.trd = 0xFFFF;
			fl_enum = 0;
			continue;
		}
		
		if (fl_enum != 1)
			continue;

		BRDCHAR strval[256];

		ret = BRDC_sscanf( str, _BRDC("%s %i %s"), oper, &reg, &strval );
		if (3 == ret)
		{
			S32 status;
			DEVS_CMD_Reg regdata;
			regdata.tetr = cfg.trd;

			oper[0] = (oper[0]);

			if ( strval[0] == _BRDC('$') )
			{
				if (oper[0] == _BRDC('W'))
				{
					stack--; 
					val = *stack;
					
				}
			}
			else
			{
				BRDCHAR	 *endptr; 
				val = BRDC_strtoul(strval, &endptr, 0);
			}

			
			if( BRDC_stricmp( oper, _BRDC("SLEEP") ) == 0  )
			{


				BRDC_printf( _BRDC("SLEEP [%d] \n"), reg );
				
				if( (S32)reg == -1 )
					getch( );
				else
					Sleep( reg );
				
				continue;
			}			
			else if (BRDC_stricmp(oper, _BRDC("MASK")) == 0)
			{


				U32 *cur = stack - 1; 
				
				//BRDC_printf(_BRDC("$=0x%x %d \n"), *cur, cur - _stack );

				*cur = (*cur & reg) | (val & (~reg));

				BRDC_printf(_BRDC("MASK [0x%02X,0x%02X]=0x%02X \n"), reg, val, *cur);

				continue;
			}
			else if( BRDC_stricmp( oper, _BRDC("WI") ) == 0  )
			{
				regdata.reg = reg;
				regdata.val = val;
				
				BRDC_printf( _BRDC("WRITE IND REG[0x%02X]=0x%08X \n"), reg, val );
				
				status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
				
				continue;
			}
			else if(  BRDC_stricmp( oper,_BRDC( "RI") ) == 0  )
			{
				regdata.reg = reg;
				status = pModule->RegCtrl( DEVScmd_REGREADIND, &regdata);

				val = regdata.val;
				BRDC_printf(_BRDC("READ IND REG[0x%02X]=0x%04X \n"), reg, val);
				
				PUSHVALIFNEED();

				continue;
			}			
			else if( BRDC_stricmp( oper, _BRDC("WD") ) == 0  )
			{
				regdata.reg = reg;
				regdata.val = val;
				
				BRDC_printf( _BRDC("WRITE DIR REG[0x%02X]=0x%08X \n"), reg, val );
				
				status = pModule->RegCtrl( DEVScmd_REGWRITEDIR, &regdata);
				
				continue;
			}
			else if(  BRDC_stricmp( oper, _BRDC("RD") ) == 0  )
			{
				regdata.reg = reg;
				status = pModule->RegCtrl( DEVScmd_REGREADDIR, &regdata);
				
				val = regdata.val;
				BRDC_printf(_BRDC("READ DIR REG[0x%02X]=0x%04X \n"), reg, val);

				PUSHVALIFNEED();

				continue;
			}



			if( str[0] == _BRDC('w') || str[0] == _BRDC('W') ) {
				if(cfg.fl_32bits)
					BRDC_printf( _BRDC("WRITE REG[0x%02X]=0x%08X \n"), reg, val );
				else
					BRDC_printf( _BRDC("WRITE REG[0x%02X]=0x%04X \n"), reg, val );
			}
//			else
//				printf( "READ  REG[0x%02X]=0x%04X \n", reg, val );
			
//TEST REGSRV ONLY CODE
//#define SPDIO
#if defined(SPDIO) && defined(SPDDEVAPP)


			{

				U32 cmd = BRDctrl_REG_READSPD;
				
				if( str[0] == _BRDC('w') || str[0] == _BRDC('W') )
					cmd = BRDctrl_REG_WRITESPD;

				BRD_Spd ctrl;

				ctrl.dev = dev;
				ctrl.mode = fl_32bits;
				ctrl.num = num;
				
				ctrl.sync = synchr;
				ctrl.tetr = trd;
				ctrl.reg = reg;
				ctrl.val = val;


				status = pModule->RegCtrl( cmd, &ctrl);
				
				if( str[0] == 'r' || str[0] == 'R')
				{
					val = ctrl.val;
					BRDC_printf(_BRDC("READ  REG[0x%02X]=0x%04X \n"), reg, val);
				}

				continue;
			}

#endif

			int cmdrdy;
			

			// ожидаем готовности тетрады
			/*
			regdata.reg = 0;
			do
			{// читаем статус тетрады
				status = pModule->RegCtrl( DEVScmd_REGREADDIR, &regdata);
				cmdrdy = regdata.val & 1;
			}while(!cmdrdy);
			*/

			// выбираем устройство
			regdata.reg = cfg.dev_baseadr; //0x203;//SPD_DEVICE
			regdata.val = cfg.dev;
			status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
			if(!BRD_errcmp(status, BRDerr_OK))
				BRDC_printf(_BRDC("Error operation device (%08X): Tetr = 0x%04X, Reg = 0x%04X\n"), status, regdata.tetr, regdata.reg);
			// записываем адрес
			regdata.reg = cfg.dev_baseadr+2; //0x205;//SPD_ADDR
			regdata.val = reg;
			status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
			if(!BRD_errcmp(status, BRDerr_OK))
				BRDC_printf(_BRDC("Error operation address (%08X): Tetr = 0x%04X, Reg = 0x%04X\n"), status, regdata.tetr, regdata.reg);

			if( str[0] == 'w' || str[0] == 'W')
			{
				// записываем данные
				regdata.reg = cfg.dev_baseadr+3; //0x206;//SPD_DATA
				regdata.val = val & 0xFFFF;
				//printf( "Writing REG[0x%02X]=0x%04X \n", regdata.reg, regdata.val);
				status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
				if(!BRD_errcmp(status, BRDerr_OK))
					BRDC_printf(_BRDC("Error operation data lo (%08X): Tetr = 0x%04X, Reg = 0x%04X\n"), status, regdata.tetr, regdata.reg);
				if(cfg.fl_32bits)
				{
					regdata.reg = cfg.dev_baseadr+4; //0x207;//SPD_DATAH
					regdata.val = val >> 16;
					//printf( "Writing REG[0x%02X]=0x%04X \n", regdata.reg, regdata.val);
					status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
					if(!BRD_errcmp(status, BRDerr_OK))
						BRDC_printf(_BRDC("Error operation data hi (%08X): Tetr = 0x%04X, Reg = 0x%04X\n"), status, regdata.tetr, regdata.reg);
				}
				regdata.val = (cfg.synchr << 12) | (cfg.num << 4) | 0x2; // write
			}
			else
			{
				regdata.val = (cfg.synchr << 12) | (cfg.num << 4) | 0x1; // read
			}

			regdata.reg = cfg.dev_baseadr+1; //0x204;//SPD_CTRL
			status = pModule->RegCtrl( DEVScmd_REGWRITEIND, &regdata);
			if(!BRD_errcmp(status, BRDerr_OK))
				BRDC_printf(_BRDC("Error operation: Tetr = 0x%04X, Reg = 0x%04X\n"), regdata.tetr, regdata.reg);

			// ожидаем готовности тетрады
			/*
			regdata.reg = 0;
			do
			{// читаем статус тетрады
				status = pModule->RegCtrl( DEVScmd_REGREADDIR, &regdata);
				cmdrdy = regdata.val & 1;
			}while(!cmdrdy);
			*/

			if( str[0] == 'r' || str[0] == 'R')
			{
				// считываем данные
				regdata.reg = cfg.dev_baseadr+3; //0x206;//SPD_DATA
				status = pModule->RegCtrl( DEVScmd_REGREADIND, &regdata);
				if(!BRD_errcmp(status, BRDerr_OK))
					BRDC_printf(_BRDC("Error operation: Tetr = 0x%04X, Reg = 0x%04X\n"), regdata.tetr, regdata.reg);
				else
				{
					val = regdata.val;
					if(cfg.fl_32bits)
					{
						regdata.reg = cfg.dev_baseadr+4; //0x207;//SPD_DATAH
						status = pModule->RegCtrl( DEVScmd_REGREADIND, &regdata);
						if(!BRD_errcmp(status, BRDerr_OK))
							BRDC_printf(_BRDC("Error operation: Tetr = 0x%04X, Reg = 0x%04X\n"), regdata.tetr, regdata.reg);
						else
						{
							val |= regdata.val << 16;
							BRDC_printf(_BRDC("READ  REG[0x%02X]=0x%08X \n"), reg, val);
						}
					}
					else
						BRDC_printf(_BRDC("READ  REG[0x%02X]=0x%04X \n"), reg, val);

					PUSHVALIFNEED();
				}
			}
		}

	}while(1);
	fclose(fh);
#if TCLAPP
	Tcl_DeleteInterp(interp);
#endif

}

