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

#ifndef _DICT_H_
#define _DICT_H_

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

#include <vector>

#include "brdapi.h"
#include "dbprintf.h"
#include "utypes.h"

class TDict {
public:
    BRDCHAR* name;

    TDict();
    TDict(TBRD_InitData* pInitData, S32 size);

    ~TDict();

    int getInt(BRDCHAR* name, int def = 0);
    BRDCHAR* getString(BRDCHAR* name, BRDCHAR* def = NULL);

    void setInt(BRDCHAR* name, int val);
    void setString(BRDCHAR* name, BRDCHAR* val);

    TDict* getDictByName(BRDCHAR* name);

    void setDictByName(BRDCHAR* name, TDict* dict);

    TDict* getDict(int index) { return pDictArray[index]; };
    int getDictCount() { return nDict; };

    int scan(TBRD_InitData* pInitData, S32 size);

    TBRD_InitData* getProp(int index) { return pPropArray[index]; };
    int getPropCount() { return nProp; };

private:
    TBRD_InitData* pPropArray[32];
    S32 nProp;

    TDict* pDictArray[32];
    S32 nDict;

    std::vector<TBRD_InitData> ownProps;

    TBRD_InitData* find(BRDCHAR* name);
};

#endif
