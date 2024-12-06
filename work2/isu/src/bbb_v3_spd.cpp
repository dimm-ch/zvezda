#include <stdio.h>
#include <stdlib.h>
#include "gipcy.h"
#include "brd.h"
#include "ctrlreg.h"
#include "reg_rw_spd.h"

void display_usage() {
    printf("Usage:\n");
    printf("  reg_rw <номер тетрады> <адрес регистра> [значение регистра]\n");
    printf("  Если параметр <значение регистра> указан, выполняется запись.\n");
    printf("  Если параметр <значение регистра> не указан, выполняется чтение.\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        display_usage();
        return -1;
    }

    int tetrad = atoi(argv[1]);
    int reg_addr = atoi(argv[2]);
    U32 reg_value = 0;

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
        delete[] lidList.pLID;
        return -1;
    }

    BRD_Handle hReg = RegRw_Capture(handle);
    if (hReg <= 0) {
        printf("Error capturing REG service\n");
        BRD_close(handle);
        delete[] lidList.pLID;
        return -1;
    }

    S32 status;
    if (write_mode) {
        status = RegRw_WriteInd(hReg, tetrad, reg_addr, reg_value);
        if (status == BRDerr_OK) {
            printf("Written 0x%X to tetrad %d, register 0x%X\n", reg_value, tetrad, reg_addr);
        } else {
            printf("Error writing to tetrad %d, register 0x%X\n", tetrad, reg_addr);
        }
    } else {
        status = RegRw_ReadInd(hReg, tetrad, reg_addr, &reg_value);
        if (status == BRDerr_OK) {
            printf("Tetrad %d, register 0x%X: 0x%X\n", tetrad, reg_addr, reg_value);
        } else {
            printf("Error reading from tetrad %d, register 0x%X\n", tetrad, reg_addr);
        }
    }

    RegRw_Release(hReg);
    BRD_close(handle);
    BRD_cleanup();
    delete[] lidList.pLID;

    return 0;
}
