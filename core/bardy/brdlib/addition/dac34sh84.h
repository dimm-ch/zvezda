// ***************************************************************
//  dac34sh84.h   version:  1.0   ·  date: 27/06/2014
//  -------------------------------------------------------------
//  Вспомогательные функции для управления узлом PLL
//  микросхемы DAC34SH84, называемым также DAC PLL
//  -------------------------------------------------------------
//  Copyright (C) 2014 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef _DAC34SH84_H
#define _DAC34SH84_H

#include "utypes.h"

#define	DAC34SH84_REGMAX	25

typedef struct  
{
	U32		adr;
	U32		val;
} TDAC34SH84_RegValue;

typedef struct  
{
	// INPUT:
	double		dFreqClkin;			// Частота источника тактовой частоты
	double		dFreqData;		// Трнбуемая частота потока отсчетов

	//OUTPUT:
	double		dFreqDac;			// Интерполированная частота дискретизации на выходе ЦАПа
	double		dFreqVco;
	double		dFreqPfd;
	U32			divP;
	U32			divN;
	U32			divM;
	S32			nKint;
	S32			isPllOn;

	S32			regCnt;
	TDAC34SH84_RegValue	aReg[DAC34SH84_REGMAX];
} TDAC34SH84_Dividers;

typedef struct 
{
	S32 nQMC_offsetAB_ena, nQMC_offsetCD_ena,
		nQMC_corrAB_ena, nQMC_corrCD_ena,  
		nQMC_offsetA, nQMC_offsetB, nQMC_offsetC, nQMC_offsetD, 
		nQMC_gainA, nQMC_gainB, nQMC_gainC, nQMC_gainD, 
		nQMC_phaseAB, nQMC_phaseCD, 
		nInvSincAB_ena, nInvSincCD_ena,
		nFifo_ena;
	S32	nMixer_ena, nMixer_gain, nNco_ena,
		nCoarse_dac, nCMIX, nSleepA, nSleepB, nSleepC, nSleepD,
		nPhase_offsetAB, nPhase_offsetCD; 
	double	dFNcoAB, dFNcoCD;
} TDAC34SH84_TuneParams;

//S32	V6MMCM_CalcDividers();

S32	DAC34SH84_CalcDividers( TDAC34SH84_Dividers *pSD, BRDCHAR *pRetText );
S32	DAC34SH84_FormRegisterCodes( TDAC34SH84_Dividers *pSD, TDAC34SH84_TuneParams *pTP,
								 TDAC34SH84_RegValue *paRV, int *pItemCnt );

#endif //_DAC34SH84_H

//
// End of File
//

