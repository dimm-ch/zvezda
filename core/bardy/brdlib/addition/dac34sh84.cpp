// ***************************************************************
//  dac34sh84.cpp   version:  1.0   ·  date: 27/06/2014
//  -------------------------------------------------------------
//  Вспомогательные функции для управления узлом PLL
//  микросхемы DAC34SH84, называемым также DAC PLL
//  -------------------------------------------------------------
//  Copyright (C) 2014 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#include <stdio.h>
#include <math.h>
#include "dac34sh84.h"

#define ROUND(x) ((int)floor((x) + 0.5))

typedef struct
{
	S32			divP, divN, divM;
	S32			nKint, isPllOn;
	double		dFreqDac, dFreqVco, dFreqPfd;
} TDac34sh84PllDividers;

static S32 GetDac34sh84PllDividers( double dFreqClkin, double dFreqData, S32 nInpKint, TDac34sh84PllDividers *pDividers, BRDCHAR *pRetText );

//=**************** V6MCMM_CalcDividers *******************
//=********************************************************
S32	DAC34SH84_CalcDividers( TDAC34SH84_Dividers *pSD, BRDCHAR *pRetText )
{
	TDac34sh84PllDividers	rDividers;
	S32						err;

	//
	// Вычислить значения делителей
	//
	err = GetDac34sh84PllDividers( pSD->dFreqClkin, pSD->dFreqData, pSD->nKint, &rDividers, pRetText );
	if( err < 0 )
		return err;

	pSD->dFreqDac = rDividers.dFreqDac;
	pSD->dFreqVco = rDividers.dFreqVco;
	pSD->dFreqPfd = rDividers.dFreqPfd;
	pSD->divP = rDividers.divP;
	pSD->divN = rDividers.divN;
	pSD->divM = rDividers.divM;
	pSD->nKint= rDividers.nKint;
	pSD->isPllOn = rDividers.isPllOn;


	//
	// Сформировать регистры
	//
	U32		reg00, reg18, reg19, reg1A, reg1B;

	reg00 = 0x8;
	if( rDividers.nKint <= 8 ) reg00 = 0x4;
	if( rDividers.nKint <= 4 ) reg00 = 0x2;
	if( rDividers.nKint <= 2 ) reg00 = 0x1;
	if( rDividers.nKint <= 1 ) reg00 = 0x0;
	reg00 <<= 8;
	reg00 |= 0x9C; //0x9C;

	if( rDividers.isPllOn )
	{
		//reg18 = 0x2440 | ((rDividers.divP & 0x7)<<3);
		reg18 = 0x2C40 | ((rDividers.divP & 0x7)<<3);
		if( rDividers.divM<=127 )
			reg19 = 0x0004 | (rDividers.divM << 8);
		else
			reg19 = 0x8004 | ((rDividers.divM & 0xFE) << 7);
		reg19 |= (rDividers.divN - 1) << 4;
		reg1A = ROUND( (rDividers.dFreqVco/1000000.0 - 2673.0) / 9.7 );
		reg1A <<= 10;
		reg1A |= 0x10;
		reg1B = 0x0000;
	}
	else
	{
		reg18 = 0x3040;
		reg19 = 0x0440;
		reg1A = 0x0020;
		reg1B = 0x0000;
	}

	//pSD->aReg[0].adr = 0x00; pSD->aReg[0].val = reg00;
	//pSD->aReg[1].adr = 0x18; pSD->aReg[1].val = reg18;
	//pSD->aReg[2].adr = 0x19; pSD->aReg[2].val = reg19;
	//pSD->aReg[3].adr = 0x1A; pSD->aReg[3].val = reg1A;
	//pSD->aReg[4].adr = 0x1B; pSD->aReg[4].val = reg1B;


	pSD->regCnt = 14;
	pSD->aReg[0].adr = 0x03; pSD->aReg[0].val = 0xF000;
	pSD->aReg[1].adr = 0x00; pSD->aReg[1].val = reg00 | 0x4;

	pSD->aReg[2].adr = 0x18; pSD->aReg[2].val = 0x2C00 | (0xFF & reg18);
	pSD->aReg[3].adr = 0x19; pSD->aReg[3].val = reg19;
	pSD->aReg[4].adr = 0x1A; pSD->aReg[4].val = reg1A;
	pSD->aReg[5].adr = 0x1B; pSD->aReg[5].val = reg1B;

	//
	// Read ALARM
	//
	pSD->aReg[6].adr = 0xFFFFFFFF; pSD->aReg[6].val = 0x0000;

	//pSD->aReg[7].adr = 0x20; pSD->aReg[7].val = 0x8801;
	pSD->aReg[7].adr = 0x20; pSD->aReg[7].val = 0x2401;
	pSD->aReg[8].adr = 0x1F; pSD->aReg[8].val = 0x1142;
	//pSD->aReg[9].adr = 0x00; pSD->aReg[9].val = reg00 & (~0x4);
	pSD->aReg[9].adr = 0x00; pSD->aReg[9].val = reg00 | 0x4;
	pSD->aReg[10].adr = 0x1F; pSD->aReg[10].val = 0x1140;
	//pSD->aReg[11].adr = 0x20; pSD->aReg[11].val = 0x0000;
	pSD->aReg[11].adr = 0x20; pSD->aReg[11].val = 0x2401;
	pSD->aReg[12].adr = 0x18; pSD->aReg[12].val = reg18;

	//
	// Read ALARM
	//
	pSD->aReg[13].adr = 0xFFFFFFFF; pSD->aReg[13].val = 0x0000;


	return 0;
}

