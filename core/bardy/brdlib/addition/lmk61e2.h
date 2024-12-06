// ***************************************************************
//  lmk61e2.h   version:  1.0   ·  date: 17/08/2022
//  -------------------------------------------------------------
//  Вспомогательные функции для управления генератором lmk61e2
//  -------------------------------------------------------------
//  Copyright (C) 2022 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef _LMK61E2_H
#define _LMK61E2_H

#include "utypes.h"

constexpr U32 OUT_DIV_LOW = 23;
constexpr U32 OUT_DIV_HIGH = 22;
constexpr U32 PLL_NDIV_LOW = 26;
constexpr U32 PLL_NDIV_HIGH = 25;
constexpr U32 PLL_NUM_HIGH = 27;
constexpr U32 PLL_NUM_MED = 28;
constexpr U32 PLL_NUM_LOW = 29;
constexpr U32 PLL_DEN_HIGH = 30;
constexpr U32 PLL_DEN_MED = 31;
constexpr U32 PLL_DEN_LOW = 32;
constexpr U32 PLL_DTHRMODE_ORDER = 33;
constexpr U32 PLL_D_CP = 34;
constexpr U32 PLL_CP_PHASE_SHIFT_ENABLE_C3 = 35;
constexpr U32 PLL_LF_R2 = 36;
constexpr U32 PLL_LF_C1 = 37;
constexpr U32 PLL_LF_R3 = 38;
constexpr U32 PLL_LF_C3 = 39;

struct LMK61E2_REG_DATA{
	U32 PLL_D; //записывать в регистр с -1
	U32 PLL_NDIV;
	U32 PLL_NUM;
	U32 PLL_DEN;
	U32 OUT_DIV;
	double Fout;
	double deltaFout;
	U32 PLL_LF_R2;
	U32 PLL_LF_R3;
	U32 PLL_LF_C1;
	U32 PLL_LF_C3;
	U32 PLL_CP_PHASE_SHIFT;
	U32 PLL_ENABLE_C3;
	U32 PLL_CP;
	U32 PLL_ORDER;
	U32 PLL_DTHRMODE;
};

int	LMK61E2_SetRate( double *pRate, double* pDelta, LMK61E2_REG_DATA* prRegData );
int	LMK61E2_GetRate( double *pRate, double* pDelta, LMK61E2_REG_DATA* prRegData );
int	LMK61E2_CalcDiv( double dRate, LMK61E2_REG_DATA* prRegData);

#endif //_LMK61E2_H
//
// End of File
//

