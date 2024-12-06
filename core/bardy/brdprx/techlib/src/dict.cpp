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

// ----- include files -------------------------

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
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

#include "dict.h"

TDict::TDict()
{
    nProp = 0;
    nDict = 0;
}

TDict::TDict(TBRD_InitData* pInitData, S32 size)
{
    nProp = 0;
    nDict = 0;

    scan(pInitData, size);
}

TDict::~TDict()
{
    int ii;

    for (ii = 0; ii < nDict; ii++) {
        if (pDictArray[ii])
            delete pDictArray[ii];
    }
}

void TDict::setDictByName(BRDCHAR* name, TDict* dict)
{
    pDictArray[nDict] = dict;
    dict->name = name;

    nDict++;
}

TDict* TDict::getDictByName(BRDCHAR* name)
{
    S32 isIntoBlock = 0; // Where Driver is Described: 0 - LID-Section, 1 - #begin/#end Block

    int ii;
    //
    //=== Search Key words
    //
    for (ii = 0; ii < nDict; ii++) {
        if (!BRDC_stricmp(pDictArray[ii]->name, name))
            return pDictArray[ii]; // Found keyword
    }

    return NULL;
}

int TDict::scan(TBRD_InitData* pInitData, S32 size)
{
    S32 isIntoBlock = 0; // Where Driver is Described: 0 - LID-Section, 1 - #begin/#end Block

    int ii;

    //
    //=== If Driver is Described in the #begin/#end Block
    //
    ii = 0;

    if (!BRDC_stricmp(pInitData[ii].key, _BRDC("#begin"))) {
        ii++;
        isIntoBlock = 1;
    }

    //
    //=== Search Key words
    //
    for (; ii < size; ii++) { //
        //=== Go through #begin/#end block
        //
        if (!BRDC_stricmp(pInitData[ii].key, _BRDC("#begin"))) // Found keyword "#begin"
        {
            TDict* dict = new TDict();

            dict->name = pInitData[ii].val;

            ii += dict->scan(&pInitData[ii], size - ii);

            pDictArray[nDict] = dict;
            nDict++;

            continue;
        }

        //
        // === Keyword "#end" - end of Driver SubSection
        // (Driver was Described in the #begin/#end Block, not in the [LID:##] Section)
        //

        if (!BRDC_stricmp(pInitData[ii].key, _BRDC("#end"))) {
            if (isIntoBlock)
                break;

            continue;
        }

        TBRD_InitData* pRec = new TBRD_InitData();

        BRDC_strcpy(pRec->key, pInitData[ii].key);
        BRDC_strcpy(pRec->val, pInitData[ii].val);

        this->pPropArray[this->nProp] = pRec;
        this->nProp++;
    }

    return ii;
}

int TDict::getInt(BRDCHAR* name, int def)
{
    TBRD_InitData* rec = find(name);
    BRDCHAR* endptr;

    if (rec == NULL)
        return def;

    BRDCHAR* val = rec->val;

    if (val == NULL)
        return def;

    return BRDC_strtoul(val, &endptr, 0);
}

BRDCHAR* TDict::getString(BRDCHAR* name, BRDCHAR* def)
{
    TBRD_InitData* rec = find(name);

    if (rec == NULL)
        return def;

    BRDCHAR* val = rec->val;

    return val;
}

void TDict::setString(BRDCHAR* name, BRDCHAR* val)
{
    TBRD_InitData* rec = find(name);

    if (rec == NULL) {

        ownProps.push_back(TBRD_InitData());

        rec = &ownProps.back();
        pPropArray[nProp] = rec;
        nProp++;
    }

    BRDC_strcpy(rec->key, name);
    BRDC_strcpy(rec->val, val);

    return;
}

TBRD_InitData* TDict::find(BRDCHAR* name)
{
    S32 isIntoBlock = 0; // Where Driver is Described: 0 - LID-Section, 1 - #begin/#end Block

    int ii;
    //
    //=== Search Key words
    //
    for (ii = 0; ii < nProp; ii++) {
        if (!BRDC_stricmp(pPropArray[ii]->key, name))
            return pPropArray[ii]; // Found keyword
    }

    return NULL;
}
