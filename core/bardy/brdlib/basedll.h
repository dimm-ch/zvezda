/*
 ****************** File basedll.h ************************
 *
 * (C) InSys by Dorokhin A. Mar 2004
 *
 ******************************************************
*/

#ifndef _BASEDLL_H_
 #define _BASEDLL_H_

#ifdef _AMBVME
#include "bambvme.h"
#define BASE_CLASS CAmbvme
#endif	// _PLXPCI

#ifdef _AMBPCX
#include "bambpci.h"
#define BASE_CLASS CBambpci
#endif	// _AMBPCX

#ifdef _ADS10X2G
#include "s10x2g.h"
#define BASE_CLASS Cs10x2g
#endif	// _ADS10X2G

#ifdef _AMB3US
#include "bamb3us.h"
#define BASE_CLASS CBamb3us
#endif	// _AMB3US

#ifdef _AMB3UV
#include "bamb3uv.h"
#define BASE_CLASS CBamb3uv
#endif	// _AMB3UV

#ifdef _AMB3UVF
#include "bamb3uvf.h"
#define BASE_CLASS CBamb3uvf
#endif	// _AMB3UVF

#ifdef _AMBPEX
#include "bambpex.h"
#define BASE_CLASS CBambpex
#endif	// _AMBPEX

#ifdef _SYNC8CHPCI
#include "Sync8chPCI.h"
#define BASE_CLASS CSync8chPCI
#endif	// _SYNC8CHPCI

#ifdef _PLXPCI
#include "bplx.h"
#define BASE_CLASS CBplx
#endif	// _PLXPCI

#ifdef _AMBUSB
#include "bambusb.h"
#define BASE_CLASS CBambusb
#endif
#ifdef _ZYNQ
#include "bzynq.h"
#define BASE_CLASS CBzynq
#endif	// _ZYNQ

#ifdef _AXIZYNQ
#include "baxizynq.h"
#define BASE_CLASS CAxizynq
#endif	// _AXIZYNQ

//#ifdef _BASEINC
//#include "basedriver.h"
//#endif	// _BASEINC

#endif	// _BASEDLL_H_

// ****************** End of file basedll.h **********************


