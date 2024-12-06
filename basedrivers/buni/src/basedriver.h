
#ifndef __BASEDRV_H__
#define __BASEDRV_H__

#include	"tech.h"
#include	"utypes.h"

#include <map>

#define REALLY_INLINE inline

enum
{
	DEVScmd_LAST = DEVScmd_PACKEXECUTE,
	DEVScmd_MAX
};

class BaseDriver;

typedef S32 (BaseDriver::*DriverCmdMember)( void *pParam );

template<class T> class CmdMap
{
public:

	CmdMap( void *f )
	{
		typedef void (*fc)( CmdMap *list );

		((fc)f)( this );
	}

	REALLY_INLINE T get( S32 cmd )
	{
		if( _map.find( cmd ) == _map.end() )
			return NULL;

		return _map[cmd];
	}

	void set( S32 cmd, T func )
	{
		_map[cmd]=func;
	}

private:

	std::map<U32,T>  _map;

};

template<int size> class CmdVector
{
public:

	CmdVector( void *f )
	{
		typedef void (*fc)( DriverCmdMember *list );

		((fc)f)( _map );
	}

	REALLY_INLINE DriverCmdMember get( S32 cmd )
	{
		if(cmd < 0 || cmd >= size)   return NULL;
		if(_map[cmd] == NULL)   return NULL;

		return _map[cmd];
	}

private:

	DriverCmdMember  _map[size];

};

class BaseDriver : public TModule
{
public:
	static const U32 MAGIC = 0xFFAABBCC;

	void *operator new(size_t size);
	void operator delete(void *gcObject);

	virtual void Init( )
	{

	}

	virtual S32 GetInfo( BRD_Info *pInfo )
	{
		return 0;
	}

protected:

	U32	_magic;

public:
	BaseDriver( );
	virtual ~BaseDriver( );

	S32 entry( S32 cmd, void *pParam );
	S32 sentry( S32 cmd, void *pParam );

	virtual S32 Ctrl( S32 cmd,void *pParam ) { return entry( cmd, pParam ); };

	//virtual S32 readIcr( void *pData, U32 tag, U32 size,U32* sizeOut=NULL ) abstract;

private:	
	
	static CmdVector<DEVcmd_MAX> bmap;
	static CmdVector<DEVScmd_MAX> smap;
	
	static void bentryConstructor(DriverCmdMember *list);
	static void sentryConstructor(DriverCmdMember *list);

protected:

	//=*************** Prepare DEV_entry *******************
	//=*****************************************************
	virtual S32 init          (  DEV_CMD_Init *pParam );
	virtual S32 cleanup       (  DEV_CMD_Cleanup *pParam );
	virtual S32 reinit        (  DEV_CMD_Reinit *pParam );
	virtual S32 checkExistance(  DEV_CMD_CheckExistance *pParam );
	virtual S32 open          (  DEV_CMD_Open *pParam );
	virtual S32 close         (  void *pParam );
	virtual S32 getinfo       (  void *pParam );
	virtual S32 load          (  void *pParam );
	virtual S32 puLoad        (  void *pParam );
	virtual S32 puState       (  void *pParam );
	virtual S32 puList        (  DEV_CMD_PuList *pParam );
	virtual S32 reset         (  void *pParam );
	virtual S32 start         (  void *pParam );
	virtual S32 stop          (  void *pParam );
	virtual S32 symbol        (  void *pParam );
	virtual S32 version       (  void *pParam );
	virtual S32 capture       (  void *pParam );
	virtual S32 release       (  void *pParam );
	virtual S32 ctrl          (  void *pParam );
	virtual S32 extension     (  void *pParam );
	virtual S32 peek          (  void *pParam );
	virtual S32 poke          (  void *pParam );
	virtual S32 readRAM       (  void *pParam );
	virtual S32 writeRAM      (  void *pParam );
	virtual S32 readFIFO      (  void *pParam );
	virtual S32 writeFIFO     (  void *pParam );
	virtual S32 readDPRAM     (  void *pParam );
	virtual S32 writeDPRAM    (  void *pParam );
	virtual S32 read          (  void *pParam );
	virtual S32 write         (  void *pParam );
	virtual S32 getMsg        (  void *pParam );
	virtual S32 putMsg        (  void *pParam );
	virtual S32 signalSend    (  void *pParam );
	virtual S32 signalGrab    (  void *pParam );
	virtual S32 signalWait    (  void *pParam );
	virtual S32 signalIack    (  void *pParam );
	virtual S32 signalFresh   (  void *pParam );
	virtual S32 signalInfo    (  void *pParam );
	virtual S32 signalList    (  void *pParam );
	virtual S32 devHardwareInit    (  void *pParam );
	virtual S32 devHardwareCleanup (  void *pParam );
	virtual S32 subHardwareInit    (  void *pParam );
	virtual S32 subHardwareCleanup (  void *pParam );
	virtual S32 getServList    (  void *pParam );
	virtual S32 puRead        (  void *pParam );
	virtual S32 puWrite       (  void *pParam );

	virtual S32 REGREADDIR       (  DEVS_CMD_Reg *pParam );
	virtual S32 REGREADSDIR       (  DEVS_CMD_Reg *pParam );
	virtual S32 REGREADIND      (  DEVS_CMD_Reg *pParam );
	virtual S32 REGREADSIND       (  DEVS_CMD_Reg *pParam );
	virtual S32 REGWRITEDIR       (  DEVS_CMD_Reg *pParam );
	virtual S32 REGWRITESDIR       (  DEVS_CMD_Reg *pParam );
	virtual S32 REGWRITEIND       (  DEVS_CMD_Reg *pParam );
	virtual S32 REGWRITESIND       (  DEVS_CMD_Reg *pParam );

	virtual S32 NOP(DEVS_CMD_Reg *pParam) { return 0; };
};



REALLY_INLINE void *BaseDriver::operator new(size_t size)
{
	void *p = (void*)malloc(size);

	memset( p, 0, size );

	return p;
}

REALLY_INLINE void BaseDriver::operator delete(void *pDev)
{
	free( pDev );
}

#endif