/*
 ****************** File getprof.c *************************
 *
 *        Z Extension of C/C++ Runtime Library.
 * 
 *    Function to get private profile string from file
 *  (source compatible with Windows analogues).
 * 
 *        (Version of 23-Oct-95 by AAZ)
 *
 ***********************************************************
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utypes.h"


#ifdef _WIN32

int getRegistryString(HKEY  hKey, 
                      BRDCHAR *ValName, 
                      BRDCHAR *Buf, 
                      int   Len, 
                      BRDCHAR *DefValue)
{
  DWORD retSize = Len;

  if (RegQueryValueEx(hKey,     
                      ValName,           
                      NULL,           
                      NULL,           
                      (PBYTE)Buf,          
                      &retSize) != ERROR_SUCCESS) 
    BRDC_strcpy(Buf, DefValue);

  return (int)retSize;
}

double getRegistryDouble(HKEY hKey, BRDCHAR *ValName, double DefValue)
{
  DWORD		retSize = 64;
  char		szBuffer[64];
  double	Res = DefValue;

  if (RegQueryValueEx(hKey,     
                      ValName,           
                      NULL,           
                      NULL,           
                      (LPBYTE)szBuffer,          
                      &retSize) == ERROR_SUCCESS)     
  {
    if (strlen(szBuffer)) 
      Res = strtod(szBuffer, NULL);
  }

  return Res;
}

DWORD getRegistryLong(HKEY hKey, BRDCHAR *ValName, DWORD DefValue)
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

#endif /* _WIN32 */


/*
* End of File 
*/

