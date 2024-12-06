// ***************************************************************
//  lmk61e2.cpp   version:  1.0   ·  date: 17/08/2022
//  -------------------------------------------------------------
//  Вспомогательные функции для управления генератором LMK61E2
//  -------------------------------------------------------------
//  Copyright (C) InSys 2022 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#include <stdio.h>
#include <math.h>
#include "lmk61e2.h"

int LMK61E2_CalcByAlpha(double dAlpha, ULONG* pnT_INT, ULONG* pnT_NUM, ULONG* pnT_DEN)
{
	double dNum0 = 0, dNum1 = 1, dNum2 = 0, dDen0 = 1, dDen1 = 0, dDen2 = 0, dInt = floor(dAlpha), dQ = 1, dDenMax = pow(2, 22) - 1, dDen = dDenMax, dNum = 0;	
	double dBeta = dAlpha - dInt, dEps = 0.0, dTau = 1 / dDenMax, dd1 = 0.0, dd2 = 0.0, dA = 0;
	if ((0.5 * dTau <= dBeta) && (dBeta < dTau))
	{
		dNum = 1;
		dQ = 0;
	}
	else
	{
		if (dBeta < 0.5 * dTau)
		{
			dNum = 0;
			dQ = 0;
		}
		else
		{
			if (dBeta > 1 - 0.5 * dTau)
			{
				dNum = 0;
				dInt = dInt + 1;
				dQ = 0;
			}
			else
			{
				if ((1.0 - dTau < dBeta) && (dBeta <= 1.0 - 0.5 * dTau))
				{
					dNum = dDenMax - 1;
					dQ = 0;
				}
				else
				{
					dDen1 = floor(1.0 / dAlpha);
					dEps = (1.0 / dAlpha) - dDen1;
					while (1)
					{
						dA = floor(1.0 / dEps);
						dEps = (1.0 / dEps) - dA;
						dNum2 = dNum1 * dA + dNum0;
						dDen2 = dDen1 * dA + dDen0;
						if (dDen2 > dDenMax)
							break;
						dNum0 = dNum1;
						dDen0 = dDen1;
						dNum1 = dNum2;
						dDen1 = dDen2;
					}
					dA = floor((dDenMax - dDen0) / dDen1);
					dNum2 = dNum1 * dA + dNum0;
					dDen2 = dDen1 * dA + dDen0;
					dd1 = dAlpha - (dNum1 / dDen1);
					dd2 = dAlpha - (dNum2 / dDen2);
					if (fabs(dd1) > fabs(dd2))
					{
						dNum = dNum2;
						dDen = dDen2;
					}
					else
					{
						dNum = dNum1;
						dDen = dDen1;
					}
				}
			}
		}
	}
	dNum = dNum - dDen * dInt * dQ;
	*pnT_NUM = (ULONG)floor(dNum);
	*pnT_DEN = (ULONG)floor(dDen);
	*pnT_INT = (ULONG)floor(dInt);
	return 0;
}

//***************************************************************************************
int	LMK61E2_SetRate( double *pRate, double *pDelta, LMK61E2_REG_DATA *prRegData )
{
	int status = 0;
	if (*pRate < 10000000)
		*pRate = 10000000;
	if (*pRate > 1000000000)
		*pRate = 1000000000;	
	status = LMK61E2_CalcDiv(*pRate, prRegData);
	*pRate = prRegData->Fout;
	*pDelta = prRegData->deltaFout;
	return status;	
}

//***************************************************************************************
int	LMK61E2_GetRate( double *pRate, double* pDelta,  LMK61E2_REG_DATA*prRegData )
{
	double dFvco = 0.0, dFout = 0.0, ddFout = 0.0, dDen = prRegData->PLL_DEN;
	if (prRegData->PLL_DEN == 0)
		dDen = 1;
	dFvco = 50'000'000 * prRegData->PLL_D * ((double)prRegData->PLL_NDIV + (double)prRegData->PLL_NUM / dDen);
	dFout = dFvco / prRegData->OUT_DIV;
	*pDelta = fabs(*pRate - dFout);
	*pRate = dFout;
	return 0;
}

