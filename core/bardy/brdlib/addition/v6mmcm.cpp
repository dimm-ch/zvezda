// ***************************************************************
//  v6mmcm.cpp   version:  1.0   ·  date: 27/06/2014
//  -------------------------------------------------------------
//  Вспомогательные функции для управления узлом Virtex-6 MMCM,
//  называемым также FPGA PLL
//  -------------------------------------------------------------
//  Copyright (C) 2014 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#include <stdio.h>
#include <math.h>
#include "v6mmcm.h"

typedef struct
{
	S32			divD, divO, divM;
	double		dFreqVco, dFreqPfd, dFreqData, delta;
} TV6FpgaMmcmDividers;

static U32 PllFPGAEncodeHL( S32 div );
static S32 GetV6FpgaMmcmDividers( double dFreqClkin, double dFreqData, 
								 double dFreqDataMax, double deltaFreqData, 
								 TV6FpgaMmcmDividers *pDividers, BRDCHAR *pRetText );

//=**************** V6MCMM_CalcDeviders *******************
//=********************************************************
S32	V6MMCM_CalcDividers( TV6MMCM_Dividers *pSD, BRDCHAR *pRetText )
{
	TV6FpgaMmcmDividers		rDividers;
	S32						err;

	//
	// Вычислить значения делителей
	//
	err = GetV6FpgaMmcmDividers( pSD->dFreqClkin, pSD->dFreqDataReq, 
		pSD->dFreqDataMax, pSD->deltaFreqData, &rDividers, pRetText );
	if( err < 0 )
		return err;

	pSD->dFreqData = rDividers.dFreqData;
	pSD->dFreqVco = rDividers.dFreqVco;
	pSD->dFreqPfd = rDividers.dFreqPfd;
	pSD->divD = rDividers.divD;
	pSD->divM = rDividers.divM;
	pSD->divO = rDividers.divO;

	//
	// Сформировать регистры
	//
	U32		nDIVCLK, nCLKFBOUT, nCLKOUT0, nCLKOUT0_, nCLKOUT1;

	nDIVCLK   = (rDividers.divD == 1) ? 0x3001 : PllFPGAEncodeHL(rDividers.divD);
	nCLKFBOUT = 0x1000 | PllFPGAEncodeHL( rDividers.divM );
	nCLKOUT0  = (rDividers.divO == 1) ? 0x1001 : 0x1000 | PllFPGAEncodeHL( rDividers.divO );
	nCLKOUT0_ = (rDividers.divO == 1) ? 0x00C0 : 0x0000;
	nCLKOUT1  = 0x1000 | PllFPGAEncodeHL( 2 * rDividers.divO );

	pSD->aReg[0].adr = 0x16; pSD->aReg[0].val = nDIVCLK;
	pSD->aReg[1].adr = 0x14; pSD->aReg[1].val = nCLKFBOUT;
	pSD->aReg[2].adr = 0x08; pSD->aReg[2].val = nCLKOUT0;
	pSD->aReg[3].adr = 0x09; pSD->aReg[3].val = nCLKOUT0_;
	pSD->aReg[4].adr = 0x0A; pSD->aReg[4].val = nCLKOUT1;
	pSD->regCnt = 5;
	return 0;
}

//=**************** V6MCMM_CalcFreqData *******************
//=********************************************************
S32	V6MMСM_CalcFreqData( TV6MMCM_Dividers *pSD, BRDCHAR *pRetText )
{
	return 0;
}

//=******************** PllFPGAEncodeHL *******************
//=********************************************************
static U32 PllFPGAEncodeHL( S32 div )
{
	U32 code = 0;

	int lc = 0;
	int hc = 0;

	if( div > 1 )
	{
		lc = div/2;
		hc = div - lc;

		code = (hc << 6) | lc;
	}

	return code;
}

//=**************** GetFpgaPllDividers ********************
//=********************************************************
static S32 GetV6FpgaMmcmDividers( double dFreqClkin, double dFreqData, 
								 double dFreqDataMax, double deltaFreqData, 
								 TV6FpgaMmcmDividers *pDividers, BRDCHAR *pRetText )
{
	S32			divD, divO, divM;
	double		dFreqVco, dFreqPfd, dFreqData2, delta;
	int			cnt;

	pDividers->divD = pDividers->divO = pDividers->divM = 0;
	pDividers->dFreqVco = pDividers->dFreqPfd = pDividers->dFreqData = 0.0;
	pDividers->delta = 2 * deltaFreqData;

	//
	// Проверить входные параметры
	//
	if( (dFreqClkin < 10000000.0) || (dFreqClkin > 800000000.0) )
	{
		BRDC_sprintf( pRetText, _BRDC("Fclkin is out of range 10-800 MHz") );
		return -1;
	}
	if( (dFreqData <= 0.0) || (dFreqData > 750000000.0) )
	{
		BRDC_sprintf( pRetText, _BRDC("Fdata is out of range 0-750 MHz") );
		return -1;
	}
	if( deltaFreqData <= 0.0 )
	{
		BRDC_sprintf( pRetText, _BRDC("Delta is out of range (more than 0)") );
		return -1;
	}

	//
	// Перебираем делитель D
	//
	cnt = 0;
	divM = 64;
	divO = 64;
	for(divD=1; divD<=80; divD++ )
	{
		dFreqPfd = dFreqClkin / divD;
		if( (dFreqPfd < 10000000.0) || (dFreqPfd > 300000000.0) )
			continue;

		//
		// Перебираем делитель M
		//
		for( ; divM>=5; divM-- )
		{
			dFreqVco = dFreqClkin * divM / divD;
			if( (dFreqVco < 600000000.0) || (dFreqVco > 1440000000.0) )
				continue;

			//
			// Перебираем делитель O
			//
			for( ; divO>=1; divO-- )
			{
				dFreqData2 = dFreqVco / divO;
				delta = dFreqData2 - dFreqData;
				if( fabs(delta) > deltaFreqData )
					continue;
				if( dFreqData2 > dFreqDataMax )
					continue;
				
				//
				// Найдена подходящая комбинация
				//

				//printf( "%2d/ D=%2d, M=%2d, O=%2d, Fdata=%12.1f, Fpfd=%12.1f, Fvco=%12.1f\n", cnt, divD, divM, divO, dFreqData2, dFreqPfd, dFreqVco );
				cnt++;

				if( fabs(delta) < fabs(pDividers->delta) )
				{
					pDividers->divD = divD;
					pDividers->divO = divO;
					pDividers->divM = divM;
					pDividers->dFreqVco = dFreqVco;
					pDividers->dFreqPfd = dFreqPfd;
					pDividers->dFreqData = dFreqData2;
					pDividers->delta = delta;

				}
				//break;
			}
			if( divO < 1 )
				divO = 64;

			//break;
		}
		if( divM < 5 )
			divM = 64;
	}

	return 0;
}

//
// End of File
//

