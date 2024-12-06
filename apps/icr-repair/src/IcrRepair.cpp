//
// IcrRepair - вносит изменения в ICR базмодуля или субмодуля
// (написана на основе bAiks2Icr.exe)
//
// Sent 2013, Ekkore, InSys
//

#include "brd.h"
#include "icr.h"
#include "sysipc.h"
#include "utypes.h"
#include <locale.h>
#include <stdio.h>

//
// Macros and Types
//
#define MAXDEVICES 32

//
// Global Vars
//

BRD_Handle g_hBrd;
BRDCHAR g_iniFileName[1024] = _BRDC("IcrRepair.ini");
BRDCHAR g_binFileName[1024];
U08 g_aIcrData[1024];
S32 g_nIcrRealSize = -1;

S32 g_nIcrType = -1;
S32 g_nDevType = -1;
S32 g_nTag = -1;
S32 g_nOffset = -1;
S32 g_nTagNumber = -1;
S32 g_nSizeb = -1;
U32 g_nNewValue = -1;
int g_lid = -1;
U32 g_puId = 0;
bool g_isRead = false;

//
// Funx Declarations
//
int OpenDevice(void);
int CloseDevice(void);
int ChangeIcrFromFile(void);
int ChangeIcrFromField(void);
S32 ReadIniFileOption(void);
S32 ReadIniFileField(BRDCHAR *fieldId);
void ParseCommandLine(int argc, char *argv[]);
int GetPuId(U32 *pPuId, U32 icrType);
S32 ReadIdCfgRom(U32 puId, U32 icrType, bool isView);
S32 WriteIdCfgRom(U32 puId, U32 icrType);
void GetInifileString(const BRDCHAR *sectionName, const BRDCHAR *paramName,
                      const BRDCHAR *defValue, BRDCHAR *strValue, int strSize,
                      const BRDCHAR *fileName);

//=************************* main ***********************
//=******************************************************
int BRDC_main(int argc, char *argv[]) {
  int err;

  //
  // Разобрать командную строку
  //

  setlocale(LC_ALL, "Russian");
  BRDC_strcpy(g_iniFileName, _BRDC("IcrRepair.ini"));
  ParseCommandLine(argc, argv);
  if (g_nIcrType == -2)
    return 0;

  //
  // Выполнить инициализацию библиотеки БАРДИ
  //
  err = OpenDevice();
  if (0 > err)
    return 1;

  //
  // Считать ини-файл секцию [Option]
  //
  if (g_nIcrType < 0)
    if (0 > ReadIniFileOption()) {
      printf("ERROR: ini-file '%s' is wrong\n\n", g_iniFileName);
      return 1;
    }

  //
  // Определить puId ICR базмодуля или субмодуля
  //
  if (g_puId == 0)
    err = GetPuId(&g_puId, g_nIcrType);

  //
  // Подготовить новое содержимое ICR
  //
  if (err >= 0) {
    if (g_isRead) {
      ReadIdCfgRom(g_puId, g_nIcrType, true);
      err = -1;
    } else {
      if (g_binFileName[0] != '\0')
        err = ChangeIcrFromFile();
      else {
        err = ReadIdCfgRom(g_puId, g_nIcrType, false);
        if (err >= 0)
          err = ChangeIcrFromField();
      }
    }
  }

  //
  // Выполнить запись в ICR базмодуля или субмодуля
  //
  if (err >= 0)
    err = WriteIdCfgRom(g_puId, g_nIcrType);

  CloseDevice();
  if (err >= 0)
    printf("SUCCESS: ICR writing is OK\n");

  return 0;
}

