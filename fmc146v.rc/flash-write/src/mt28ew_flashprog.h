#ifndef M25EW_FLASHPROG_H
#define M25EW_FLASHPROG_H

#include	"brd_dev.h"
#include	"brderr.h"
#include    "ctrlreg.h"
#include	"utypes.h"

#ifndef __linux__
#include "isdll.h"
#endif

#define TRD_MT28EW_ID 0x10E 

#define IS_ID 1
#define IS_RD 2
#define IS_WR 4
#define IS_BE 8
#define IS_FE 16
#define IS_IPROG 32
#define IS_BC 64

#define MAXBUFSIZE 512
#define TRDREG_STATUS 0
#define TRDREG_DATA 1
#define TRD_MAIN 0

#define TRDIND_MODE0	0
#define TRDIND_MODE2	0xA
#define TRDIND_DATA		0x200
#define TRDIND_ADDR_L	0x201
#define TRDIND_ADDR_H	0x202

// Error Param
typedef struct
{
	U32		Addr;
	bool	isErr[128];
	U16		exp_val[128];
	U16		rec_val[128];
	U32		Words;
} ErrorParam;

ErrorParam g_ErrorParam[16];

union AddrUnion
{
	U32 x32;
	U16 x16[2];
} ;

typedef struct
{
	AddrUnion	Addr;
	U16			WordsArr[MAXBUFSIZE];
	U32			Words;
} DataParam;

int MainLoop (void);
void DisplayUsageMsg(void);
BRD_Handle  GetSrvHandle(const BRDCHAR* name, U32 *atr);

U08 Crc(U08 *pcBlock, U08 len);

//int trd_peek_ind (U32 trd_flash, U32 trdAddr);
//int trd_poke_ind (U32 trd_flash, U32 trdAddr, U32 trdData);
//int trd_poke_dir (U32 trd_flash, U32 trdRegNo, void *trdData, U32 size_byte);
//int trd_peek_dir (U32 trd_flash, U32 trdRegNo, void *trdData, U32 size_byte);
//int trd_status (U32 trd_flash);

#ifndef __linux__
HANDLE	hConsoleOut;
#endif

U32 GetFileSize(void);
void ReadID(brd_dev_t dev);
void ChipErase(brd_dev_t dev);
void BlockErase(brd_dev_t dev);
void BlankCheck(brd_dev_t dev);
void WriteData(brd_dev_t dev,U32 fsize);
void ReadData(brd_dev_t dev,U32 fsize);
void IprogCmd(brd_dev_t dev, U32 trd_flash);

#ifndef __linux__
HANDLE	handle;		// Driver Handle

typedef struct _DEV_DESCR {
	PCI_DEVICE	pciDev;				// PCI device
	ULONG	cfg_data[64]; // PCI configuration data
} DEV_DESCR, *PDEV_DESCR;

DEV_DESCR* pDevices[16] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
int dev_num = 0;
ULONG	cfg_data[64]; // PCI configuration data
// Function Declarations
int FindDevice(U32 devID, U32 inst);
int	FindBridge(int sec_bus);
int	OffAndOn(void);
#endif

#endif //M25EW_FLASHPROG_H
