#include "brd.h"
#include "ctrlreg.h"
#include "gipcy.h"
#include <stdio.h>
#include <stdlib.h>

void display_usage()
{
    printf("Usage:\n");
    printf("  reg_rw <����� �������> <����� ��������> [�������� ��������]\n");
    printf("  ���� �������� <�������� ��������> ������, ����������� ������.\n");
    printf("  ���� �������� <�������� ��������> �� ������, ����������� ������.\n");
}

int main(int argc, char* argv[])
{
    if (argc < 3 || argc > 4) {
        display_usage();
        return -1;
    }

    int tetrad = atoi(argv[1]);
    int reg_addr = atoi(argv[2]);
    int reg_value = 0;

    bool write_mode = (argc == 4);
    if (write_mode) {
        reg_value = strtol(argv[3], NULL, 0);
    }

    BRD_init(_BRDC("brd.ini"), NULL);

    BRD_LidList lidList;
    lidList.item = 10;
    lidList.pLID = new U32[10];
    BRD_shell(BRDshl_LID_LIST, &lidList);

    BRD_Handle handle = BRD_open(lidList.pLID[0], BRDopen_SHARED, NULL);
    if (handle <= 0) {
        printf("Error opening device\n");
        return -1;
    }

    U32 mode = BRDcapt_SHARED;
    BRD_Handle hReg = BRD_capture(handle, 0, &mode, _BRDC("REG0"), 10000);
    if (hReg <= 0) {
        BRDC_printf(_BRDC("REG NOT capture (%X)\n"), hReg);
        return 0;
    }

    regdata.tetr = tetrad;
    regdata.reg = reg_addr;

    if (write_mode) {
        regdata.val = reg_value;
        S32 status = BRD_ctrl(handle, 0, BRDctrl_REG_WRITEIND, &regdata);
        if (status == BRDerr_OK) {
            printf("Written 0x%X to tetrad %d, register 0x%X\n", reg_value, tetrad, reg_addr);
        } else {
            printf("Error writing to tetrad %d, register 0x%X\n", tetrad, reg_addr);
        }
    } else {
        S32 status = BRD_ctrl(handle, 0, BRDctrl_REG_READIND, &regdata);
        if (status == BRDerr_OK) {
            printf("Tetrad %d, register 0x%X: 0x%X\n", tetrad, reg_addr, regdata.val);
        } else {
            printf("Error reading from tetrad %d, register 0x%X\n", tetrad, reg_addr);
        }
    }

    BRD_close(handle);
    BRD_cleanup();
    delete[] lidList.pLID;

    return 0;
}
