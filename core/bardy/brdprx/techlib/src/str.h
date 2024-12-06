/********************************************************************
 *  (C) Copyright 1992-2005 InSys Corp.  All rights reserved.
 *
 *  ======== b101E.cpp ========
 *  Driver for:
 *		TUNI
 *
 *  version 1.0
 *
 ********************************************************************/

#ifndef _TSTR_H_
#define _TSTR_H_
 
// ----- include files -------------------------
#include	<malloc.h>
#include	<stdio.h>
//#include	<conio.h>
#include	<string.h>
#include	<ctype.h>
//#include	<dos.h>
#include	<time.h>
//#include	<windows.h>
//#include	<winioctl.h>

#include	"utypes.h"
#include	"brdapi.h"
#include	"dbprintf.h"


#include <string>

class TString
{
public:

private:
	std::string _s;

public:

	char operator [] (unsigned index)
	{
		return _s[index];
	}

};

#endif
