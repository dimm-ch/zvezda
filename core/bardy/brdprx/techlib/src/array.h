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

#ifndef _TARRAY_H_
#define _TARRAY_H_

// ----- include files -------------------------
#include <malloc.h>
#include <stdio.h>
//#include	<conio.h>
#include <ctype.h>
#include <string.h>
//#include	<dos.h>
#include <time.h>
//#include	<windows.h>
//#include	<winioctl.h>

#include "brdapi.h"
#include "dbprintf.h"
#include "utypes.h"

#include <list>
#include <vector>

template <class T>
class TArray {
public:
private:
    std::vector<T> _list;

public:
    void push(T value)
    {
        _list.push_back(value);
    }

    T pop()
    {
        T out = _list.front();

        _list.pop_front();

        return out;
    }

    int get_length()
    {
        return _list.size();
    }

    T& operator[](unsigned index)
    {
        return _list[index];
    }
};

#endif
