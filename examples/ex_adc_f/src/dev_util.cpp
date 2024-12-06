#include "dev_util.h"
#include <vector>

const U32 ADM2IFnr_SPDDEVICE = 0x203;
const U32 ADM2IFnr_SPDCTRL = 0x204;
const U32 ADM2IFnr_SPDADDR = 0x205;
const U32 ADM2IFnr_SPDDATAL = 0x206;
const U32 ADM2IFnr_SPDDATAH = 0x207;

const U32 ADM2IFnr_STATUS = 0; // (0x00) Status register
const U32 ADM2IFnr_DATA = 1; // (0x02) Data register
const U32 ADM2IFnr_CMDADR = 2; // (0x04) Address command register
const U32 ADM2IFnr_CMDDATA = 3; // (0x06) Data command register

S32 regWriteIndir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val)
{
    BRD_Reg reg;
    reg.bytes = sizeof(U32);
    reg.reg = rgnum;
    reg.tetr = trdNo;
    reg.val = val;

    return BRD_ctrl(hSrv, NODE0, BRDctrl_REG_WRITEIND, &reg);
}

U32 regReadIndir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status)
{
    BRD_Reg reg;
    reg.bytes = sizeof(U32);
    reg.reg = rgnum;
    reg.tetr = trdNo;
    reg.val = 0;

    status = BRD_ctrl(hSrv, NODE0, BRDctrl_REG_READIND, &reg);
    return (reg.val & 0xFFFF);
}

S32 regWriteDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, U32 val)
{
    BRD_Reg reg;
    reg.bytes = sizeof(U32);
    reg.reg = rgnum;
    reg.tetr = trdNo;
    reg.val = val;

    return BRD_ctrl(hSrv, NODE0, BRDctrl_REG_WRITEDIR, &reg);
}

U32 regReadDir(BRD_Handle hSrv, S32 trdNo, S32 rgnum, S32& status)
{
    BRD_Reg reg;
    reg.bytes = sizeof(U32);
    reg.reg = rgnum;
    reg.tetr = trdNo;
    reg.val = 0;

    status = BRD_ctrl(hSrv, NODE0, BRDctrl_REG_READDIR, &reg);
    return reg.val;
}

using namespace std;

std::string getStrOpenModeDevice(U32 nMode)
{
    string s = "INI";
    switch (nMode) {
    case BRDopen_EXCLUSIVE:
        s = (_BRDC("EXCLUSIVE"));
        break;
    case BRDopen_SHARED:
        s = (_BRDC("SHARED"));
        break;
    case BRDopen_SPY:
        s = (_BRDC("SPY"));
        break;
    default:
        s = (_BRDC(" -???-"));
        break;
    }
    return s;
}

std::string getStrCaptureModeService(U32 nMode)
{
    string s = "INI";
    switch (nMode) {
    case BRDcapt_SPY:
        s = (_BRDC("SPY"));
        break;
    case BRDcapt_SHARED:
        s = (_BRDC("SHARED"));
        break;
    case BRDcapt_EXCLUSIVE:
        s = (_BRDC("EXCLUSIVE"));
        break;
    default:
        s = (_BRDC(" -???-"));
    }
    return s;
}

S32 processReg(commandLineParams& params)
{
    S32 status = 0;
    params.indirect = (params.reg >= 0x100) ? true : false;
    if (params.indirect) {
        if (params.write)
            status = regWriteIndir(params.hService, params.tetrad, params.reg, params.value);
        else
            params.value = regReadIndir(params.hService, params.tetrad, params.reg, status);
    } else {
        if (params.write)
            status = regWriteDir(params.hService, params.tetrad, params.reg, params.value);
        else
            params.value = regReadDir(params.hService, params.tetrad, params.reg, status);
    }
    return status;
}

// SPD
///
/// \brief      Чтение регистра устройства на SPD шине конкретной тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      dev       Номер устройства/группы
/// \param      numb      Номер устройства в группе
/// \param      addr      Адрес регистра
///
/// \return     Значение из регистра
///
U32 SpdRead(BRD_Handle hService, size_t tetr_num, size_t dev, size_t numb, size_t addr)
{
    U32 regSTAT;
    S32 status;

    do { // ожидаем готовность тетрады
        regSTAT = regReadDir(hService, tetr_num, ADM2IFnr_STATUS, status);
    } while ((regSTAT & 1) != 1);

    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDDEVICE, dev);
    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDADDR, addr);
    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDCTRL, 1 | (numb << 4));

    do { // ожидаем готовность тетрады
        regSTAT = regReadDir(hService, tetr_num, ADM2IFnr_STATUS, status);
    } while ((regSTAT & 1) != 1);

    return regReadIndir(hService, tetr_num, ADM2IFnr_SPDDATAL, status) | regReadIndir(hService, tetr_num, ADM2IFnr_SPDDATAH, status) << 16;
}

