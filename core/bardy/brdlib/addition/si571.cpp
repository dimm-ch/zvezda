// ***************************************************************
//  Si571.cpp   version:  1.0   ·  date: 12/05/2012
//  -------------------------------------------------------------
//  Вспомогательные функции для управления генератором Si570/Si571
//  -------------------------------------------------------------
//  Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#include <stdio.h>
#include "si571.h"

//***************************************************************************************
int	SI571_SetRate( double *pRate, double dGenFxtal, U32 *paRegData )
{
	//
	// Функция формирует коды регистров, позволяющих сформировать
	// частоту входного сигнала
	//
	U32			rfreqLo, rfreqHi, hsdivCode, n1Code;
	int			status = 0;

	//
	// Скорректировать частоту, если необходимо
	//
	if( *pRate < 10000000.0 )
	{
		*pRate = 10000000.0;
		status = 1;
	}
	if( (*pRate > 945000000.0) && (*pRate<970000000.0) )
	{
		*pRate = 945000000.0;
		status = 1;
	}
	if( (*pRate > 1134000000.0) && (*pRate<1212500000.0) )
	{
		*pRate = 1134000000.0;
		status = 1;
	}

	//
	// Вычислить коэффициенты
	//
	if( 0 > SI571_CalcDiv( *pRate, dGenFxtal, &rfreqLo, &rfreqHi, &hsdivCode, &n1Code ) )
		return -1;

	paRegData[7]  = paRegData[13] = (hsdivCode << 5) | (n1Code >> 2);
	paRegData[8]  = paRegData[14] = (n1Code << 6) | (rfreqHi >> 4);
	paRegData[9]  = paRegData[15] = (rfreqHi << 4) | (rfreqLo >> 24);
	paRegData[10] = paRegData[16] = (rfreqLo >> 16) & 0xFF;
	paRegData[11] = paRegData[17] = (rfreqLo >> 8) & 0xFF;
	paRegData[12] = paRegData[18] = rfreqLo & 0xFF;	

	return status;	
}

//***************************************************************************************
int	SI571_GetRate( double *pRate, double dGenFxtal, U32 *paRegData )
{
	//
	// Функция по кодам регистров вычисляет и возвращает 
	// частоту выходного сигнала.
	//

	U32			rfreqLo, rfreqHi, hsdivCode, n1Code;
	double		freqTmp;
	double		dRFreq, dHsdiv, dN1;

	//
	// Восстановить коэффициенты 
	//
	rfreqLo  = 0xFF & paRegData[12];
	rfreqLo |= (0xFF & paRegData[11]) << 8;
	rfreqLo |= (0xFF & paRegData[10]) << 16;
	rfreqLo |= (0xF & paRegData[9]) << 24;

	rfreqHi  = (0xF0 & paRegData[9]) >> 4;
	rfreqHi |= (0x3F & paRegData[8]) << 4;

	hsdivCode    = (0xE0 & paRegData[7]) >> 5;

	n1Code   = (0xC0 & paRegData[8]) >> 6;
	n1Code  |= (0x1F & paRegData[7]) << 2;

	dRFreq   = (double)rfreqLo;
	dRFreq  /= 1024.0 * 1024.0 * 256.0;
	dRFreq  += (double)rfreqHi;
	dHsdiv   = (double)( hsdivCode + 4 );
	//dN1      = (n1Code==1) ? 1.0 : (double)( 0xFE & (n1Code+1));
	dN1      = (n1Code==0) ? 1.0 : (double)( 0xFE & (n1Code+1));

	//
	// Рассчитать частоту 
	//
	freqTmp  = dGenFxtal;
	freqTmp /= dHsdiv * dN1;
	freqTmp *= dRFreq;
	*pRate   = freqTmp;

	return 0;
}

