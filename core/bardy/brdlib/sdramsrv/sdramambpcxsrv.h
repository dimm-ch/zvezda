/*
 ****************** File SdramAmbpcxSrv.h *************************
 *
 *  Definitions of user application interface
 *	structures and constants
 *	for BRD_ctrl : SdramAmbpcx section
 *
 * (C) InSys by Dorokhin Andrey Feb, 2005
 *
 ************************************************************
*/

#ifndef _SDRAMAMBPCXSRV_H
 #define _SDRAMAMBPCXSRV_H

#include "ctrlsdram.h"
#include "service.h"
#include "adm2if.h"
#include "mainregs.h"
#include "sdramregs.h"
#include "sdramsrv.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!! Warning change in fact!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
const int SDRAMAMBPCX_TETR_ID = 0x25; // tetrad id

class CSdramAmbpcxSrv : public CSdramSrv
{

protected:

	virtual void GetSdramTetrNum(CModule* pModule);
	
public:

	CSdramAmbpcxSrv(int idx, int srv_num, CModule* pModule, PSDRAMSRV_CFG pCfg); // constructor

};

#endif // _SDRAMAMBPCDSRV_H

//
// End of file
//