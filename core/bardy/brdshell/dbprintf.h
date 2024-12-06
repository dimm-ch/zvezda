/****************************************
*
* DBPRINTF.H
*
* Define DB_printf
*
* (C) Instr.Sys. by Ekkore Dec, 1997
*
* Changed:
*	12.08.98	Added DB_errdisplay
*
*****************************************/

#ifndef	__DBPRINF_H_
#define	__DBPRINF_H_
/*
typedef enum
{
	errlvlNONE = 0,		// Display Nothing
	errlvlINFO = 1,		// Display Information
	errlvlWARN = 2,		// Display Warning
	errlvlERROR = 4,	// Display Error
	errlvlFATAL = 8,	// Display Fatal Error
	errlvlALWAYS = 15,	// Display Always
	errlvlCONSOLE = 0x80	// Display to Console
} DB_ErrLevel;

void				DB_errdisplay( DB_ErrLevel errLevel );
BRD_API int STDCALL	DB_print (DB_ErrLevel errLevel, char *title, char *msg );
BRD_API int STDCALL	DB_printf(DB_ErrLevel errLevel, char *title, char *format, va_list arglist );
*/

#endif	// __DBPRINF_H_ 


//
//  End of File
//