///
/// \brief      Запись в регистр устройства на SPD шине конкретной тетрады
///
/// \param      tetr_num  Порядковый номер тетрады
/// \param      dev       Номер устройства/группы
/// \param      numb      Номер устройства в группе
/// \param      addr      Адрес регистра
/// \param      val       Значение для записи в регистр
///
/// \return     status
///
S32 SpdWrite(BRD_Handle hService, size_t tetr_num, size_t dev, size_t numb, size_t addr, size_t val)
{
    U32 regSTAT;
    S32 status;

    do { // ожидаем готовность тетрады
        regSTAT = regReadDir(hService, tetr_num, ADM2IFnr_STATUS, status);
    } while ((regSTAT & 1) != 1);

    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDDEVICE, dev);
    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDADDR, addr);
    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDDATAL, val & 0xFFFF);
    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDDATAH, val >> 16);
    regWriteIndir(hService, tetr_num, ADM2IFnr_SPDCTRL, 2 | (numb << 4));

    do { // ожидаем готовность тетрады
        regSTAT = regReadDir(hService, tetr_num, ADM2IFnr_STATUS, status);
    } while ((regSTAT & 1) != 1);

    return status;
}

#define MAX_PUs 16
const U16 PLD_CFG_TAG = 0x0500;

void printInfo(commandLineParams params, BRD_Handle hDev)
{
    if (hDev <= 0) {
        printf("ERR: bad handle device!\n");
        return;
    }
    BRD_Version ver;
    BRD_version(hDev, &ver);
    BRDC_printf("-- Version: Shell v.%d.%d, Driver v.%d.%d\n",
        ver.brdMajor, ver.brdMinor, ver.drvMajor, ver.drvMinor);
    BRD_PuList PuList[MAX_PUs];
    U32 ItemReal;
    BRD_puList(hDev, PuList, MAX_PUs, &ItemReal);
    if (ItemReal <= 8) {
        BRDC_printf("----------------------------------------------------------\n");
        BRDC_printf("\n * PU lists : \n");
        for (U32 j = 0; j < ItemReal; j++) {
            BRDC_printf(_BRDC("* PU # %d: %s, Id = %d, Code = %x, Attr = %x \n"),
                j, PuList[j].puDescription, PuList[j].puId, PuList[j].puCode, PuList[j].puAttr);
            if (PuList[j].puCode == PLD_CFG_TAG && PuList[j].puId == 0x100) {
                U32 PldState;
                BRD_puState(hDev, PuList[j].puId, &PldState);
                BRDC_printf(_BRDC("  PU state: ADM PLD State = %d\n"), PldState);
                if (!PldState) {
                    BRDC_printf(_BRDC("  This PU don't loaded ..\n"));
                }
            }
            BRDC_printf("----------------------------------------------------------\n");
        }
    }
    U32 status;
    BRD_Reg regdata;
    S32 iTetr = 0;
    for (iTetr = 0; iTetr < 14; iTetr++) {
        regdata.tetr = iTetr;
        printf("-TETR = %d -------------------------------------------\n", iTetr);
        regdata.reg = 0x100;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x101;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x102;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x103;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x104;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x105;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x106;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x107;
        printConstRegsFromTetrads(params.hService, regdata);
        regdata.reg = 0x108;
        printConstRegsFromTetrads(params.hService, regdata);
    }
}

struct ListsPrintf {
    std::string name;
    int num;
    U32 value;
};

U32 selectListValue(std::string title, std::vector<ListsPrintf>& list, string& selName)
{
m1:
    printf("\n%s :\n", title.c_str());
    for (size_t i = 0; i < list.size(); i++) {
        printf(" [%d] - %s \n", list[i].num, list[i].name.c_str());
    }
    printf("Select number (into []) : ");
    std::string com;
    getline(std::cin, com);
    int sel = atoi(com.c_str());
    for (int j = 0; j < list.size(); j++) {
        if (list[j].num == sel) {
            selName = list[j].name;
            return list[j].value;
        }
    }
    printf("ERR: bad choice, try again ..\n ");
    goto m1;
}

int selectTetrad(std::string& name);

