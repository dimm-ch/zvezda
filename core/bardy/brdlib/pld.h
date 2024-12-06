///////////////////////////////////////////////////////////////////////
//	PLD.H - PLD difinitions
//	Copyright (c) 2001, Instrumental Systems,Corp.
///////////////////////////////////////////////////////////////////////

#ifndef _PLD_H_
#define _PLD_H_

#ifndef EOS
	#define EOS ('\0')
#endif
//#ifndef EOL
//	#define EOL ("\r\n")
//#endif

#define PLD_MAXDATASIZE 256
#define PLD_HEADERSIZE 9

typedef enum _PLD_MODES {
  PLD_modePROGR,
  PLD_modeGETSTAT,
  PLD_modeVPROGR	= 0x101, // сброс конфигурации ПЛИС
  PLD_modeVPROGR_EN	= 0x201, // разрешение записи VPROG
  PLD_modeVCLK		= 0x401  // формирование 1-го сигнала VCLK
} PLD_MODES;

typedef enum _PldRecordTypes {
  PLD_rectDATA,
  PLD_rectEND
} PldRecordTypes;

#pragma pack(1)

typedef struct _PLD_RECORD {
  UCHAR  len;
  USHORT addr;
  UCHAR  type;
  UCHAR  data[PLD_MAXDATASIZE];
} PLD_RECORD, *PPLD_RECORD;

#pragma pack()

#define PLD_MAXRECORDSIZE (sizeof(PLD_RECORD) + 4)

// PLD error type
enum {
	PLD_errOK,
	PLD_errFORMAT,		// file format or check sum error
	PLD_errROM,			// PLD programming from ROM only 
	PLD_errNOTRDY,		// PLD not ready for programming 
	PLD_errPROG			// Programming error
};
#endif //_PLD_H_
