/*
 ****************** File dacsubmodule.h ************************
 *
 * Definitions of structures and constants
 * of derived class CDacSubModule for Data Acquisition Boards.
 *
 * (C) InSys by Dorokhin A. Jun. 2007
 *
 *
 *********************************************************
*/

#ifndef _DACSUBMODULE_H_
 #define _DACSUBMODULE_H_

#include	"submodule.h"
#include	"dacsrv.h"

#pragma pack(push,1)

//typedef struct _DAC_INI {
//	U32	Bits;		// разрядность
//	U32	Encoding;	// тип кодировки (0 - прямой, 1 - двоично-дополнительный, 2 - код Грея)
//	U32	MinRate;	// минимальная частота дискретизации
//	U32	MaxRate;	// максимальная частота дискретизации
//	U32	OutRange;	// выходной диапазон
//	U32	FifoSize;	// размер FIFO (в байтах)
//} DAC_INI, *PDAC_INI;

#pragma pack(pop)

// CDacSubModule base class definition
class CDacSubModule : public CSubModule
{

protected:

//	void GetDacIni(PDAC_INI pDacIni, PINIT_Data pInitData, long size);
	void GetDacIni(PDAC_CFG pDacIni, PINIT_Data pInitData, long size);
//	ULONG SetDacConfig(PDAC_CFG pDacCfg, PICR_CfgDac pDac, PICR_CfgDacFifo pDacFifo, int isTag, PDAC_INI pDacIni);
	ULONG SetDacConfig(PDAC_CFG pDacCfg, PICR_CfgAdmDac pDac, PICR_CfgDacFifo pDacFifo, int isTag, PDAC_CFG pDacIni);
	void SetDacCfg(PDACSRV_CFG pDacDst, PDAC_CFG pDacSrc);

  	const BRDCHAR* getClassName(void) { return _BRDC("CDacSubModule"); }

public:

	CDacSubModule(); // constructor
	CDacSubModule(PBRD_InitData pBrdInitData, long sizeInitData); // one more constructor
	//virtual ~CSubModule(); // destructor

};

#endif	// _SUBMODULE_H_

// ****************** End of file dacsubmodule.h **********************


 