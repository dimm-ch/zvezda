#include "brd.h"
#include "extn.h"
#include "gipcy.h"
#include "utypes.h"
#include <csignal>
#include "ctrlfm415s.h"

const auto SPD_DEVICE = 0x203;
const auto SPD_CTRL = 0x204;
const auto SPD_ADDR = 0x205;
const auto SPD_DATAL = 0x206;

const U32 FM415S_TETR_ID = 0x135;

enum
{
	TEST_ERR_NO_INI				= -1,
	TEST_ERR_BRD_INI			= -2,
	TEST_ERR_LIDLIST			= -3,
	TEST_ERR_NO_LIDS			= -4,
	TEST_ERR_LID_NOT_FOUND		= -5,
	TEST_ERR_OPEN				= -6,
	TEST_ERR_SERVICELIST		= -7,
	TEST_ERR_NO_SERVICES		= -8,
	TEST_ERR_NO_DEVREG			= -9,
	TEST_ERR_NO_415S			= -10,
	TEST_ERR_NO_MAIN			= -11,
	TEST_ERR_NO_QSFP			= -12,
	TEST_ERR_NO_PLL_LOCK		= -13,
	TEST_ERR_NO_LINE_UP			= -14,
	TEST_ERR_NO_CHAN_UP			= -15,
	TEST_ERR_CHAN_MASK			= -16,
};

BRDCHAR g_sIniFileName[MAX_PATH];
U32		g_nLid[2];
bool	g_isTStudio;
U32		g_nChanMask;
//U32		g_nDirChanMask; //0-Tx, 1-Rx
//U32		g_nStreamChan;
U32		g_nDecim;
double	g_dRate;
U32		g_nTestType;	//0-256 разрядная последовательность, 1-64 разрядный счётчик

void ParseCommandLine(int argc, BRDCHAR *argv[]);
S32 ParseIniFile(BRDCHAR *filename);
