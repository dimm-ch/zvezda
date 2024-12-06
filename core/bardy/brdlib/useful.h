//**************************************************************************
//					Полезные макросы и функции
//**************************************************************************

#ifndef _USEFUL_H_
#define	_USEFUL_H_

#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_NAMELEN 32
#define MAX_DESCRIPLEN 128
#define MAX_STRLEN 255

int powof2(unsigned int val);

//ULONG GetRegistryLocalParams();

#ifdef _WIN32
double getRegistry(HKEY hKey, LPTSTR ValName, double DefValue);
DWORD getRegistry(HKEY hKey, LPTSTR ValName, DWORD DefValue);
int getRegistry(HKEY hKey, LPTSTR ValName, int DefValue);
int getRegistry(HKEY hKey, LPTSTR ValName, LPTSTR ValBuf, LPTSTR DefValue);
#endif

#pragma pack(push, 1)    

typedef struct _INIT_Enum {
	ULONG val; 
	BRDCHAR name[MAX_PATH];
} INIT_Enum, *PINIT_Enum;

typedef struct _INIT_Data { 
	BRDCHAR key[32]; 
	BRDCHAR val[MAX_PATH];
} INIT_Data, *PINIT_Data;

#pragma pack(pop)

void FindKeyWord(const BRDCHAR* keyWord, UINT& keyVal, PINIT_Data pInitData, int size);
void FindKeyWord(const BRDCHAR* keyWord, BRDCHAR* pkeyVal, PINIT_Data pInitData, int size);

#define ROUND(x) ((int)floor((x) + 0.5))
#define ROUND64(x) ((__int64)floor((x) + 0.5))
#define LOG2(x) (log(x)/log(2.0))
#ifndef PI
 #define PI (4.0*atan(1.0))
 #define TWO_PI (8.0*atan(1.0))
#endif

#define CLEAR 0
#define SET 1

#define ARR_SIZE(arr) (sizeof(arr)/sizeof(arr[0]))

#ifdef _ISA
  #define SIZE_OF_REG	sizeof(WORD)
#else
  #define SIZE_OF_REG	sizeof(DWORD)
#endif

#endif // _USEFUL_H_