//=******************* OpenDevice ***********************
//=******************************************************
int OpenDevice(void) {
  S32 nDevNum = 0;
  int err;

  BRD_displayMode(BRDdm_VISIBLE |
                  BRDdm_CONSOLE); // режим вывода информационных сообщений :
                                  // отображать все уровни на консоле

  err = BRD_init(_BRDC("brd.ini"), &nDevNum); // инициализировать библиотеку
  if (!BRD_errcmp(err, BRDerr_OK)) {
    BRDC_printf(_BRDC("ERROR: BARDY Initialization = 0x%X\n"), err);
    return -1;
  }
  if (nDevNum <= 0) {
    BRDC_printf(_BRDC("ERROR: None of Devices\n"));
    return -1;
  }
  fprintf(stderr, "lid %d\n", g_lid);
  if (g_lid < 0) {
    BRD_LidList lidList;
    U32 aLid[10] = {(U32)-1};

    lidList.item = 10; // считаем, что устройств может быть не больше 10
    lidList.pLID = aLid;
    BRD_lidList(lidList.pLID, lidList.item, &lidList.itemReal);
    g_lid = lidList.pLID[0];
  }
  g_hBrd = BRD_open(g_lid, BRDopen_EXCLUSIVE, NULL);
  if (0 >= g_hBrd) {
    BRDC_printf(_BRDC("ERROR: Can't open Devices with LID:%02d\n"), g_lid);
    return -1;
  }

  return 0;
}

//=******************* CloseDevice **********************
//=******************************************************
int CloseDevice(void) {
  BRD_close(g_hBrd);
  BRD_cleanup();

  return 0;
}

//=******************* ChangeIcrFromFile ****************
//=******************************************************
int ChangeIcrFromFile(void) {
  FILE *pFile;
  U32 wSize;

  pFile = BRDC_fopen(g_binFileName, _BRDC("r+b"));
  if (pFile == NULL) {
    printf("ERROR: can't open file %s\n", g_binFileName);
    return -1;
  }

  for (int i = 0; i < 1024; i++)
    g_aIcrData[i] = 0;

  wSize = (U32)fread(g_aIcrData, sizeof(char), 1024, pFile);
  fclose(pFile);

  g_nIcrRealSize = wSize;

  return 0;
}

//=****************** ChangeIcrFromField ****************
//=******************************************************
int ChangeIcrFromField(void) {
  BRDCHAR fullname[FILENAME_MAX];
  U32 nOffset, tag, sizeb, tagNo;
  BRDCHAR sSections[2048], *pSections;

  //
  // Проверить старое содержимое ICR
  //
  if (g_nIcrType == 0) {
    ICR_IdBase *pIcrBase;

    pIcrBase = (ICR_IdBase *)g_aIcrData;
    if (pIcrBase->wTag != BASE_ID_TAG) {
      printf("ERROR: Baseunit ICR is wrong\n\n");
      return -1;
    }
    if (pIcrBase->wDeviceId != g_nDevType) {
      printf("ERROR: Baseunit ICR (0x%04X) doesn't match DevType from ini-file "
             "(0x%04X)\n\n",
             pIcrBase->wDeviceId, g_nDevType);
      return -1;
    }

  } else {
    ICR_IdAdm *pIcrAdm;

    pIcrAdm = (ICR_IdAdm *)g_aIcrData;
    if (pIcrAdm->wTag != ADM_ID_TAG) {
      printf("ERROR: Subunit ICR is wrong\n\n");
      return -1;
    }
    if (pIcrAdm->wType != g_nDevType) {
      printf("ERROR: Subunit ICR (0x%04X) doesn't match DevType from ini-file "
             "(0x%04X)\n\n",
             pIcrAdm->wType, g_nDevType);
      return -1;
    }
  }

  //
  // Go through All Sections
  //

  //
  // Перебрать все Field'ы из ини-файла
  //
  IPC_getCurrentDir(fullname, FILENAME_MAX);
  BRDC_strcat(fullname, _BRDC("/"));
  BRDC_strcat(fullname, g_iniFileName);
  IPC_getPrivateProfileString(NULL, NULL, _BRDC(""), sSections,
                              sizeof(sSections), fullname);

  for (pSections = sSections; '\0' != pSections[0];
       pSections += BRDC_strlen(pSections) + 1) {
    if (BRDC_strstr(pSections, _BRDC("Field")) == NULL)
      continue;
    if (0 > ReadIniFileField(pSections))
      continue;

    //
    // Обработать очередное поле
    //
    printf("Section [%s]\n", pSections);
    nOffset = 0;
    if ((g_nIcrType == 0) || (g_nIcrType == 1)) {
      for (;;) {
        if ((int)nOffset > g_nIcrRealSize - 1)
          break;
        tag = *((U16 *)(g_aIcrData + nOffset));
        sizeb = *((U16 *)(g_aIcrData + nOffset + 2));
        tagNo = *((U08 *)(g_aIcrData + nOffset + 4));
        if (tag == 0 || tag == 0xFFFF)
          break;
        if (tag != g_nTag) {
          nOffset += sizeb + 4;
          continue;
        }
        if ((tagNo == g_nTagNumber) || (g_nTagNumber == -1)) {
          memcpy(g_aIcrData + nOffset + g_nOffset, &g_nNewValue, g_nSizeb);
          nOffset += sizeb + 4;
          printf("Section [%s] was processed\n", pSections);
        }
      }
    }
  }

  return 0;
}

