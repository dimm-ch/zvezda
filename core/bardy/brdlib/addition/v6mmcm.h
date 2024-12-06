// ***************************************************************
//  v6mmcm.h   version:  1.0   ·  date: 27/06/2014
//  -------------------------------------------------------------
//  Вспомогательные функции для управления узлом Virtex-6 MMCM,
//  называемым также FPGA PLL
//  -------------------------------------------------------------
//  Copyright (C) 2014 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#ifndef _V6MMCM_H
#define _V6MMCM_H

#include "utypes.h"

#define	V6MCMM_REGMAX	24

typedef struct  
{
	U32		adr;
	U32		val;
} TV6MMCM_RegValue;

typedef struct  
{
	// INPUT:
	double		dFreqClkin;			// Частота источника тактовой частоты
	double		dFreqDataReq;		// Трнбуемая частота потока отсчетов
	double		dFreqDataMax;		// Максимальная частота потока отсчетов
	double		deltaFreqData;		// Разрешенное отклонение от часьоты dFreqDataReq

	//OUTPUT:
	double		dFreqData;			// Вычисленная частота потока отсчетов
	double		dFreqVco;
	double		dFreqPfd;
	U32			divD;
	U32			divM;
	U32			divO;

	S32			regCnt;
	TV6MMCM_RegValue	aReg[V6MCMM_REGMAX];
} TV6MMCM_Dividers;

//S32	V6MMCM_CalcDividers();

S32	V6MMCM_CalcDividers( TV6MMCM_Dividers *pSD, BRDCHAR *pRetText );
//S32	V6MMСM_CalcDividers( TV6MMCM_Dividers *pSD, BRDCHAR *pRetText );
S32	V6MMСM_CalcFreqData( TV6MMCM_Dividers *pSD, BRDCHAR *pRetText );

#endif //_V6MMCM_H

//
// End of File
//

