#ifndef _V7MMCM_H
#define _V7MMCM_H

typedef enum {
    V7MMCMnr_CLKOUT0_ClkReg1 = 0x8,
    V7MMCMnr_CLKOUT0_ClkReg2,
    V7MMCMnr_CLKOUT1_ClkReg1,
    V7MMCMnr_CLKOUT1_ClkReg2,
    V7MMCMnr_CLKFBOUT_ClkReg1 = 0x14,
    V7MMCMnr_CLKFBOUT_ClkReg2,
    V7MMCMnr_DivReg,
} TV7MMCM_NUM_REGS;

#define V7MMCM_FVCO_MAX 1200; //600 для 10 МГц; 1200 МГц - максимальная частота ГУН

#pragma pack(push, 1)

typedef union {
    U32 asWhole;
    struct {
        U32 LowTime : 6,
            HighTime : 6,
            Reseved : 1,
            PhaseMux : 3;
    };
} TV7MMCM_ClkReg1;

typedef union {
    U32 asWhole;
    struct {
        U32 DelayTime : 6,
            NoCount : 1,
            Edge : 1,
            MX : 2,
            FracWfR : 1,
            FracEn : 1,
            Frac : 3,
            Reserved : 2;
    };
} TV7MMCM_ClkReg2;

typedef union {
    U32 asWhole;
    struct {
        U32 LowTime : 6,
            HighTime : 6,
            NoCount : 1,
            Edge : 1,
            Reserved : 2;
    };
} TV7MMCM_DivReg;

#pragma pack(pop)
#endif //_V7MMCM_H