//=******************* ReadIniFileOption ****************
//=******************************************************
S32 ReadIniFileOption(void) {
  BRDCHAR lin[1024], *endptr;
  BRDCHAR fullname[FILENAME_MAX];

  IPC_getCurrentDir(fullname, FILENAME_MAX);
  BRDC_strcat(fullname, _BRDC("/"));
  BRDC_strcat(fullname, g_iniFileName);

  //
  // Read [Option] Section
  //
  GetInifileString(_BRDC("Option"), _BRDC("IcrType"), _BRDC("-1"), lin,
                   sizeof(lin), fullname);
  g_nIcrType = (U32)BRDC_strtoul(lin, &endptr, 0);

  GetInifileString(_BRDC("Option"), _BRDC("DevType"), _BRDC("-1"), lin,
                   sizeof(lin), fullname);
  g_nDevType = (U32)BRDC_strtoul(lin, &endptr, 0);

  GetInifileString(_BRDC("Option"), _BRDC("BinFile"), _BRDC(""), lin,
                   sizeof(lin), fullname);
  BRDC_strcpy(g_binFileName, lin);

  GetInifileString(_BRDC("Option"), _BRDC("TagNumber"), _BRDC("-1"), lin,
                   sizeof(lin), fullname);
  g_nTagNumber = (U32)BRDC_strtoul(lin, &endptr, 0);

  if ((g_nIcrType < 0) || (g_nDevType < 0))
    return -1;
  return 0;
}

//=******************* ReadIniFileField *****************
//=******************************************************
S32 ReadIniFileField(BRDCHAR *fieldId) {
  BRDCHAR lin[1024], *endptr;
  BRDCHAR fullname[FILENAME_MAX];

  IPC_getCurrentDir(fullname, FILENAME_MAX);
  BRDC_strcat(fullname, _BRDC("/"));
  BRDC_strcat(fullname, g_iniFileName);

  GetInifileString(fieldId, _BRDC("Tag"), _BRDC("-1"), lin, sizeof(lin),
                   fullname);
  g_nTag = (U32)BRDC_strtoul(lin, &endptr, 0);

  if (-1 == g_nTag)
    return -1;

  GetInifileString(fieldId, _BRDC("Offset"), _BRDC("-1"), lin, sizeof(lin),
                   fullname);
  g_nOffset = (U32)BRDC_strtoul(lin, &endptr, 0);

  if (-1 == g_nOffset)
    return -1;

  GetInifileString(fieldId, _BRDC("Sizeb"), _BRDC("-1"), lin, sizeof(lin),
                   fullname);
  g_nSizeb = (U32)BRDC_strtoul(lin, &endptr, 0);

  if (-1 == g_nSizeb)
    return -1;

  GetInifileString(fieldId, _BRDC("NewValue"), _BRDC("-1"), lin, sizeof(lin),
                   fullname);
  g_nNewValue = (U32)BRDC_strtoul(lin, &endptr, 0);

  if (-1 == g_nNewValue)
    return -1;

  return 0;
}