void spdAccess(commandLineParams& params, BRD_Handle hDev)
{
    size_t dev = 0, numb = 0;
    std::vector<ListsPrintf> list;
    ListsPrintf l;
    string nameTetr, nameGroup, nameSubGroup;

    params.tetrad = selectTetrad(nameTetr);

    switch (params.tetrad) {
    case 0: // MAIN
    {
        l.num = 0;
        l.name = "Generator Si571/570";
        l.value = 0;
        list.push_back(l);
        l.num = 1;
        l.name = "Switcher ADN4600";
        l.value = 1;
        list.push_back(l);
        dev = selectListValue("Select device", list, nameGroup);
        numb = dev == 0 ? 0x49 : 0x48;
    } break;
    case 1: // CLK_SYNC
    {
        l.num = 0;
        l.name = "Synt. LMX2592";
        l.value = 0;
        list.push_back(l);
        l.num = 1;
        l.name = "DAC AD5621";
        l.value = 1;
        list.push_back(l);
        l.num = 2;
        l.name = "DAC AD5621";
        l.value = 2;
        list.push_back(l);
        l.num = 3;
        l.name = "Divider LMK01801";
        l.value = 3;
        list.push_back(l);
        l.num = 4;
        l.name = "FPGA GT Channel";
        l.value = 4;
        list.push_back(l);
        l.num = 5;
        l.name = "FPGA GT Common";
        l.value = 4;
        list.push_back(l);
        dev = selectListValue("Select device", list, nameGroup);
        if (dev == 4) {
            list.clear();
            for (size_t i = 0; i < 10; i++) {
                l.num = i;
                l.name = "channel " + to_string(i);
                l.value = i;
                list.push_back(l);
            }
            numb = selectListValue("Select channel", list, nameSubGroup);
        }

    } break;
    case 4: // ADC9208
    {
        numb = 0;
        l.num = 0;
        l.name = "ADC AD9208";
        l.value = 0;
        list.push_back(l);
        l.num = 1;
        l.name = "FPGA: RECIEVER JESD";
        l.value = 8;
        list.push_back(l);
        dev = selectListValue("Select device", list, nameGroup);
    } break;
    case 7: // DAC
    {
        numb = 0;
        l.num = 0;
        l.name = "DAC AD9176";
        l.value = 0;
        list.push_back(l);
        l.num = 1;
        l.name = "FPGA: TRANSMITTER JESD";
        l.value = 8;
        list.push_back(l);
        dev = selectListValue("Select device", list, nameGroup);
    } break;
    }

    printf("Input for Write: REG=VALUE, for Read: REG, for exit - q and press Enter ..\n");

    if (!nameSubGroup.empty())
        nameSubGroup = "." + nameSubGroup;
    while (1) {
        printf("SPD:%s.%s%s> ", nameTetr.c_str(), nameGroup.c_str(), nameSubGroup.c_str());
        params.write = false;
        std::string line;
        getline(std::cin, line);
        if (line == "q" || line == "Q")
            break;
        std::string reg, val;
        int n = line.find('=');
        reg = line.substr(0, n);
        if (n > 0) {
            params.write = true;
            val = line.substr(n + 1);
        }
        params.reg = strtol(reg.c_str(), NULL, 0);
        params.value = strtol(val.c_str(), NULL, 0);
        S32 status = 0;
        if (params.write)
            status = SpdWrite(params.hService, params.tetrad, dev, numb, params.reg, params.value);
        else
            params.value = SpdRead(params.hService, params.tetrad, dev, numb, params.reg);
        printf("SPD access register = 0x%X  value= 0x%X\n", params.reg, params.value);
    }
}

U32 printConstRegsFromTetrads(BRD_Handle hService, BRD_Reg& reg)
{
    U32 status = BRD_ctrl(hService, 0, BRDctrl_REG_READIND, &reg);
    if (!BRD_errcmp(status, BRDerr_OK))
        BRDC_printf(_BRDC(" ERR: Reg = 0x%04X\n"), reg.reg);
    else {
        BRDC_printf(_BRDC(" > Reg = 0x%04X  value = 0x%04X\n"),
            reg.reg, reg.val);
    }
    return reg.val;
}

int selectTetrad(std::string& name)
{
    std::vector<ListsPrintf> list;
    ListsPrintf l;

    l.num = 0;
    l.name = "MAIN";
    l.value = 0;
    list.push_back(l);
    l.num = 1;
    l.name = "CLK_SYNC";
    l.value = 1;
    list.push_back(l);
    l.num = 2;
    l.name = "adc_ad9208";
    l.value = 4;
    list.push_back(l);
    l.num = 3;
    l.name = "dac_ad9176";
    l.value = 7;
    list.push_back(l);

    return selectListValue("Select the tetrad (node control) : \n", list, name);
}
