#pragma once

#include "brdapi.h"
#include "gipcy.h"

// All total definition

extern S32 x_DevNum; // кол-во у-в
extern BRD_Handle x_hADC; // сервис АЦП
extern BRD_Handle x_hDAC; // сервис ЦAП
extern BRD_Handle x_hREG;
extern int x_lid;
extern BRD_Handle x_handleDevice;
extern int x_mode;

static int version_srv_hi = 0;
static int version_srv_lo = 5;