//=****************** ParseCommandLine ********************
//=********************************************************
void ParseCommandLine(int argc, char *argv[]) {
  int ii;
  char *pLin, *endptr;

  for (ii = 1; ii < argc; ii++) {
    if (argv[ii][0] != _BRDC('-')) {
      for (int i = 0; i < strlen(argv[ii]); i++) {
        g_iniFileName[i] = argv[ii][i];
        g_iniFileName[i + 1] = _BRDC('\0');
      }
      // BRDC_strcpy(g_iniFileName, argv[ii]);
      break;
    }
    if (argv[ii][1] == 'h') {
      printf("command line arguments:\n");
      printf("-r<b|s|puId> [filename]  = read from Base module ICR, Submodule "
             "ICR,\n\t\t\t\t<puId> to console [and filename]\n\n");
      printf("-w<b|s|puId> <filename>  = write to Base module ICR, Submodule "
             "ICR,\n\t\t\t\t<puId> from <filename>\n\n");
      printf("FileName  = set FileName as ini-file name\n\n");
      printf("-b<lid>  = set lid number\n");
      g_nIcrType = -2;
    }
    if (tolower(argv[ii][1]) == 'b') {
      pLin = &argv[ii][2];
      if (argv[ii][2] == '\0') {
        ii++;
        pLin = argv[ii];
      }
      fprintf(stderr, "plin=%s\n", pLin);
      g_lid = strtoul(pLin, &endptr, 0);
    }
    if (tolower(argv[ii][1]) == 'r') // чтение
    {
      g_isRead = true;
      if (tolower(argv[ii][2]) == 'b') // базовый модуль
      {
        g_nIcrType = 0;
      } else if (tolower(argv[ii][2]) == 's') // субмодуль
      {
        g_nIcrType = 1;
      } else // задан puId
      {
        g_nIcrType = 2;
        char puId[255] = {0};
        for (UINT i = 2; i < strlen(argv[ii]); i++)
          puId[i - 2] = (char)argv[ii][i];
        g_puId = (U32)atoi(puId);
      }
      if (strlen(argv[ii]) > 3 &&
          (tolower(argv[ii][2]) == 's' ||
           tolower(argv[ii][2]) == 'b')) // имя файла в текущем параметре
        for (UINT i = 3; i < strlen(argv[ii]); i++)
          g_binFileName[i - 3] = argv[ii][i];
      else // имя файла в следующем параметре
        if (ii + 1 < argc) {
          ii++;
          for (UINT i = 0; i < strlen(argv[ii]); i++)
            g_binFileName[i] = argv[ii][i];
        }
    }
    if (tolower(argv[ii][1]) == _BRDC('w')) // запись
    {
      g_isRead = false;
      if (tolower(argv[ii][2]) == _BRDC('b')) // базовый модуль
      {
        g_nIcrType = 0;
      } else if (tolower(argv[ii][2]) == _BRDC('s')) // субмодуль
      {
        g_nIcrType = 1;
      } else // задан puId
      {
        g_nIcrType = 2;
        char puId[255] = {0};
        for (UINT i = 2; i < strlen(argv[ii]); i++)
          puId[i - 2] = (char)argv[ii][i];
        g_puId = (U32)atoi(puId);
      }
      if (strlen(argv[ii]) > 3 &&
          (tolower(argv[ii][2]) == 's' ||
           tolower(argv[ii][2]) == 'b')) // имя файла в текущем параметре
        for (UINT i = 3; i < strlen(argv[ii]); i++)
          g_binFileName[i - 3] = argv[ii][i];
      else if (ii + 1 < argc) // имя файла в следующем параметре
      {
        ii++;
        for (UINT i = 0; i < strlen(argv[ii]); i++)
          g_binFileName[i] = argv[ii][i];
      } else
        printf("ERROR: no filename in command line\n");
    }
  }
}

