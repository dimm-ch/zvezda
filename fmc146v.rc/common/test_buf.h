#pragma once
#include "bardy.h"
#include "brd_dev.h"
#include "strconv.h"
#include "exceptinfo.h"
#include <iostream>
#include <thread>

typedef struct {
    char str[10240];
    U32 bit_error0[64]; //!< Число ошибок типа принят 0
    U32 bit_error1[64]; //!< Число ошибок типа принят 1
    U32 bit_cnt;
    U32 bit_cnt_error;
    U32 max_cnt_error;
    U32 max_bit_cnt;
    U32 block_mode;
    U32 buf_cnt_ok;
    U32 buf_cnt_error;
    U32 word_cnt_error;
    U32 buf_current;
    __int64 word_error[8 * 4 * 32];
    __int64 data_ex_psd;
    __int64 data_ex_psd_high;
    __int64 data_ex_psd256[8];
    __int64 data_ex_noise;
    __int64 data_ex_cnt;
    __int64 data_ex_psd32;
    U32 data_cnt_psd256[4];
    __int64 data_ex_cnt64;
    __int64 data_set_cnt64;
    U32 chn_error[8];
    U32 word_chn_cnt_error[8];
    U32 m_FlagFirst;
} buf_params;

void buf_set(U32* buf, U32 n, U32 size, U32 mode);
U32 check(U32 index, __int64 d0, __int64 di0);
U32 buf_check(U32* buf, U32 n, U32 size, U32 mode);
U32 buf_check_psd(U32* buf, U32 size);
void buf_check_start(U32 n_error, U32 _bit_cnt);
void set_psd_init(U32 data);
void restart(void);
U32 check_result(U32* cnt_ok, U32* cnt_error, U32** error, U32** bit0, U32** bit1);
char* report_word_error(void);
char* report_word_error(int chn);
U32 buf_check_psd256(U32* buf, U32 size, U32 chn);
U32 buf_check_cnt256(U32* buf, U32 size, U32 chn);
void chn_start();
U32 chn_step();
U32 buf_check_cnt256_single(U32* buf, U32 size);
U32 buf_check_psd512(U32* buf, U32 size, U32 chn);