//***************************************************************************************
int	LMK61E2_CalcDiv( double dRate, LMK61E2_REG_DATA* prRegData)
{
	double dT_OUT_DIV = 4600000000 / dRate, dAlpha = 0.0, ddFout = 1000000000.0, dT_dFout = 1000000000.0, dFvco = 0.0, dFout = 0.0, dT_Fout = 0.0;
	double d_TFvco = 0.0;
	ULONG nT_PLL_D = 1, nPLL_D = 1, nPLL_CP_PHASE_SHIFT = 0, nPLL_LF_R2 = 0, nPLL_LF_R3 = 0, nPLL_LF_C1 = 0, nPLL_LF_C3 = 0;
	ULONG nPLL_ENABLE_C3 = 0, nPLL_CP = 0, nPLL_ORDER = 0, nPLL_DTHRMODE = 0, nT_INT = 0, nT_NUM = 0, nT_DEN = 0;
	ULONG nINT = 0, nNUM = 0, nDEN = 0, nOUT_DIV = 0;
	if (dT_OUT_DIV - floor(dT_OUT_DIV) != 0)
		dT_OUT_DIV = floor(dT_OUT_DIV) + 1;
	while (true)
	{
		dFvco = dRate * dT_OUT_DIV;
		if ((dFvco <= 5600000000) && (dT_OUT_DIV <= 511.0))
		{
			dAlpha = dFvco / 100000000.0;
			nT_PLL_D = 2;
			while (1/*ddFout != 0.0*/)
			{
				LMK61E2_CalcByAlpha(dAlpha, &nT_INT, &nT_NUM, &nT_DEN);
				double dDen = nT_DEN;
				if (nT_DEN == 0)
					dDen = 1;
				d_TFvco = 50000000.0 * (double)nT_PLL_D * ((double)nT_INT + (double)((double)nT_NUM / dDen));
				dT_Fout = d_TFvco / dT_OUT_DIV;
				dT_dFout = fabs(dT_Fout - dRate);
				if (nT_NUM == 0)
				{
					nPLL_D = nT_PLL_D;
					nINT = nT_INT;
					nNUM = nT_NUM;
					nDEN = nT_DEN;
					nOUT_DIV = floor(dT_OUT_DIV);
					dFout = dT_Fout;
					nPLL_LF_R2 = 0x08;
					nPLL_LF_R3 = 0x0;
					nPLL_LF_C1 = 0x0;
					nPLL_LF_C3 = 0x0;
					nPLL_CP_PHASE_SHIFT = 0x0;
					nPLL_ENABLE_C3 = 0;
					nPLL_CP = 0x8;
					nPLL_ORDER = 0x0;
					nPLL_DTHRMODE = 0x3;
					break;
				}
				else
				{
					if (dT_dFout < ddFout)
					{
						nPLL_D = nT_PLL_D;
						nINT = nT_INT;
						nNUM = nT_NUM;
						nDEN = nT_DEN;
						nOUT_DIV = floor(dT_OUT_DIV);
						dFout = dT_Fout;
						ddFout = dT_dFout;
					}
					//if (ddFout != 0.0)
					{
						if (dAlpha == dFvco / 100000000.0)
						{
							dAlpha *= 2;
							nT_PLL_D = 1;
						}
						else
						{
							dT_OUT_DIV++;
							break;
						}
					}
					//else
					//	break;
				}
			}
			if ((nT_NUM == 0)/* || (ddFout == 0.0)*/)
				break;
		}
		else
			break;
	}
	if ((dFvco > 5600000000) || (dT_OUT_DIV > 511.0)/* || (ddFout == 0.0)*/)
	{
		if (nPLL_D == 2)
			nPLL_CP_PHASE_SHIFT = 0x2;
		else
			nPLL_CP_PHASE_SHIFT = 0x5;
		nPLL_LF_R2 = 0x12;
		nPLL_LF_R3 = 0x09;
		nPLL_LF_C1 = 0x02;
		nPLL_LF_C3 = 0x07;
		nPLL_ENABLE_C3 = 1;
		nPLL_CP = 0x4;
		nPLL_ORDER = 0x3;
		nPLL_DTHRMODE = 0;
	}
	prRegData->deltaFout = ddFout;
	prRegData->Fout = dFout;
	prRegData->OUT_DIV = nOUT_DIV;
	prRegData->PLL_CP = nPLL_CP;
	prRegData->PLL_CP_PHASE_SHIFT = nPLL_CP_PHASE_SHIFT;
	prRegData->PLL_D = nPLL_D;
	prRegData->PLL_DEN = nDEN;
	prRegData->PLL_DTHRMODE = nPLL_DTHRMODE;
	prRegData->PLL_ENABLE_C3 = nPLL_ENABLE_C3;
	prRegData->PLL_LF_C1 = nPLL_LF_C1;
	prRegData->PLL_LF_C3 = nPLL_LF_C3;
	prRegData->PLL_LF_R2 = nPLL_LF_R2;
	prRegData->PLL_LF_R3 = nPLL_LF_R3;
	prRegData->PLL_NDIV = nINT;
	prRegData->PLL_NUM = nNUM;
	prRegData->PLL_ORDER = nPLL_ORDER;	
	return 0;
}

//
// End of File//***************************************************************************************

//

