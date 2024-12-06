/*
 ****************** File adcdacsubmodule.h ************************
 *
 * Definitions of structures and constants
 * of derived class CAdcDacSubModule for Data Acquisition Boards.
 *
 * (C) InSys by Dorokhin A. Jun. 2007
 *
 *
 *********************************************************
*/

#ifndef _ADCDACSUBMODULE_H_
 #define _ADCDACSUBMODULE_H_

#include	"submodule.h"
#include	"adcsrv.h"
#include	"dacsrv.h"

// CAdcSubModule base class definition
class CAdcDacSubModule : public CSubModule
{

protected:

	void GetAdcIni(PADC_CFG pAdcIni, PINIT_Data pInitData, long size);
	ULONG SetAdcConfig(PADC_CFG pAdcCfg, PICR_CfgAdc pAdc, PICR_CfgAdcFifo pAdcFifo, int isTag, PADC_CFG pAdcIni);
	void SetAdcCfg(PADCSRV_CFG pAdcDst, PADC_CFG pAdcSrc);

	void GetDacIni(PDAC_CFG pDacIni, PINIT_Data pInitData, long size);
	ULONG SetDacConfig(PDAC_CFG pDacCfg, PICR_CfgAdmDac pDac, PICR_CfgDacFifo pDacFifo, int isTag, PDAC_CFG pDacIni);
	void SetDacCfg(PDACSRV_CFG pDacDst, PDAC_CFG pDacSrc);

  	const BRDCHAR* getClassName(void) { return _BRDC("CAdcDacSubModule"); }

public:

	CAdcDacSubModule(); // constructor
	CAdcDacSubModule(PBRD_InitData pBrdInitData, long sizeInitData); // one more constructor
	//virtual ~CSubModule(); // destructor

};

#endif	// _ADCDACSUBMODULE_H_

// ****************** End of file adcdacsubmodule.h **********************


 