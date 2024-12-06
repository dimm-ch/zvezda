/*
 ****************** File sidedll.h ************************
 *
 * (C) InSys by Dorokhin A. Mar 2004
 *
 ******************************************************
*/

#ifndef _SIDEDLL_H_
 #define _SIDEDLL_H_

#ifdef _DSP201
#include "dsp201.h"
#define SIDE_CLASS CDsp201
#endif	// _DSP201

#ifdef _DEVREG
#include "devreg.h"
#define SIDE_CLASS CDevReg
#endif	// _DEVREG

#ifdef _TESTEX
#include "testex.h"
#define SIDE_CLASS CTestEx
#endif	// _TESTEX

#ifdef _BASE2ADM
#include "base2adm.h"
#define SIDE_CLASS CBase2Adm
#endif	// _BASE2ADM

#ifdef _BASERES
#include "baseRes.h"
#define SIDE_CLASS CBaseRes
#endif	// _BASERES

#ifdef _BASE2FMC
#include "base2fmc.h"
#define SIDE_CLASS CBase2fmc
#endif	// _BASE2FMC

#ifdef _DSPNODE
#include "dspnode.h"
#define SIDE_CLASS CDspNode
#endif	// _DSPNODE

#ifdef _DEMOD
#include "demod.h"
#define SIDE_CLASS CDemod
#endif	// _DEMOD

#ifdef _MADM
#include "madm.h"
#define SIDE_CLASS CAdm
#endif	// _MADM

#ifdef _MDIO2
#include "mdio2.h"
#define SIDE_CLASS Cmdio2
#endif	// _MDIO2

#ifdef _MDIO32
#include "mdio32.h"
#define SIDE_CLASS CAdmDio32
#endif	// _MDIO32

#ifdef _M212x200M
#include "m212x200m.h"
#define SIDE_CLASS Cm212x200m
#endif	// _M212x200M

#ifdef _M28x800M
#include "m28x800m.h"
#define SIDE_CLASS Cm28x800m
#endif	// _M28x800M

#ifdef _M3224x192
#include "m3224x192.h"
#define SIDE_CLASS Cm3224x192
#endif	// _M3224x192

#ifdef _MDDC4x16
#include "mddc4x16.h"
#define SIDE_CLASS Cmddc4x16
#endif	// _MDDC4x16

#ifdef _MDIOV
#include "mdiov.h"
#define SIDE_CLASS CAdmDiov
#endif	// _MDIOV

#ifdef _MFOTR
#include "mfotr.h"
#define SIDE_CLASS CAdmFotr
#endif	// _MFOTR

#ifdef _M414x65M
#include "m414x65m.h"
#define SIDE_CLASS Cm414x65m
#endif	// _M414x65M

#ifdef _M214x200M
#include "m214x200m.h"
#define SIDE_CLASS Cm214x200m
#endif	// _M214x200M

#ifdef _M214x400M 
#include "m214x400m.h"
#define SIDE_CLASS Cm214x400m
#endif	// _M214x400M

#ifdef _M28x1G
#include "m28x1g.h"
#define SIDE_CLASS Cm28x1g
#endif	// _M28x1G

#ifdef _M216x100M
#include "m216x100m.h"
#define SIDE_CLASS Cm216x100m
#endif	// _M216x100M

#ifdef _MQM9857
#include "mqm9857.h"
#define SIDE_CLASS Cmqm9857
#endif	// _MQM9857

#ifdef _MGC5016
#include "mgc5016.h"
#define SIDE_CLASS Cmgc5016
#endif	// _MGC5016

#ifdef _M1624x192
#include "m1624x192.h"
#define SIDE_CLASS Cm1624x192
#endif	// _M1624x192

#ifdef _MDAC216x400M
#include "mdac216x400m.h"
#define SIDE_CLASS Cmdac216x400m
#endif	// _MDAC216x400M

#ifdef _MDAC216x400MT
#include "mdac216x400mt.h"
#define SIDE_CLASS Cmdac216x400mt
#endif	// _MDAC216x400MT

#ifdef _M818X800
#include "m818x800.h"
#define SIDE_CLASS Cm818X800
#endif	// _M818x800

#ifdef _M214x10M
#include "m214x10m.h"
#define SIDE_CLASS Cm214x10m
#endif	// _M214x10M

#ifdef _M1612x1M
#include "m1612x1m.h"
#define SIDE_CLASS Cm1612x1m
#endif	// _M1612x1M

#ifdef _M210x1G
#include "m210x1g.h"
#define SIDE_CLASS Cm210x1g
#endif	// _M210x1G

#ifdef _M212x1G
#include "m212x1g.h"
#define SIDE_CLASS Cm212x1g
#endif	// _M212x1G

#ifdef _MFM814x125M
#include "mfm814x125m.h"
#define SIDE_CLASS Cmfm814x125m
#endif	// _MFM814x125M

#ifdef _MFM814x250M
#include "mfm814x250m.h"
#define SIDE_CLASS Cmfm814x250m
#endif	// _MFM814x250M

#ifdef _MFM214x250M
#include "mfm214x250m.h"
#define SIDE_CLASS Cmfm214x250m
#endif	// _MFM214x250M

#ifdef _MFM412x500M
#include "mfm412x500m.h"
#define SIDE_CLASS Cmfm412x500m
#endif	// _MFM412x500M

#ifdef _MFM212x1G
#include "mfm212x1g.h"
#define SIDE_CLASS Cmfm212x1g
#endif	// _MFM212x1G

#ifdef _MFM816x250M
#include "mfm816x250m.h"
#define SIDE_CLASS Cmfm816x250m
#endif	// _MFM816x250M

#ifdef _MDAC
#include "mdac.h"
#define SIDE_CLASS Cmdac
#endif	// _MDAC

#ifdef _MADC
#include "madc.h"
#define SIDE_CLASS Cmadc
#endif	// _MADC

#ifdef _M1624x128
#include "m1624x128.h"
#define SIDE_CLASS Cm1624x128
#endif	// _M1624x128

#ifdef _SIDEINC
#include "sidedriver.h"
#endif	// _SIDEINC

#ifdef _MDDC8X32
#include "mddc8x32.h"
#define SIDE_CLASS Cmddc8x32
#endif	// _MDDC8X32

#ifdef _M8GC
#include "m8gc.h"
#define SIDE_CLASS Cm8gc
#endif

#ifdef _DR16RES
#include "dr16res.h"
#define SIDE_CLASS CDr16Res
#endif

#ifdef _MDUC9957
#include "mduc9957.h"
#define SIDE_CLASS Cmduc9957
#endif

#ifdef _MFM402S
#include "mfm402s.h"
#define SIDE_CLASS Cmfm402s
#endif

#ifdef _MFM401S
#include "mfm401s.h"
#define SIDE_CLASS Cmfm401s
#endif

#endif	// _SIDEDLL_H_

// ****************** End of file sidedll.h **********************
