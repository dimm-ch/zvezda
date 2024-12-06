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

#ifndef _IO_H_
#define _IO_H_
 
// ----- include files -------------------------
#include	<malloc.h>
#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>
#include	<time.h>

#include	"utypes.h"
#include	"brdapi.h"
//#include	"dbprintf.h"

class TIO
{
private:
	char *_in;
	int _insize;
	int _pos;
public:

	TIO( char *in, int insize )
	{
		_in = in;
		_insize = insize;		
		
		_pos = 0;
	}

	U32 readU32()
	{
		U32 out = *(U32*)(_in + _pos);
		_pos += sizeof(U32);
        return out;
	}	
	
	U32 writeU32( U32 val )
	{
		*(U32*)(_in + _pos) = val;
		_pos += sizeof(U32);

		return 0;
	}

	char *pointer()
	{
		return (_in + _pos);
	}

	int bufSize()
	{
		return _insize;
	}

	int position()
	{
		return _pos;
	}
};

#endif
