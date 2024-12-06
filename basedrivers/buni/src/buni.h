/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== b101E.h ========
 *  B101E BASE Driver Declaration for Boards: 
 *		ADP101E
 *
 *  version 1.0
 ********************************************************************/

#ifndef	__B101E_H_
#define	__B101E_H_

#include	"brdapi.h"
#include	"baseserv.h"
#include	"basestream.h"
#include	"tech.h"
#include	"array.h"
#include	"basedriver.h"

//
//==== Constants & Macros
//

#define	CURRFILE		_BRDC("Buni")
#define	btB101			0x101
#define	VER_MAJOR		0x00000000L
#define	VER_MINOR		0x00000001L

#define	BOOT_HOST  0x0
#define	BOOT_EPROM 0x4
#define	BOOT_SLAVE 0x10
#define	BOOT_LINK  0x8

#define	STREAM_MAX  2

enum    BOARD_TYPES {
        UNDEFINED = 0,
        ADP101N = 1,
};

enum { szNULL=0, szU08=1, szU16=2, szU32=4 };

//
//==== Types
//


// Define Geniric Device Extension Type BRDDRV_Board
class BUNI2Driver : public BaseDriver
{
public:

	U32			brdId;					// LID of net board
	U32			ipPort;
	
	BRDCHAR			ipAddr[16];

	

	HANDLE		pCTPHost;					// Handle if Use
	HANDLE		pCTPDev;					// Handle if Use
	
	SINT32		irq;					// IRQ line number 0-15 

	U32			deviceId;				// device Id
	U32			revision;				// board revision

	U32			argsAdr,  argsSize;		// .args Segment Adr and Size 

	SINT32		errcode; 		// Driver Last Error Number 
	SINT32		errcnt;	 		// Driver Error Counter 

	U32							baseServListSize;	// Size of Service List

	SERV_ServiceCollection		*pCollServ;		// Base Service List

	//	USER MEMBERS

	U32			BOARD;			// Board type
	U32			BOOT;			// BOOT Mode

	U32			SizeOfFIFO0;	// size of FIFO0 (HOST to DSP)
	U32			SizeOfFIFO1;	// size of FIFO1 (DSP to HOST)
	//////////////////

	BRD_BaseStrm *apBaseStrm[STREAM_MAX];

	BUNI2Driver(  );
	~BUNI2Driver();

	void fillServList( TDict *pDict );
	void loadSide(BRDCHAR *type, TDict *dict );
	
	S32 readIcr( void *pData, U32 tag, U32 size,U32* sizeOut=NULL );

protected:

	TArray<TModuleProxy*> _list;
	
	// public service list
	TArray<TSrv*> _servList;
	

	//hidden service list
	TArray<TSrv*> _loadedServList;

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

	TSrv* findServ(BRDCHAR *name );
};


typedef BUNI2Driver BRDDRV_Board ;


typedef struct 
{
	BRDCHAR   *host;
	U32		ipport;					// Data from Registry or INI FileU32		
	U32		brdId;					// Data from Registry or INI File
} TIniData;

typedef struct { UINT32 adr, val, size; }		   TIEepromWrite;



//
//==== Global Variables
//

//
//==== Internal Functions
//

//
// B101E.CPP
//

S32		Dev_ErrorPrintf( S32 errorCode, void *ptr, BRDCHAR *title, BRDCHAR *format, ... );
void	Dev_Printf( S32 errLevel, BRDCHAR *title, BRDCHAR *format, ... );
S32		Dev_CheckSignature( U32 pid );
S32     Dev_AllocBoard( void **ppDev, HANDLE hWDM, char *pBoardName );
S32     Dev_FreeBoard( BRDDRV_Board *pDev );
S32     Dev_ParseBoard( TIniData *pIniData, void **ppDev, BRDCHAR *pBoardName, S32 idxThisBrd );

enum {
	verWindows95 = 95,
	verWindows98 = 98,
	verWindowsME = 1000,
	verWindowsNT351 = 351,
	verWindowsNT4 = 400,
	verWindows2000 = 2000,
	verWindowsXP = 0x314,
	verWindowsUnknow = -1
};
S32	GetWinVersion(void);


//
// and also the standard entry points:
//



#endif	// __B101E_H_

//
// End of File
//


 
