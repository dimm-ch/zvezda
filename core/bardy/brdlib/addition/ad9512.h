// ***************************************************************
//  Ad9512.h   version:  1.0   ·  date: 12/05/2012
//  -------------------------------------------------------------
//  Вспомогательные функции для управления синтезатором AD9512
//  -------------------------------------------------------------
//  Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef _AD9512_H
#define _AD9512_H

#include "utypes.h"
#include "useful.h"

typedef struct { U32 adr, val; } AD9512_TReg;

int	AD9512_ClkSource( int ClkSrc, AD9512_TReg *paRegs, S32 arrSize, S32 *realSize );
int	AD9512_DividerMode( double *pClk, double *pRate, U32 mask, AD9512_TReg *paRegs, S32 arrSize, S32 *realSize );

#endif //_AD9512_H
//
// End of File
//

