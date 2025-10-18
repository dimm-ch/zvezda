#include "total.h"

//=****************** GetInifileString ********************
//=********************************************************
S32 GetInifileString(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    BRDCHAR* strValue, int strSize, const BRDCHAR* sFullName)
{
    BRDCHAR* pChar;
    int ii;
    int len;

    int rv = IPC_getPrivateProfileString(sSectionName, sParamName, defValue, strValue, strSize, sFullName);
    if (rv == IPC_BAD_INI_FILE) {
        printf("<ERR> GetInifileString: bad ini-file ..\n");
        return -1;
    } else if (rv == 0) {
        printf("<SRV> GetInifileString: parameter %s - not found .. (use default value = %s) \n", sParamName, defValue);
    } else {
        printf("<SRV> GetInifileString: parameter %s = %s \n", sParamName, strValue);
    }
    //
    // удалить комментарий из строки
    //
    pChar = BRDC_strchr(strValue, ';');
    if (pChar)
        *pChar = 0;
    pChar = BRDC_strchr(strValue, '/');
    if (pChar)
        if (*(pChar + 1) == '/')
            *pChar = 0;

    //
    // Удалить пробелы в конце строки
    //
    len = (int)BRDC_strlen(strValue);
    for (ii = len - 1; ii > 1; ii--)
        if (strValue[ii] != ' ' && strValue[ii] != '\t') {
            strValue[ii + 1] = 0;
            break;
        }

    return 0;
}

//=****************** GetInifileInt ********************
//=********************************************************
S32 GetInifileInt(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    int& value, const BRDCHAR* sFullName)
{
    BRDCHAR* pChar;
    int ii;
    int len;
    BRDCHAR sBuffer[1024];

    int rv = IPC_getPrivateProfileString(sSectionName, sParamName, defValue, sBuffer, sizeof(sBuffer), sFullName);
    if (rv == IPC_BAD_INI_FILE) {
        printf("<ERR> GetInifileInt: bad ini-file ..\n");
        return -1;
    }

    if (rv == 1) {

        // удалить комментарий из строки
        pChar = BRDC_strchr(sBuffer, ';');
        if (pChar)
            *pChar = 0;
        pChar = BRDC_strchr(sBuffer, '/');
        if (pChar)
            if (*(pChar + 1) == '/')
                *pChar = 0;
        // Удалить пробелы в конце строки
        len = (int)BRDC_strlen(sBuffer);
        for (ii = len - 1; ii > 1; ii--)
            if (sBuffer[ii] != ' ' && sBuffer[ii] != '\t') {
                sBuffer[ii + 1] = 0;
                break;
            }

        value = BRDC_atoi(sBuffer);
        printf("<SRV> GetInifileInt: parameter %s = %d \n", sParamName, value);
        return 1;
    } else {
        value = BRDC_atoi(defValue);
        printf("<SRV> GetInifileInt: parameter %s - not found .. (use default value = %d) \n", sParamName, value);
        return 0;
    }
}

//=****************** GetInifileBig ********************
//=********************************************************
S32 GetInifileBig(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    long long& value, const BRDCHAR* sFullName)
{
    BRDCHAR* pChar;
    int ii;
    int len;
    BRDCHAR sBuffer[1024];

    int rv = IPC_getPrivateProfileString(sSectionName, sParamName, defValue, sBuffer, sizeof(sBuffer), sFullName);
    if (rv == IPC_BAD_INI_FILE) {
        printf("<ERR> GetInifileInt: bad ini-file ..\n");
        return -1;
    }

    if (rv == 1) {

        // удалить комментарий из строки
        pChar = BRDC_strchr(sBuffer, ';');
        if (pChar)
            *pChar = 0;
        pChar = BRDC_strchr(sBuffer, '/');
        if (pChar)
            if (*(pChar + 1) == '/')
                *pChar = 0;
        // Удалить пробелы в конце строки
        len = (int)BRDC_strlen(sBuffer);
        for (ii = len - 1; ii > 1; ii--)
            if (sBuffer[ii] != ' ' && sBuffer[ii] != '\t') {
                sBuffer[ii + 1] = 0;
                break;
            }

        value = strtoll(sBuffer, NULL, 10);
        printf("<SRV> GetInifileInt: parameter %s = %lld \n", sParamName, value);
        return 1;
    } else {
        value = strtoll(defValue, NULL, 10);
        printf("<SRV> GetInifileInt: parameter %s - not found .. (use default value = %lld) \n", sParamName, value);
        return 0;
    }
}

//=****************** GetInifileFloat ********************
//=********************************************************
S32 GetInifileFloat(const BRDCHAR* sSectionName, const BRDCHAR* sParamName, const BRDCHAR* defValue,
    double& value, const BRDCHAR* sFullName)
{
    BRDCHAR* pChar;
    int ii;
    int len;
    BRDCHAR sBuffer[1024];

    int rv = IPC_getPrivateProfileString(sSectionName, sParamName, defValue, sBuffer, sizeof(sBuffer), sFullName);
    if (rv == IPC_BAD_INI_FILE) {
        printf("<ERR> GetInifileFloat: bad ini-file ..\n");
        return -1;
    }

    if (rv == 1) {

        // удалить комментарий из строки
        pChar = BRDC_strchr(sBuffer, ';');
        if (pChar)
            *pChar = 0;
        pChar = BRDC_strchr(sBuffer, '/');
        if (pChar)
            if (*(pChar + 1) == '/')
                *pChar = 0;
        // Удалить пробелы в конце строки
        len = (int)BRDC_strlen(sBuffer);
        for (ii = len - 1; ii > 1; ii--)
            if (sBuffer[ii] != ' ' && sBuffer[ii] != '\t') {
                sBuffer[ii + 1] = 0;
                break;
            }

        value = BRDC_atof(sBuffer);
        printf("<SRV> GetInifileFloat: parameter %s = %.2f \n", sParamName, value);
        return 1;
    } else {
        value = BRDC_atof(defValue);
        printf("<SRV> GetInifileFloat: parameter %s - not found .. (use default value = %.2f) \n", sParamName, value);
        return 0;
    }
}