//***************************************************************************************
int	SI571_CalcDiv( double dRate, double dFXtal, U32 *pRfreqLo, U32 *pRfreqHi, U32 *pHsdivCode, U32 *pN1Code )
{
	//
	// Вспомогательная функция.
	//

	double		dRFreq, dHsdiv, dN1;
    unsigned	ii;

	//
	// Вычислить коэффициенты HS_DIV, N1
	//
	dHsdiv = dN1 = 0.0;

	if( dRate>=1212500000.0 )
	{
		dHsdiv = 4;
		dN1 = 1;
	}
	else if( dRate>=970000000.0 )
	{
		dHsdiv = 5;
		dN1 = 1;
	}
	else
	{
		double		dDcoMin = 4850000000.0;
		double		dDcoMax = 5670000000.0;
		double		adHsdiv[] = { 4.0, 5.0, 6.0, 7.0, 9.0, 11.0 };
		double		dFreqDco, dFreqTmp;
		S32			n1;

		dFreqDco = dDcoMax;
		for( n1=1; n1<=128; n1++ )
		{
			if( (n1>1) && (n1&0x1) )
				continue;				// только четные n1
			for( ii=0; ii<sizeof(adHsdiv)/sizeof(adHsdiv[0]); ii++ )
			{
				dFreqTmp = dRate * adHsdiv[ii] * (double)n1;
				if( (dFreqTmp>=dDcoMin) && (dFreqTmp<=dFreqDco) )
				{
					dFreqDco = dFreqTmp;
					dHsdiv = adHsdiv[ii];
					dN1 = (double)n1;
				}
			}
		}

	}
	if( (dHsdiv==0.0) || (dN1==0.0) )
		return -1;

	//
	// Вычислить коэффициент RFREQ
	//
	dRFreq  = dRate * dHsdiv * dN1;
	dRFreq /= dFXtal;

	//
	// Преобразовать коэффициенты в коды
	//
	*pRfreqHi = (U32)dRFreq;
	*pRfreqLo = (U32)( (dRFreq - (double)(*pRfreqHi)) * 1024.0 * 1024.0 * 256.0 );
	*pHsdivCode = (U32)(dHsdiv-4.0);
	*pN1Code = (U32)(dN1-1.0);
//printf( "\n" );
//printf( "SI571: freq = %f Hz, xtal = %f Hz", dRate, dFXtal );
//printf( "SI571: N1 = %f, HSDIV = %f, RF = %f\n\n", dN1, dHsdiv, dRFreq );

	return 0;
}

//***************************************************************************************
int	SI571_CalcFxtal( double *pGenFxtal, double dGenRef, U32 *paRegData )
{
    S32			rfreqLo, rfreqHi, hsdivCode, n1Code;
	double		freqTmp;
	double		dRFreq, dHsdiv, dN1;

	rfreqLo  = 0xFF & paRegData[12];
	rfreqLo |= (0xFF & paRegData[11]) << 8;
	rfreqLo |= (0xFF & paRegData[10]) << 16;
	rfreqLo |= (0xF & paRegData[9]) << 24;

	rfreqHi  = (0xF0 & paRegData[9]) >> 4;
	rfreqHi |= (0x3F & paRegData[8]) << 4;

	hsdivCode    = (0xE0 & paRegData[7]) >> 5;

	n1Code   = (0xC0 & paRegData[8]) >> 6;
	n1Code  |= (0x1F & paRegData[7]) << 2;

	dRFreq   = (double)rfreqLo;
	dRFreq  /= 1024.0 * 1024.0 * 256.0;
	dRFreq  += (double)rfreqHi;
	dHsdiv   = (double)( hsdivCode + 4 );
	dN1      = (n1Code==1) ? 1.0 : (double)( 0xFE & (n1Code+1));

	freqTmp  = dGenRef;
	freqTmp /= dRFreq;
	freqTmp *= dHsdiv * dN1;

    // на Zynq вариант кода *pGenFxtal = freqTmp
    // генерирует ошибку невыровненного досутпа к памяти
    memcpy(pGenFxtal, &freqTmp, sizeof(double));

	return 0;
}

//
// End of File//***************************************************************************************

//

