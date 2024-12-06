//***************** File UseProp.cpp ************************
//
// Полезные функции
//
//**********************************************************

#include "utypes.h"
#include "useful.h"

// функция возвращает степень, в которую надо возвести base, чтобы получить a
double logbase(double a, double base)
{
   return log(a) / log(base);
}

// число val представляет собой степень 2 (1, 2, 4, 8, 16 и т.д.)
// функция возвращает эту степень (0, 1, 2, 3, 4 и т.д.)
int powof2(unsigned int val)
{
	if(!val)
		return -1;
	double rezult = logbase(val, 2);
//	short rezult = 0;
//	__asm	bsf dx, val
//	__asm	mov rezult, dx
    return ROUND(rezult);
}

// 0 - eng; 1 - rus
//ULONG GetRegistryLocalParams()
//{
//	ULONG retcode = 0;
//	HKEY hkey;
//	RegOpenKeyEx(HKEY_LOCAL_MACHINE, _BRDC("System\\CurrentControlSet\\Control\\Nls\\Locale"), 0, KEY_QUERY_VALUE, &hkey);
//	DWORD Type = REG_SZ;
//	DWORD retSize = MAX_PATH;
//	BRDCHAR szBuffer[MAX_PATH];
//	if(RegQueryValueEx(hkey, _BRDC(""), NULL, &Type, (LPBYTE)szBuffer, &retSize) == ERROR_SUCCESS)
//		if(!lstrcmpi(szBuffer, _BRDC("00000419")))
//			retcode = 1;
//	RegCloseKey(hkey);
//	return retcode;
//}

#ifdef _WIN32
//double getRegistry(HKEY hKey, LPSTR ValName, double DefValue)
double getRegistry(HKEY hKey, BRDCHAR *ValName, double DefValue)
{
	DWORD Type = REG_SZ;
	DWORD retSize = 128;
	BRDCHAR szBuffer[128];
	double Res = DefValue;

	if (RegQueryValueEx(hKey,     
					  ValName,           
					  NULL,           
					  &Type,           
					  (LPBYTE)szBuffer,          
					  &retSize) == ERROR_SUCCESS)     
	{
		if (BRDC_strlen(szBuffer)) 
			//Res = strtod(szBuffer, NULL);
			Res = BRDC_strtod(szBuffer, NULL);
	}

	return Res;
}

DWORD getRegistry(HKEY hKey, BRDCHAR *ValName, DWORD DefValue)
{
  DWORD retSize = sizeof(ULONG), Value;

  if (RegQueryValueEx(hKey,     
                      ValName,           
                      NULL,           
                      NULL,           
                      (LPBYTE)&Value,          
                      &retSize) != ERROR_SUCCESS) return DefValue;     
  return Value;
}

int getRegistry(HKEY hKey, BRDCHAR *ValName, int DefValue)
{
  DWORD retSize = sizeof(ULONG), Value;

  if (RegQueryValueEx(hKey,     
                      ValName,           
                      NULL,           
                      NULL,           
                      (LPBYTE)&Value,          
                      &retSize) != ERROR_SUCCESS) return DefValue;     
  return Value;
}

int getRegistry(HKEY hKey, BRDCHAR *ValName, BRDCHAR *ValBuf, BRDCHAR *DefValue)
{
	DWORD Type = REG_SZ;
	DWORD retSize = MAX_PATH;
	BRDCHAR szBuffer[MAX_PATH];

	if (RegQueryValueEx(hKey,     
					  ValName,           
					  NULL,           
					  &Type,           
					  (LPBYTE)szBuffer,          
					  &retSize) == ERROR_SUCCESS)     
	{
		BRDC_strcpy(ValBuf, szBuffer);
	}
	else
	{
		BRDC_strcpy(ValBuf, DefValue);
	}
	retSize = (DWORD)BRDC_strlen(ValBuf);
	return retSize;
}
#endif

//=*************** FindKeyWord **************************
// Поиск ключевого слова в массиве инициализационных данных
// keyWord - искомое ключевое слово
// keyVal - сюда пишется значение ключевого слова
// pInitData - массив инициализационных данных
// size - размер инициализационных данных
//*******************************************************
void FindKeyWord(const BRDCHAR* keyWord, UINT& keyVal, PINIT_Data pInitData, int size)
{
	keyVal = 0xFFFFFFFF;
	// Search Key words
	for(int i = 0; i < size; i++)
	{
		// Go through #begin/#end block 
		if(!BRDC_stricmp(pInitData[i].key, _BRDC("#begin")))	// Found keyword "#begin"
		{
			int beginCnt = 1;
			while(++i < size)
			{
				if(!BRDC_stricmp(pInitData[i].key, _BRDC("#begin")))		// Begin of nested SubSection
					beginCnt++;
				if(!BRDC_stricmp(pInitData[i].key, _BRDC("#end")))			// End of this or nested SubSection
					if(--beginCnt == 0)
						break;
			}
		}
		else
		{	// Compare Key Words
			if(!BRDC_stricmp(pInitData[i].key, keyWord)) 
			{
				BRDCHAR	*endptr;
				//keyVal = (pInitData[i].val[0]) ? strtol( pInitData[i].val, &endptr, 0 ) : 0xFFFFFFFF;
				keyVal = (pInitData[i].val[0]) ? BRDC_strtoul( pInitData[i].val, &endptr, 0 ) : 0xFFFFFFFF;
				break;
			}
		}
	}
}

// ключевое слово - строка
void FindKeyWord(const BRDCHAR* keyWord, BRDCHAR* pkeyVal, PINIT_Data pInitData, int size)
{
//	keyVal = 0xFFFFFFFF;
	pkeyVal[0] = 0;
	// Search Key words
	for(int i = 0; i < size; i++)
	{
		// Go through #begin/#end block 
		if(!BRDC_stricmp(pInitData[i].key, _BRDC("#begin")))	// Found keyword "#begin"
		{
			int beginCnt = 1;
			while(++i < size)
			{
				if(!BRDC_stricmp(pInitData[i].key, _BRDC("#begin")))		// Begin of nested SubSection
					beginCnt++;
				if(!BRDC_stricmp(pInitData[i].key, _BRDC("#end")))			// End of this or nested SubSection
					if(--beginCnt == 0)
						break;
			}
		}
		else
		{	// Compare Key Words
//			if(!_stricmp(pInitData[i].key, keyWord)) 
			if(!BRDC_stricmp(pInitData[i].key, keyWord)) 
			{
				BRDC_strcpy(pkeyVal, pInitData[i].val);
				break;
			}
		}
	}
}

//****************** ~UseProp.cpp ****************************
