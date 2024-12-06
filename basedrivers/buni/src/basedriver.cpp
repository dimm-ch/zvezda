// ----- include files -------------------------
#include	<malloc.h>
#include	<stdio.h>
//#include	<conio.h>
#include	<string.h>
#include	<ctype.h>
//#include	<dos.h>
#include	<time.h>
//#include	<windows.h>
//#include	<winioctl.h>

//#define	DEV_API		DllExport
//#define	DEVS_API	DllExport

#include	"utypes.h"
#include	"brdapi.h"
#include	"ctpapi.h"
#include	"dbprintf.h"
#include	"buni.h"
#include	"extn.h"
#include	"tech.h"
#include	"basedriver.h"

//
//==== Types
//

#define DEFINE_CMD( func ) { *list++ = horrible_cast<DriverCmdMember>(&BaseDriver::func); }
#define DEFINE_SIDE( func ) { *list++ = horrible_cast<DriverCmdMember>(&BaseDriver::func); }

CmdVector<DEVcmd_MAX> BaseDriver::bmap((void*)bentryConstructor);
CmdVector<DEVScmd_MAX> BaseDriver::smap((void*)sentryConstructor);


BaseDriver::BaseDriver( )
{
	_magic = MAGIC;

	BRDC_sprintf( boardName, _BRDC("BUNI") );
}

BaseDriver::~BaseDriver( )
{
	_magic = 0;
}

void BaseDriver::sentryConstructor(DriverCmdMember *list)
{
	U32 cmd = 0;

	DEFINE_SIDE( REGREADDIR    );
	DEFINE_SIDE( REGREADSDIR   );
	DEFINE_SIDE( REGREADIND    );
	DEFINE_SIDE( REGREADSIND   );
	DEFINE_SIDE( REGWRITEDIR   );
	DEFINE_SIDE( REGWRITESDIR  );
	DEFINE_SIDE( REGWRITEIND   );
	DEFINE_SIDE( REGWRITESIND  );

}

void BaseDriver::bentryConstructor(DriverCmdMember *list)
{
	U32 cmd = 0;

	DEFINE_CMD(init );
	DEFINE_CMD(cleanup );
	DEFINE_CMD(reinit );
	DEFINE_CMD(checkExistance );
	DEFINE_CMD(open );
	DEFINE_CMD(close );
	DEFINE_CMD(getinfo );
	DEFINE_CMD(load         );
	DEFINE_CMD(puLoad       );
	DEFINE_CMD(puState      );
	DEFINE_CMD(puList       );
	DEFINE_CMD(reset        );
	DEFINE_CMD(start        );
	DEFINE_CMD(stop         );
	DEFINE_CMD(symbol       );
	DEFINE_CMD(version      );
	DEFINE_CMD(capture      );
	DEFINE_CMD(release      );
	DEFINE_CMD(ctrl         );
	DEFINE_CMD(extension    );
	DEFINE_CMD(peek         );
	DEFINE_CMD(poke         );
	DEFINE_CMD(readRAM      );
	DEFINE_CMD(writeRAM     );
	DEFINE_CMD(readFIFO     );		
	DEFINE_CMD(writeFIFO    );
	DEFINE_CMD(readDPRAM    );
	DEFINE_CMD(writeDPRAM   );
	DEFINE_CMD(read         );
	DEFINE_CMD(write        );
	DEFINE_CMD(getMsg       );
	DEFINE_CMD(putMsg       );
	DEFINE_CMD(signalSend   );
	DEFINE_CMD(signalGrab   );
	DEFINE_CMD(signalWait   );
	DEFINE_CMD(signalIack   );
	DEFINE_CMD(signalFresh  );
	DEFINE_CMD(signalInfo   );
	DEFINE_CMD(signalList   );
	DEFINE_CMD(devHardwareInit    );
	DEFINE_CMD(devHardwareCleanup );
	DEFINE_CMD(subHardwareInit    );
	DEFINE_CMD(subHardwareCleanup );
	DEFINE_CMD(getServList );
	DEFINE_CMD(puRead      );
	DEFINE_CMD(puWrite     );


}

S32 BaseDriver::entry( S32 cmd, void *pParam )
{
	
	DriverCmdMember pm = horrible_cast<DriverCmdMember>(bmap.get(cmd));
	

	
	if( pm == NULL )
		return BRDerr_CMD_UNSUPPORTED;

	return ((BaseDriver*)this->*pm)( pParam );
}

S32 BaseDriver::sentry( S32 cmd, void *pParam )
{
	DriverCmdMember pm = horrible_cast<DriverCmdMember>(smap.get(cmd));
	

	
	if( pm == NULL )
		return BRDerr_CMD_UNSUPPORTED;

	return ((BaseDriver*)this->*pm)( pParam );
}