//=************************ GetPuId *********************
//=******************************************************
int GetPuId(U32 *pPuId, U32 icrType) {
  int ii;
  BRD_PuList PuListTmp[1], *pPuList;
  U32 itemReal, itemsCnt;

  if ((icrType != READ_WRITE_BASEMODULE) && (icrType != READ_WRITE_SUBMODULE)) {
    printf("ERROR GetPuId(): Wrong Device Type\n");
    return -1;
  }

  //
  // Считать список ПУ
  //
  BRD_puList(g_hBrd, PuListTmp, 1, &itemReal);
  pPuList = new BRD_PuList[itemReal];
  itemsCnt = itemReal;
  BRD_puList(g_hBrd, pPuList, itemsCnt, &itemReal);
  if (itemReal > itemsCnt) {
    printf("ERROR GetPuId(): Can't properly allocate PU list\n");
    delete[] pPuList;
    return -1;
  }

  //
  // Ищем puId ICR базового модуля
  //
  *pPuId = 0;
  if (icrType == READ_WRITE_BASEMODULE) {
    for (ii = 0; ii < (int)itemReal; ii++) {
      if (pPuList[ii].puCode == BASE_ID_TAG) {
        *pPuId = pPuList[ii].puId;
        break;
      }
    }
    if (*pPuId == 0) {
      for (ii = 0; ii < (int)itemReal; ii++) {
        if (pPuList[ii].puId == 0x1) {
          *pPuId = pPuList[ii].puId;
          break;
        }
      }
    }
  }

  //
  // Ищем puId ICR субмодуля
  //
  if (icrType == READ_WRITE_SUBMODULE) {
    for (ii = 0; ii < (int)itemReal; ii++) {
      if (pPuList[ii].puCode == ADM_ID_TAG) {
        *pPuId = pPuList[ii].puId;
        break;
      }
    }
  }

  delete[] pPuList;

  if (0 == *pPuId) {
    printf("ERROR GetPuId(): Can't find ICR PU\n");
    return -2;
  }
  return 0;
}

//***************************************************************************************
//  ReadIdCfgRom - функция служит для чтения ППЗУ
//***************************************************************************************
S32 ReadIdCfgRom(U32 puId, U32 icrType, bool isView) {
  S32 err = BRDerr_OK;
  U32 tag;
  FILE *file;

  // if( (icrType != READ_WRITE_BASEMODULE) && (icrType != READ_WRITE_SUBMODULE)
  // )
  //{
  //	printf("ERROR ReadIdCfgRom(): Wrong Device Type\n");
  //	return -1;
  // }

  //
  // считывание ICR
  //
  err = BRD_puRead(g_hBrd, puId, 0, g_aIcrData, BASEMOD_CFGMEM_SIZE);
  if (!BRD_errcmp(err, BRDerr_OK)) {
    printf("ERROR: Can't read from ICR (puId=%d)\n", puId);
    return -1;
  }

  //
  // Проверить содержимое ICR
  //
  g_nIcrRealSize = 0;
  tag = *(U16 *)g_aIcrData;
  g_nIcrRealSize = *(U16 *)(g_aIcrData + 4);
  if (icrType == READ_WRITE_BASEMODULE) {
    if (tag != BASE_ID_TAG) {
      printf("ERROR: Base module ICR contains invalid data (0x%X)\n", tag);
      g_nIcrRealSize = 0;
      if (!isView)
        return -1;
    }
  }
  if (icrType == READ_WRITE_SUBMODULE) {
    if (tag != ADM_ID_TAG) {
      printf("ERROR: Submodule ICR contains invalid data (0x%X)\n", tag);
      g_nIcrRealSize = 0;
      if (!isView)
        return -1;
    }
  }
  if (isView) {
    if (g_nIcrRealSize == 0)
      g_nIcrRealSize = 1024;
    printf("0x00:");
    for (int i = 0; i < g_nIcrRealSize; i++) {
      printf("%02X ", *((U08 *)(g_aIcrData + i)));
      if ((((i + 1) % 16) == 0) && (i > 0))
        printf("\n0x%02X:", (i / 16));
    }
    printf("\n");
    if (g_binFileName[0] != 0) {
      file = BRDC_fopen(g_binFileName, _BRDC("w+b"));
      for (int i = 0; i < g_nIcrRealSize; i++)
        fwrite(&g_aIcrData[i], 1, 1, file);
      fclose(file);
    }
  }
  return 0;
}

