/****************************************
*
* DBPRINTF.CPP
*
* Write DEBUG message to screen.
*
* (C) Instr.Sys. by Ekkore Dec, 1997
*
* Changed:
*	12.08.98	Added DB_errdisplay()
*				isDB_printf type - static
*	22.11.01	Added DB_ErrLevel support
*
*****************************************/

//#include <stdio.h>
//#include <conio.h>
//#include <stdarg.h>

#ifdef _WIN32
#define	BRD_API		DllExport
#define	BRDI_API	DllExport
#endif

#include "utypes.h"
#include "brdi.h"


static S32	db_ErrLevel = BRDdm_UNVISIBLE;

int			MSG_printf( S32 errLevel, const BRDCHAR *title, const BRDCHAR *msg );

//=***************** DB_errdisplay *******************
//=***************************************************
void	DB_errdisplay( S32 errLevel )
{
	db_ErrLevel = errLevel;
}

//=***************** BRDI_print **********************
//=***************************************************
BRD_API int STDCALL	BRDI_print( S32 errLevel, const BRDCHAR *title, const BRDCHAR *msg )
{
	if( (db_ErrLevel&errLevel) || (BRDdm_VISIBLE==errLevel) )
		return MSG_printf( errLevel, title, msg );	
	return 0;
}

//=***************** BRDI_printf *********************
//=***************************************************
BRD_API int STDCALL	BRDI_printf(S32 errLevel, const BRDCHAR *title, const BRDCHAR *format, va_list arglist )
{
	//va_start( arglist, format );

	if( (db_ErrLevel&errLevel) || (BRDdm_VISIBLE==errLevel) )
	{
		BRDCHAR	lin[256];

		BRDC_vsprintf( lin, format, arglist );
		return MSG_printf( errLevel, title, lin );	
	}
	return 0;
}

//=***************** MSG_printf ***********************
//=****************************************************
int MSG_printf( S32 errLevel, const BRDCHAR *title, const BRDCHAR *msg )
{
		BRDCHAR	capture[300];
#ifdef _WIN32
		UINT	iconType;
		
		//
		// Prepare Msg Text
		//
		if( (errLevel & BRDdm_VISIBLE) == BRDdm_VISIBLE )
		{
			BRDC_sprintf( capture, _BRDC("BRD IMPORTANT [%s]"), title );
			iconType = MB_ICONWARNING;
		}
		else if( errLevel & BRDdm_WARN )
		{
			BRDC_sprintf( capture, _BRDC("BRD WARNING [%s]"), title );
			iconType = MB_ICONWARNING;
		}
		else if( errLevel & BRDdm_INFO )
		{
			BRDC_sprintf( capture, _BRDC("BRD INFO [%s]"), title );
			iconType = MB_ICONINFORMATION;
		}
		else if( errLevel & BRDdm_FATAL )
		{
			BRDC_sprintf( capture, _BRDC("BRD FATAL [%s]"), title );
			iconType = MB_ICONERROR;
		}
		else
		{
			BRDC_sprintf( capture, _BRDC("BRD ERROR [%s]"), title );
			iconType = MB_ICONERROR;
		}

		//
		// Display Msg
		//
		if( db_ErrLevel & BRDdm_CONSOLE )
		{
			BRDC_printf(_BRDC("%s %s\n"), capture, msg );
			if( (errLevel & BRDdm_FATAL) || (errLevel & BRDdm_ERROR) )
			{
				if( 0x1B == _getch() )
					exit(0);
			}
		}
		else
		{
			int		ret;
			BRDCHAR	*pLin = (BRDCHAR*)malloc( BRDC_strlen(msg)+300 );

			if( pLin != NULL )
			{	
				BRDC_strcpy( pLin, msg );
				BRDC_strcat( pLin, _BRDC("\n\nContinue output of BRD message boxes?\nPress [CANCEL] to stop Application.") );
				ret = MessageBox(NULL, pLin, capture, MB_YESNOCANCEL | iconType);
				free( pLin );
			}
			else
				ret = MessageBox(NULL, msg, capture, MB_OKCANCEL | iconType);
			if( ret==IDNO )
				db_ErrLevel &= ~0xF;
			else if( ret == IDCANCEL )
				exit(0);
		}
#endif
#ifdef __linux__
                //
                // Prepare Msg Text
                //
                if( (errLevel & BRDdm_VISIBLE) == BRDdm_VISIBLE )
                {
                        BRDC_sprintf( capture, _BRDC("BRD IMPORTANT [%s]"), title );
                }
                else if( errLevel & BRDdm_WARN )
                {
                        BRDC_sprintf( capture, _BRDC("BRD WARNING [%s]"), title );
                }
                else if( errLevel & BRDdm_INFO )
                {
                        BRDC_sprintf( capture, _BRDC("BRD INFO [%s]"), title );
                }
                else if( errLevel & BRDdm_FATAL )
                {
                        BRDC_sprintf( capture, _BRDC("BRD FATAL [%s]"), title );
                }
                else
                {
                        BRDC_sprintf( capture, _BRDC("BRD ERROR [%s]"), title );
                }

                //
                // Display Msg
                //
                if( db_ErrLevel & BRDdm_CONSOLE )
                {
                        BRDC_printf(_BRDC("%s %s\n"), capture, msg );
                }
#endif
	return 0;
}



//
//  End of File
//

