// ***************************************************************
//  Si571.h   version:  1.0   ·  date: 12/05/2012
//  -------------------------------------------------------------
//  Вспомогательные функции для управления генератором Si570/Si571
//  -------------------------------------------------------------
//  Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef _SI571_H
#define _SI571_H

#include "utypes.h"


int	SI571_SetRate( double *pRate, double dGenFxtal, U32 *paRegData );
int	SI571_GetRate( double *pRate, double dGenFxtal, U32 *paRegData );
int	SI571_CalcDiv( double dRate, double dFXtal, U32 *pRfreqLo, U32 *pRfreqHi, U32 *pHsdivCode, U32 *pN1Code );
int	SI571_CalcFxtal( double *pGenFxtal, double dGenRef, U32 *paRegData );

#endif //_SI571_H
//
// End of File
//