//********************** WriteIdCfgRom ********************
//*********************************************************
S32 WriteIdCfgRom(U32 puId, U32 icrType) {
  S32 err = BRDerr_OK;
  U32 tag;

  // if( (icrType != READ_WRITE_BASEMODULE) && (icrType != READ_WRITE_SUBMODULE)
  // )
  //{
  //	printf("ERROR ReadIdCfgRom(): Wrong Device Type\n");
  //	return -1;
  // }

  //
  // Проверить содержимое ICR
  //
  g_nIcrRealSize = 0;
  tag = *(U16 *)g_aIcrData;
  g_nIcrRealSize = *(U16 *)(g_aIcrData + 4);
  if (icrType == READ_WRITE_BASEMODULE) {
    if (tag != BASE_ID_TAG) {
      printf("ERROR: Base module image ICR contains invalid data (0x%X)\n",
             tag);
      return -1;
    }
  }
  if (icrType == READ_WRITE_SUBMODULE) {
    if (tag != ADM_ID_TAG) {
      printf("ERROR: Submodule image ICR contains invalid data (0x%X)\n", tag);
      return -1;
    }
  }
  //
  // Прописать ICR
  //
  BRD_puEnable(g_hBrd, puId);
  err = BRD_puWrite(g_hBrd, puId, 0, g_aIcrData, g_nIcrRealSize);
  if (!BRD_errcmp(err, BRDerr_OK)) {
    printf("ERROR: Can't write to (puId=%d)\n", puId);
    return -1;
  }

  return 0;
}

//=************************* GetInifileString *************
//=********************************************************
// Получить параметр типа строка из ini-файла:
//    FileName - имя ini-файла (с путем, если нужно)
//    SectionName - название секции
//    ParamName - название параметра (ключа)
//    defValue - значение параметра по-умолчанию (если параметра в файле нет)
//    strValue - значение параметра
//    strSize - максимальная длина параметра
void GetInifileString(const BRDCHAR *sectionName, const BRDCHAR *paramName,
                      const BRDCHAR *defValue, BRDCHAR *strValue, int strSize,
                      const BRDCHAR *fileName) {
  BRDCHAR *pChar;
  int ii;
  int linSize;

  IPC_getPrivateProfileString(sectionName, paramName, defValue, strValue,
                              strSize, fileName);

  //
  // Удалить комментарий из строки
  //
  pChar = BRDC_strchr(strValue, ';');
  if (pChar)
    *pChar = 0;
  pChar = BRDC_strchr(strValue, '/');
  if (pChar)
    if (pChar[1] == '/')
      *pChar = 0;

  //
  // Удалить пробелы в конце строки
  //
  linSize = BRDC_strlen(strValue);
  for (ii = linSize - 1; ii > 1; ii--)
    if (strValue[ii] != ' ' && strValue[ii] != '\t') {
      strValue[ii + 1] = 0;
      break;
    }
}

//
// End of File
//
