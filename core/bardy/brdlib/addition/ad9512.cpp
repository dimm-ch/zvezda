// ***************************************************************
//  Ad9512.cpp   version:  1.0   ·  date: 12/05/2012
//  -------------------------------------------------------------
//  Вспомогательные функции для управления синтезатором AD9512
//  -------------------------------------------------------------
//  Copyright (C) 2012 - All Rights Reserved
// ***************************************************************
// 
// ***************************************************************

#include <stdio.h>
#include "ad9512.h"

#define DEVIDER_MINMAX( div ) \
	if( pDividers->##div < 1  ) pDividers->##div = 1; \
		if( pDividers->##div > 32 ) pDividers->##div = 32;

#define REG_BEGIN( _p ) AD9512_TReg *_paRegs = _p;
#define REG_END( _p ) (S32)(_paRegs - _p);
#define REGVAL( _adr, _val ) {_paRegs->adr = _adr;  _paRegs->val = _val; _paRegs++;}

int	AD9512_ClkSource( int ClkSrc, AD9512_TReg *paRegs, S32 arrSize, S32 *pRealSize )
{
/*
"00","00010000","10"
"02","00100000","20"
"34","00000001","01"
"35","00000000","00"
"36","00000000","00"
"37","00000100","04"
"3D","00001000","08"
"3E","00001000","08"
"3F","00001000","08"
"40","00000010","02"
"41","00000010","02"
"44","00010011","13"
"45","00010010","12"
"4A","00000000","00"
"4B","10000000","80"
"4C","00010001","11"
"4D","10000000","80"
"4E","00110011","33"
"4F","10000000","80"
"50","00000000","00"
"51","10000000","80"
"52","00010001","11"
"53","10000000","80"
"58","00000000","00"
"59","00000000","00"
"5A","00000000","00"
*/

	REG_BEGIN( paRegs );

		REGVAL( 0, 0x10 );
		REGVAL( 0x2, 0x20 );//?

		REGVAL( 0x34, 0x1 );

		REGVAL( 0x35, 0 );
		REGVAL( 0x36, 0 );


		REGVAL( 0x37, 0x4 );//?

		REGVAL( 0x3D, 0x8 );
		REGVAL( 0x3E, 0x8 );
		REGVAL( 0x3F, 0x8 );

		REGVAL( 0x40, 0x2 );
		REGVAL( 0x41, 0x2 );

		REGVAL( 0x44, 0x13 );//?

		if( ClkSrc==2 )
		{
			REGVAL( 0x45, 0x0 );//clk1 clk 2
		}
		else
		{
			REGVAL( 0x45, 0x11 );
		}

		

		//FUNCTION
		{
			REGVAL( 0x58, 0 );
			REGVAL( 0x59, 0 );//?

			//UPDATE
			//REGVAL( 0x5A, 0x1 );
		}

	*pRealSize = REG_END( paRegs );

	return 0;
}



//***************************************************************************************
int	AD9512_DividerMode( double *pClk, double *pRate, U32 mask, AD9512_TReg *paRegs, S32 arrSize, S32 *pRealSize )
{
		
	int div = (int)ROUND( (*pClk) / (*pRate) );

	if( div <= 0 )
		div = 1;

	*pRate = *pClk / div;

	U32 divA = 0;
	U32 divB = 0;

	if( div > 1 )
	{
		int ncl = (div / 2);
		int nch = (div -ncl);

		nch -= 1;
		ncl -= 1;

		divA = nch | (ncl << 4);
		
	}
	else
	{
		divB = 0x80;
	}
	
	REG_BEGIN( paRegs );
		
		//DEVIDERS
		{
			if( mask & 0x1 )
			{
				REGVAL( 0x4A, divA );
				REGVAL( 0x4B, divB );
			}
			//REGVAL( 0x4B, 0x0 );

			if( mask & 0x2 )
			{
				REGVAL( 0x4C, divA );
				REGVAL( 0x4D, divB );
			}

			if( mask & 0x4 )
			{
				REGVAL( 0x4E, divA );
				REGVAL( 0x4F, divB );
			}
			//REGVAL( 0x4F, 0x0 );

			if( mask & 0x8 )
			{
				REGVAL( 0x50, divA );
				REGVAL( 0x51, divB );
			}
			//REGVAL( 0x51, 0x0 );

			if( mask & 0x10 )
			{
				REGVAL( 0x52, divA );
				//REGVAL( 0x53, 0x0 );
				REGVAL( 0x53, divB );
			}
		}


	*pRealSize = REG_END( paRegs );

	return 0;
}

#undef DEVIDER_MINMAX
#undef REGVAL

//
// End of File
//