//=************ DAC34SH84_FormRegisterCodes ***************
//=********************************************************
S32	DAC34SH84_FormRegisterCodes( 
				TDAC34SH84_Dividers *pSD, TDAC34SH84_TuneParams *pTP,
				TDAC34SH84_RegValue *paRV, int *pItemNum )
{
	int			idx = 0;
	U32			val;
	U32			code;
	U32			aConfig[50];

	//
	// Config0
	//
	val = 0x001C;
	val |= (pTP->nQMC_offsetAB_ena) ? 0x8000 : 0x0;
	val |= (pTP->nQMC_offsetCD_ena) ? 0x4000 : 0x0;
	val |= (pTP->nQMC_corrAB_ena) ? 0x2000 : 0x0;
	val |= (pTP->nQMC_corrCD_ena) ? 0x1000 : 0x0;
	val |= (pTP->nInvSincAB_ena) ? 0x0002 : 0x0;
	val |= (pTP->nInvSincCD_ena) ? 0x0001 : 0x0;
	val |= (pTP->nFifo_ena) ? 0x0080 : 0x0;

	code = 0x8;
	if( pSD->nKint <= 8 ) code = 0x4;
	if( pSD->nKint <= 4 ) code = 0x2;
	if( pSD->nKint <= 2 ) code = 0x1;
	if( pSD->nKint <= 1 ) code = 0x0;
	val |= code << 8;
	aConfig[0] = val; 


	//
	// Config1
	//
	val = 0x04FE;
	aConfig[1] = val; 

	//
	// Config2
	//
	val = 0x6002;
	val |= (pTP->nMixer_ena)  ? 0x0040 : 0x0;
	val |= (pTP->nMixer_gain) ? 0x0020 : 0x0;
	val |= (pTP->nNco_ena)    ? 0x0010 : 0x0;
	aConfig[2] = val; 

	//
	// Config3
	//
	val = 0x0000;
	val |= pTP->nCoarse_dac << 12;
	aConfig[3] = val; 

	//
	// Config7
	//
	val = 0xD8FF;
	aConfig[7] = val; 

	//
	// Config8
	//
	val = 0x0000;
	val |= 0x1FFF & pTP->nQMC_offsetA;
	aConfig[8] = val; 

	//
	// Config9
	//
	val = 0x8000;
	val |= 0x1FFF & pTP->nQMC_offsetB;
	aConfig[9] = val; 

	//
	// Config10
	//
	val = 0x0000;
	val |= 0x1FFF & pTP->nQMC_offsetC;
	aConfig[10] = val; 

	//
	// Config11
	//
	val = 0x0000;
	val |= 0x1FFF & pTP->nQMC_offsetD;
	aConfig[11] = val; 

	//
	// Config12
	//
	val = 0x0000;
	val |= 0x07FF & pTP->nQMC_gainA;
	aConfig[12] = val; 

	//
	// Config13
	//
	val = 0x0000;
	val |= pTP->nCMIX << 12;
	val |= 0x07FF & pTP->nQMC_gainB;
	aConfig[13] = val; 

	//
	// Config14
	//
	val = 0x0000;
	val |= 0x07FF & pTP->nQMC_gainC;
	aConfig[14] = val; 

	//
	// Config15
	//
	val = 0x0000;
	val |= 0x07FF & pTP->nQMC_gainD;
	aConfig[15] = val; 

	//
	// Config16
	//
	val = 0x0000;
	val |= 0x0FFF & pTP->nQMC_phaseAB;
	aConfig[16] = val; 

	//
	// Config17
	//
	val = 0x0000;
	val |= 0x0FFF & pTP->nQMC_phaseCD;
	aConfig[17] = val; 

	//
	// Config18
	//
	val = 0xFFFF & pTP->nPhase_offsetAB;
	aConfig[18] = val; 

	//
	// Config19
	//
	val = 0xFFFF & pTP->nPhase_offsetCD;
	aConfig[19] = val; 

	//
	// Config20, Config21
	//
	double			dVal;

	dVal  = pTP->dFNcoAB / pSD->dFreqDac;
	dVal *= 65536.0;
	dVal *= 65536.0;
	val = (U32)dVal;
	aConfig[20] = 0xFFFF & val; 
	aConfig[21] = 0xFFFF & (val>>16);

	//
	// Config22, Config23
	//
	dVal  = pTP->dFNcoCD / pSD->dFreqDac;
	dVal *= 65536.0;
	dVal *= 65536.0;
	val = (U32)dVal;
	aConfig[22] = 0xFFFF & val; 
	aConfig[23] = 0xFFFF & (val>>16);

	//
	// Config24
	//
	val = 0x3040;
	if( pSD->isPllOn )
		val = 0x2C40 | ((pSD->divP & 0x7)<<3);
	aConfig[24] = val; 

	//
	// Config25
	//
	val = 0x0440;
	if( pSD->isPllOn )
	{
		if( pSD->divM<=127 )
			val = 0x0004 | (pSD->divM << 8);
		else
			val = 0x8004 | ((pSD->divM & 0xFE) << 7);
		val |= (pSD->divN - 1) << 4;
	}
	aConfig[25] = val; 

	//
	// Config26
	//
	val = 0x0020;
	if( pSD->isPllOn )
	{
		val = ROUND( (pSD->dFreqVco/1000000.0 - 2673.0) / 9.7 );
		val <<= 10;
		val |= (pTP->nSleepA) ? 0x8 : 0x0;
		val |= (pTP->nSleepB) ? 0x4 : 0x0;
		val |= (pTP->nSleepC) ? 0x2 : 0x0;
		val |= (pTP->nSleepD) ? 0x1 : 0x0;
	}
	aConfig[26] = val; 

	//
	// Config27, 30, 31, 32, 36
	//

	//
	// Формировать выходной массив для регистров
	//
	idx = 0;

	paRV[idx].adr = 0;	paRV[idx++].val = aConfig[0];
	paRV[idx].adr = 1;	paRV[idx++].val = aConfig[1];
	paRV[idx].adr = 2;	paRV[idx++].val = aConfig[2];
	paRV[idx].adr = 3;	paRV[idx++].val = aConfig[3];
	paRV[idx].adr = 7;	paRV[idx++].val = aConfig[7];
	paRV[idx].adr = 8;	paRV[idx++].val = aConfig[8];
	paRV[idx].adr = 9;	paRV[idx++].val = aConfig[9];

	paRV[idx].adr = 10;	paRV[idx++].val = aConfig[10];
	paRV[idx].adr = 11;	paRV[idx++].val = aConfig[11];
	paRV[idx].adr = 12;	paRV[idx++].val = aConfig[12];
	paRV[idx].adr = 13;	paRV[idx++].val = aConfig[13];
	paRV[idx].adr = 14;	paRV[idx++].val = aConfig[14];
	paRV[idx].adr = 15;	paRV[idx++].val = aConfig[15];
	paRV[idx].adr = 16;	paRV[idx++].val = aConfig[16];
	paRV[idx].adr = 17;	paRV[idx++].val = aConfig[17];
	paRV[idx].adr = 18;	paRV[idx++].val = aConfig[18];
	paRV[idx].adr = 19;	paRV[idx++].val = aConfig[19];

	paRV[idx].adr = 20;	paRV[idx++].val = aConfig[20];
	paRV[idx].adr = 21;	paRV[idx++].val = aConfig[21];
	paRV[idx].adr = 22;	paRV[idx++].val = aConfig[22];
	paRV[idx].adr = 23;	paRV[idx++].val = aConfig[23];
	paRV[idx].adr = 24;	paRV[idx++].val = aConfig[24];
	paRV[idx].adr = 25;	paRV[idx++].val = aConfig[25];
	paRV[idx].adr = 26;	paRV[idx++].val = aConfig[26];

	paRV[idx].adr = 27;	paRV[idx++].val = 0x0000;
	paRV[idx].adr = 32;	paRV[idx++].val = 0x8801;
	paRV[idx].adr = 30;	paRV[idx++].val = 0x8888;
	paRV[idx].adr = 31;	paRV[idx++].val = 0x8880;
	paRV[idx].adr = 31;	paRV[idx++].val = 0x8882; 
	paRV[idx].adr = 31;	paRV[idx++].val = 0x8880;
	paRV[idx].adr = 32;	paRV[idx++].val = 0x1201;
	paRV[idx].adr = 36;	paRV[idx++].val = 0x1000;

	*pItemNum = idx;

	return 0;
}