//=********************** CMD_init ******************
//=*****************************************************
S32 BaseDriver::init(  DEV_CMD_Init *pParam )
{
	    //
		//=== Dummy fuction
		//
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_cleanup ******************
//=*****************************************************
S32 BaseDriver::cleanup(  DEV_CMD_Cleanup *pParam )
{   
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_reinit ******************
// используется для устройств с "горячей" заменой
//=*****************************************************
S32 BaseDriver::reinit(  DEV_CMD_Reinit *pParam )
{
		//
		//=== Dummy fuction
		//
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_checkExistance ******************
// Функция проверяет, что устройство уже используется
//=*****************************************************
S32 BaseDriver::checkExistance( DEV_CMD_CheckExistance *pParam )
{
    

	//
	//=== TODO: If Board is in Work, return 1.
	//
    *(pParam->state) = 1;

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_open ******************
//=*****************************************************
S32 BaseDriver::open( DEV_CMD_Open *ptr)
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}

//=********************** CMD_close ******************
//=*****************************************************
S32 BaseDriver::close(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}


//=********************** CMD_getinfo ******************
//=*****************************************************
S32 BaseDriver::getinfo(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_load ******************
//=*****************************************************
S32 BaseDriver::load(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_puLoad *******************
//=*****************************************************
S32 BaseDriver::puLoad(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_puState ******************
//=*****************************************************
S32 BaseDriver::puState(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_puList *******************
//=*****************************************************
S32 BaseDriver::puList(  DEV_CMD_PuList *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}


//=********************** CMD_puRead *******************
//=*****************************************************
S32 BaseDriver::puRead(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}


//=********************** CMD_puWrite ******************
//=*****************************************************
S32 BaseDriver::puWrite(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}


//=********************** CMD_reset ******************
//=*****************************************************
S32 BaseDriver::reset(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_start ********************
//=*****************************************************
S32 BaseDriver::start(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_stop *********************
//=*****************************************************
S32 BaseDriver::stop(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_capture ******************
//=*****************************************************
S32 BaseDriver::capture(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_release ******************
//=*****************************************************
S32 BaseDriver::release(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_ctrl *********************
//=*****************************************************
S32 BaseDriver::ctrl(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_extension ****************
//  Дополнительные функции - общие для всех модулей
//  и уникальные для конкретного модуля
//=*****************************************************
S32 BaseDriver::extension(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_symbol *******************
// Функция возвращает адрес символьной переменной в исполняемом файле для ЦОС.
// Исполняемый файл должен содержать символьную информацию
//=*****************************************************
S32 BaseDriver::symbol( void *pParam) 
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_version ******************
//=*****************************************************
S32 BaseDriver::version(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_peek *********************
// чтение ячейки памяти из модуля ЦОС
//=*****************************************************
S32  BaseDriver::peek( void *pParam)
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_poke *********************
// запись ячейки памяти в модуле ЦОС
//=*****************************************************
S32  BaseDriver::poke(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_readRAM ******************
// чтение блока памяти из модуля ЦОС
//=*****************************************************
S32 BaseDriver::readRAM(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_writeRAM ******************
// запись блока памяти в модуле ЦОС
//=*****************************************************
S32 BaseDriver::writeRAM(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_readFIFO ******************
//=*****************************************************
S32 BaseDriver::readFIFO(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_writeFIFO ******************
//=*****************************************************
S32 BaseDriver::writeFIFO(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_readDPRAM ******************
//=*****************************************************
S32 BaseDriver::readDPRAM(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_writeDPRAM ******************
//=*****************************************************
S32 BaseDriver::writeDPRAM(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_read ******************
//=*****************************************************
S32 BaseDriver::read(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_write ******************
//=*****************************************************
S32 BaseDriver::write(  void *pParam )
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_GetMsg *******************
//=*****************************************************
S32 BaseDriver::getMsg(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}




//=********************** CMD_putMsg *******************
//=*****************************************************
S32 BaseDriver::putMsg(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}





//=********************** CMD_signalSend ******************
//=********************************************************
S32 BaseDriver::signalSend(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_signalGrab ******************
//=*****************************************************
S32 BaseDriver::signalGrab(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_signalWait ******************
//=*****************************************************
S32 BaseDriver::signalWait(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_signalIack ******************
//=*****************************************************
S32 BaseDriver::signalIack(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_signalFresh *****************
//=*****************************************************
S32 BaseDriver::signalFresh(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_signalInfo ******************
//=*****************************************************
S32 BaseDriver::signalInfo(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}


//=********************** CMD_signalList ***************
//=*****************************************************
S32 BaseDriver::signalList(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}



//=********************** CMD_devHardwareInit **********
//=*****************************************************
S32 BaseDriver::devHardwareInit( void *pParam)
{
    return DRVERR(BRDerr_OK);
}

//=********************** CMD_devHardwareCleanup *******
//=*****************************************************
S32 BaseDriver::devHardwareCleanup( void *pParam)
{
    //DEV_CMD_DevHardwareCleanup  *ptr = (DEV_CMD_DevHardwareCleanup*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareInit **********
//=*****************************************************
S32 BaseDriver::subHardwareInit( void *pParam)
{
    //DEV_CMD_SubHardwareInit *ptr = (DEV_CMD_SubHardwareInit*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_subHardwareCleanup *******
//=*****************************************************
S32 BaseDriver::subHardwareCleanup(  void *pParam )
{
    //DEV_CMD_SubHardwareCleanup  *ptr = (DEV_CMD_SubHardwareCleanup*)pParam;

    //
    // TODO: Make operation
    //

    return DRVERR(BRDerr_OK);
}

//=********************** CMD_getServList ***************
//=*****************************************************
S32 BaseDriver::getServList(  void *pParam )
{
	return DRVERR(BRDerr_FUNC_UNIMPLEMENTED);
}

S32  BaseDriver::REGREADDIR       (  DEVS_CMD_Reg *pParam )
{
	 return DRVERR(BRDerr_OK);
}

S32  BaseDriver::REGREADSDIR       (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}

S32  BaseDriver::REGREADIND      (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}
S32  BaseDriver::REGREADSIND       (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}
S32  BaseDriver::REGWRITEDIR       (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}
S32  BaseDriver::REGWRITESDIR       (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}
S32  BaseDriver::REGWRITEIND       (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}
S32  BaseDriver::REGWRITESIND       (  DEVS_CMD_Reg *pParam )
{
		 return DRVERR(BRDerr_OK);
}


//
// End of file
//