//=**************** GetDac34sh84PllDividers ***************
//=********************************************************
static S32 GetDac34sh84PllDividers( double dFreqClkin, double dFreqData, S32 nInpKint, TDac34sh84PllDividers *pDividers, BRDCHAR *pRetText )
{
	S32			divP, divN, divM;
	double		dFreqVco, dFreqPfd, dFreqDac;
	int			nKint;
	double		dR, dM;
	int			nPM;

	pDividers->divP = pDividers->divN = pDividers->divM = pDividers->nKint = 0;
	pDividers->dFreqDac = pDividers->dFreqVco = pDividers->dFreqPfd = 0.0;

	//
	// Если тактовая частота равна частоте потока данных
	//
	if( dFreqClkin == dFreqData )
	{
		nKint = nInpKint;
		if( (nKint!=2) && (nKint!=4) && (nKint!=8) && (nKint!=16) )
			nKint = 1;
		if( nKint == 1 )
		{
			pDividers->isPllOn = 0;
			pDividers->nKint = 1;
			pDividers->dFreqDac = dFreqData;

			return 0;
		}
		pDividers->isPllOn = 1;
	}
	else
	{
		pDividers->isPllOn = 1;
		nKint = 16;
	}

	//
	// Поиск Kint
	//
	for( ; nKint>=1; nKint /= 2 )
	{
		dFreqDac = nKint * dFreqData;
		if( dFreqDac > (1500.0*1000000.0) )
			continue;

		dR = dFreqDac / dFreqClkin;
		//printf( "Kint=%d, R=%f\n", nKint, dR );

		//
		// Поиск divP
		//
		for( divP=8; divP>=2; divP-- )
		{
			dFreqVco = dFreqClkin * dR * divP;
			if( ((2940.0*1000000.0)<=dFreqVco) && (dFreqVco<=(3300.0*1000000.0)) )
				break;
		}
		if( divP >= 2 )
			break;
	}
	if( nKint<1 )
	{
		BRDC_sprintf( pRetText, _BRDC("P2. Fvco is out of range. Change Fdata or Fclkin") );
		return -1;
	}

	//printf( "P=%d, Fvco=%f MHz\n", divP, dFreqVco/1000000.0 );

	//
	// Поиск divN и divM
	//
	for( divN=1; divN<=16; divN++ )
	{
		dFreqPfd = dFreqClkin / divN;
		if( (dFreqPfd<1000000.0) || (dFreqPfd>(155.0*1000000.0)) )
			continue;

		dM = dR * divN;
		if( dM != floor( dM ) )
			continue;
		divM = (int)floor( dM );
		if( divM<4 )
			continue;
		if( divM>254 )
			continue;
		if( (divM>127) && (divM&1) )
			continue;
		break;
	}
	if( divN > 16 )
	{
		BRDC_sprintf( pRetText, _BRDC("P3. N is out of range. Change Fdata or Fclkin") );
		return -1;
	}
	//printf( "M=%d, N=%d, Fpfd=%f MHz\n", divM, divN, dFreqPfd/1000000.0 );

	//
	// Если включать PLL не требуется (Kint=1)
	//
	if( (dFreqClkin == dFreqData) && (nKint == 1) )
	{
		pDividers->isPllOn = 0;
		pDividers->dFreqDac = dFreqData;
	}

	//
	// Результат
	//
	pDividers->divP  = divP; 
	pDividers->divN  = divN;
	pDividers->divM  = divM;
	pDividers->nKint = nKint;
	pDividers->dFreqDac = dFreqDac;
	pDividers->dFreqVco = dFreqVco;
	pDividers->dFreqPfd = dFreqPfd;

	nPM = divP * divM;
	if( (nPM<24) || (nPM>480) )
	{
		BRDC_sprintf( pRetText, _BRDC("P4. Risk of PLL DAC self-oscillaltion") );
		return 1;
	}

	return 0;
}

//
// End of File
//

