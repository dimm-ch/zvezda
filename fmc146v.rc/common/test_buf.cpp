#include "test_buf.h"

buf_params g_buf_params = { 0 };

void buf_set(U32* buf, U32 n, U32 size, U32 mode)
{
    U32 ii;    
    U32 cnt_err = 0;

    __int64* ptr = (__int64*)buf;
    U32 size64 = size / 2;
    __int64 data_ex;
    __int64 data_sig;
    __int64 data_ex1;
    __int64 data_ex2;

    data_sig = n;
    if (mode & 0x80)
    {
        g_buf_params.block_mode = (mode >> 8) & 0xF;
        data_sig = n;
    }

    data_sig <<= 32;
    data_sig |= 0xA5A50123;
    *ptr++ = data_sig;

    switch (g_buf_params.block_mode)
    {
    case 0:  data_ex = 1; break;
    case 1:  data_ex = ~1; break;
    case 2:  data_ex = 1; break;
    case 3:  data_ex = ~1; break;
    case 4:  data_ex = 1;  data_ex2 = 0; break;
    case 5:  data_ex = ~1; data_ex2 = 0xFFFFFFFFFFFFFFFFL;  break;
    case 6:
    case 7:  data_ex = g_buf_params.data_ex_cnt; break;
    case 8:
    case 9:  data_ex = g_buf_params.data_ex_noise; break;
    }

    switch (g_buf_params.block_mode)
    {
    case 0:
    case 1:
        for (ii = 1; ii < size64; ii++)
        {
            *ptr++ = data_ex;
            {
                U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f;
            }
        }
        break;
    case 2:
    case 3:
        *ptr++ = (~data_ex);
        {
            U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
            data_ex <<= 1;
            data_ex &= ~1;
            data_ex |= f;
        }
        for (ii = 2; ii < size64; ii += 2)
        {
            *ptr++ = data_ex;
            *ptr++ = ~data_ex;
            {
                U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f;
            }
        }
        break;
    case 4:
    case 5:
    {
        int flag;
        for (ii = 1; ii < size64; ii++)
        {
            flag = ((n & 0xFF) == (ii & 0xFF)) ? 1 : 0;
            data_ex1 = (flag) ? data_ex : data_ex2;
            *ptr++ = data_ex1;
            if (flag)
            {
                U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f;
            }
        }
    }
    break;
    case 6:
    case 7:
        *ptr++ = ~data_ex;
        data_ex++;
        for (ii = 2; ii < size64; ii += 2)
        {
            *ptr++ = data_ex;
            *ptr++ = ~data_ex;
            data_ex++;
        }
        g_buf_params.data_ex_cnt = data_ex;
        break;
    case 8:
    case 9:
    {
        for (ii = 1; ii < size64; ii++)
        {
            *ptr++ = data_ex;
            {
                U32 data_h = data_ex >> 32;
                U32 f63 = data_h >> 31;
                U32 f62 = data_h >> 30;
                U32 f60 = data_h >> 28;
                U32 f59 = data_h >> 27;
                U32 f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f0;
            }
        }
    }

    g_buf_params.data_ex_noise = data_ex;
    break;
    }

    g_buf_params.block_mode++;
    if (g_buf_params.block_mode == 10)
        g_buf_params.block_mode = 0;

    g_buf_params.buf_current++;
}

U32 check(U32 index, __int64 d0, __int64 di0)
{
    U32 flag_error = 0;
    {
        flag_error = 1;
        // Запись информации об ошибке
        if (g_buf_params.word_cnt_error < g_buf_params.max_cnt_error) {
            g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 0] = g_buf_params.buf_current;
            g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 1] = index;
            g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 2] = d0;
            g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 3] = di0;
        }
        g_buf_params.word_cnt_error++;
    }
    return flag_error;
}

U32 buf_check(U32* buf, U32 n, U32 size, U32 mode)
{
    U32 ii;    
    U32 cnt_err = 0;

    __int64* ptr = (__int64*)buf;
    U32 size64 = size / 2;
    __int64 data_ex;
    __int64 data_in;
    __int64 data_sig;
    __int64 data_ex1;
    __int64 data_ex2;

    data_sig = n;
    if (mode & 0x80)
    {
        g_buf_params.block_mode = (mode >> 8) & 0xF;
        data_sig = n;
    }

    data_sig <<= 32;
    data_sig |= 0xA5A50123;
    data_in = *ptr++;
    if (data_sig != data_in)
    {
        cnt_err += check(0, data_sig, data_in);
    }

    switch (g_buf_params.block_mode)
    {
    case 0:  data_ex = 1; break;
    case 1:  data_ex = ~1; break;
    case 2:  data_ex = 1; break;
    case 3:  data_ex = ~1; break;
    case 4:  data_ex = 1;  data_ex2 = 0; break;
    case 5:  data_ex = ~1; data_ex2 = 0xFFFFFFFFFFFFFFFFL;  break;
    case 6:
    case 7:  data_ex = g_buf_params.data_ex_cnt; break;
    case 8:
    case 9:  data_ex = g_buf_params.data_ex_noise; break;
    }

    switch (g_buf_params.block_mode)
    {
    case 0:
    case 1:
        for (ii = 1; ii < size64; ii++)
        {
            data_in = *ptr++;
            if (data_ex != data_in)
            {
                cnt_err += check(ii, data_ex, data_in);
            }
            {
                U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f;
            }
        }
        break;

    case 2:
    case 3:

        data_in = *ptr++;
        if ((~data_ex) != data_in)
        {
            cnt_err += check(1, ~data_ex, data_in);
        }
        {
            U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
            data_ex <<= 1;
            data_ex &= ~1;
            data_ex |= f;
        }

        for (ii = 2; ii < size64; ii += 2)
        {
            data_in = *ptr++;
            if (data_ex != data_in)
            {
                cnt_err += check(ii, data_ex, data_in);
            }

            data_in = *ptr++;
            if ((~data_ex) != data_in)
            {
                cnt_err += check(ii + 1, ~data_ex, data_in);
            }

            {
                U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f;
            }
        }
        break;

    case 4:
    case 5:
    {
        int flag;
        for (ii = 1; ii < size64; ii++)
        {
            flag = ((n & 0xFF) == (ii & 0xFF)) ? 1 : 0;
            data_in = *ptr++;
            data_ex1 = (flag) ? data_ex : data_ex2;
            if (data_ex1 != data_in)
            {
                cnt_err += check(ii, data_ex1, data_in);
            }
            if (flag)
            {
                U32 f = (data_ex & 0x8000000000000000) ? 1 : 0;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f;
            }
        }
    }
    break;

    case 6:
    case 7:

        data_in = *ptr++;
        if ((~data_ex) != data_in)
        {
            cnt_err += check(1, ~data_ex, data_in);
        }
        data_ex++;

        for (ii = 2; ii < size64; ii += 2)
        {
            data_in = *ptr++;
            if (data_ex != data_in)
            {
                cnt_err += check(ii, data_ex, data_in);
            }

            data_in = *ptr++;
            if ((~data_ex) != data_in)
            {
                cnt_err += check(ii + 1, ~data_ex, data_in);
            }

            data_ex++;

        }
        g_buf_params.data_ex_cnt = data_ex;
        break;

    case 8:
    case 9:
    {
        for (ii = 1; ii < size64; ii++)
        {
            data_in = *ptr++;

            if (data_ex != data_in)
            {
                cnt_err += check(ii, data_ex, data_in);
            }

            {
                U32 data_h = data_ex >> 32;
                U32 f63 = data_h >> 31;
                U32 f62 = data_h >> 30;
                U32 f60 = data_h >> 28;
                U32 f59 = data_h >> 27;
                U32 f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;
                data_ex <<= 1;
                data_ex &= ~1;
                data_ex |= f0;
            }
        }
    }

    g_buf_params.data_ex_noise = data_ex;
    break;
    }

    g_buf_params.block_mode++;
    if (g_buf_params.block_mode == 10)
        g_buf_params.block_mode = 0;

    g_buf_params.buf_current++;
    if (cnt_err == 0)
        g_buf_params.buf_cnt_ok++;
    else
        g_buf_params.buf_cnt_error++;

    return cnt_err;
}

U32 buf_check_psd(U32* buf, U32 size)
{
    U32  ii;
    U32 cnt_err = 0;

    __int64* ptr = (__int64*)buf;
    U32 size64 = size / 2;
    __int64 data_ex;
    __int64 data_in;
    data_ex = g_buf_params.data_ex_psd;

    for (ii = 0; ii < size64; ii++)
    {
        data_in = *ptr++;
        if (data_ex != data_in)
        {
            if (g_buf_params.word_cnt_error < g_buf_params.max_cnt_error)
            {
                g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 0] = g_buf_params.buf_current;
                g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 1] = ii;
                g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 2] = data_ex;
                g_buf_params.word_error[g_buf_params.word_cnt_error * 4 + 3] = data_in;
            }
            g_buf_params.word_cnt_error++;
            cnt_err++;
            data_ex = data_in;
        }

        {
            U32 data_h = data_ex >> 32;
            U32 f63 = data_h >> 31;
            U32 f62 = data_h >> 30;
            U32 f60 = data_h >> 28;
            U32 f59 = data_h >> 27;
            U32 f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;
            data_ex <<= 1;
            data_ex &= ~1;
            data_ex |= f0;
        }
    }

    g_buf_params.data_ex_psd = data_ex;

    g_buf_params.block_mode++;
    if (g_buf_params.block_mode == 10)
        g_buf_params.block_mode = 0;

    g_buf_params.buf_current++;
    if (cnt_err == 0)
        g_buf_params.buf_cnt_ok++;
    else
        g_buf_params.buf_cnt_error++;

    return cnt_err;
}

void buf_check_start(U32 n_error, U32 _bit_cnt)
{
    if (n_error < 32) {
        g_buf_params.max_cnt_error = n_error;
    }
    else {
        g_buf_params.max_cnt_error = 32;
    }
    g_buf_params.buf_cnt_ok = 0;
    g_buf_params.buf_cnt_error = 0;
    g_buf_params.word_cnt_error = 0;
    g_buf_params.buf_current = 0;
    g_buf_params.max_bit_cnt = _bit_cnt;
    g_buf_params.block_mode = 0;

    g_buf_params.data_ex_cnt = 0;
    g_buf_params.data_ex_noise = 1;
    g_buf_params.data_ex_psd = 2;
    g_buf_params.data_ex_psd_high = 0xAA55;

    g_buf_params.data_ex_psd256[0] = 2; // | 0x10000000;
    g_buf_params.data_ex_psd256[1] = 0xAA55; // | 0x12000000;
    g_buf_params.data_ex_psd256[2] = 0xBB66;
    g_buf_params.data_ex_psd256[3] = 0xCC77;

    g_buf_params.data_ex_psd256[4] = 0xDD88;
    g_buf_params.data_ex_psd256[5] = 0xEE99;
    g_buf_params.data_ex_psd256[6] = 0xFFAA;
    g_buf_params.data_ex_psd256[7] = 0x100BB;

    g_buf_params.data_cnt_psd256[0] = 0;
    g_buf_params.data_cnt_psd256[1] = 1;
    g_buf_params.data_cnt_psd256[2] = 2;
    g_buf_params.data_cnt_psd256[3] = 3;

    g_buf_params.data_ex_cnt64 = -2;
    g_buf_params.data_set_cnt64 = 0;

    g_buf_params.m_FlagFirst = 0;

    for (int ii = 0; ii < 64; ii++) {
        g_buf_params.bit_error0[ii] = 0;
        g_buf_params.bit_error1[ii] = 0;
    }

    for (int ii = 0; ii < 8 * 4 * 32; ii++) {
        g_buf_params.word_error[ii] = 0;
    }

    g_buf_params.word_chn_cnt_error[0] = 0;
    g_buf_params.word_chn_cnt_error[1] = 0;
    g_buf_params.word_chn_cnt_error[2] = 0;
    g_buf_params.word_chn_cnt_error[3] = 0;

    g_buf_params.word_chn_cnt_error[4] = 0;
    g_buf_params.word_chn_cnt_error[5] = 0;
    g_buf_params.word_chn_cnt_error[6] = 0;
    g_buf_params.word_chn_cnt_error[7] = 0;
}

void set_psd_init(U32 data)
{
    g_buf_params.data_ex_psd = data;
}

void restart(void)
{
    g_buf_params.data_ex_cnt = 0;
    g_buf_params.data_ex_noise = 1;
    g_buf_params.data_ex_psd = 2;

    g_buf_params.buf_current = 0;
    g_buf_params.block_mode = 0;

    g_buf_params.data_ex_cnt = 0;
    g_buf_params.data_ex_noise = 1;
    g_buf_params.data_ex_psd = 2;
    g_buf_params.data_ex_psd_high = 0xAA55;

    g_buf_params.data_ex_psd256[0] = 2;
    g_buf_params.data_ex_psd256[1] = 0xAA55;
    g_buf_params.data_ex_psd256[2] = 0xBB66;
    g_buf_params.data_ex_psd256[3] = 0xCC77;

    g_buf_params.data_ex_psd256[4] = 0xDD88;
    g_buf_params.data_ex_psd256[5] = 0xEE99;
    g_buf_params.data_ex_psd256[6] = 0xFFAA;
    g_buf_params.data_ex_psd256[7] = 0x100BB;

    g_buf_params.data_cnt_psd256[0] = 0;
    g_buf_params.data_cnt_psd256[1] = 1;
    g_buf_params.data_cnt_psd256[2] = 2;
    g_buf_params.data_cnt_psd256[3] = 3;

    g_buf_params.data_ex_cnt64 = -2;
    g_buf_params.data_set_cnt64 = 0;
}

U32 check_result(U32* cnt_ok, U32* cnt_error, U32** error, U32** bit0, U32** bit1) {

    if (cnt_ok) *cnt_ok = g_buf_params.buf_cnt_ok;
    if (cnt_error) *cnt_error = g_buf_params.buf_cnt_error;
    if (error) *error = (U32*)(g_buf_params.word_error);
    if (bit0)  *bit0 = g_buf_params.bit_error0;
    if (bit1)  *bit1 = g_buf_params.bit_error1;

    return g_buf_params.word_cnt_error;
}

char* report_word_error(void)
{
    return report_word_error(-1);
}

char* report_word_error(int chn) {

    char* ptr = g_buf_params.str;
    int len;
    char bit[64], * ptr_bit;
    U32 nb, na;
    __int64 dout, din;
    int size = 0;
    *ptr = 0;
    int cnt = g_buf_params.max_cnt_error;
    if (chn >= 0)
    {
        cnt = g_buf_params.word_chn_cnt_error[chn];
    }
    if (cnt > g_buf_params.max_cnt_error)
    {
        cnt = g_buf_params.max_cnt_error;
    }

    if (-1 == chn)
    {
        chn = 0;
    }

    for (int ii = 0; ii < cnt; ii++) {
        nb = g_buf_params.word_error[chn * 128 + ii * 4 + 0];
        na = g_buf_params.word_error[chn * 128 + ii * 4 + 1];
        dout = g_buf_params.word_error[chn * 128 + ii * 4 + 2];
        din = g_buf_params.word_error[chn * 128 + ii * 4 + 3];
        ptr_bit = bit;
        len = sprintf(ptr, "ERROR: %04d  block: %04d  index: %08X  expect: %016llX  receive: %016llX xor: %016llX\r\n",
            ii, nb, na, dout, din, dout ^ din);
        ptr += len;
        size += len;
        if (size > 5000)
            break;
    }
    return g_buf_params.str;
}

U32 buf_check_psd256(U32* buf, U32 size, U32 chn)
{
    U32 ii, cnt_err = 0;

    __int64* ptr = (__int64*)buf;
    U32 size256 = size / 8;
    __int64 data_ex;
    __int64 data_in;

    data_ex = g_buf_params.data_ex_psd256[chn];

    ptr += chn;

    for (ii = 0; ii < size256; ii++)
    {
        data_in = *ptr; ptr += 4;

        if (data_ex != data_in)
        {
            if (g_buf_params.word_chn_cnt_error[chn] < g_buf_params.max_cnt_error)
            {
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 0] = g_buf_params.buf_current;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 1] = ii;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 2] = data_ex;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 3] = data_in;
            }
            g_buf_params.word_chn_cnt_error[chn]++;
            g_buf_params.chn_error[chn]++;
            if (data_in)
            {
                data_ex = data_in;
            }
            else
            {
                data_ex = 1;
            }

        }

        {
            U32 data_h = data_ex >> 32;
            U32 f63 = data_h >> 31;
            U32 f62 = data_h >> 30;
            U32 f60 = data_h >> 28;
            U32 f59 = data_h >> 27;
            U32 f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;

            data_ex <<= 1;
            data_ex &= ~1;
            data_ex |= f0;
        }
    }
    g_buf_params.data_ex_psd256[chn] = data_ex;
    return g_buf_params.chn_error[chn];
}

U32 buf_check_cnt256(U32* buf, U32 size, U32 chn)
{
    U32 ii, cnt_err = 0;
    U32 size256 = size / 8;
    U32 data_in;
    U32 data_ex;
    U32* ptr = (U32*)buf;

    if (0 == g_buf_params.m_FlagFirst)
    {
        g_buf_params.data_cnt_psd256[0] = buf[0] & 0xFFFFFFFF;
        g_buf_params.data_cnt_psd256[1] = buf[2] & 0xFFFFFFFF;
        g_buf_params.data_cnt_psd256[2] = buf[4] & 0xFFFFFFFF;
        g_buf_params.data_cnt_psd256[3] = buf[6] & 0xFFFFFFFF;
        g_buf_params.m_FlagFirst = 1;
    }

    data_ex = g_buf_params.data_cnt_psd256[chn];

    ptr += chn * 2;

    for (ii = 0; ii < size256; ii++)
    {
        data_in = *ptr; ptr += 8;
        if (data_ex != data_in)
        {
            if (g_buf_params.word_chn_cnt_error[chn] < g_buf_params.max_cnt_error)
            {
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 0] = g_buf_params.buf_current;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 1] = ii;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 2] = data_ex;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 3] = data_in;
            }
            g_buf_params.word_chn_cnt_error[chn]++;
            g_buf_params.chn_error[chn]++;
            if (data_in)
            {
                data_ex = data_in & 0xFFFFFFFF;
            }
            else
            {
                data_ex = 1;
            }
        }
        data_ex += 4;
    }
    g_buf_params.data_cnt_psd256[chn] = data_ex;
    return g_buf_params.chn_error[chn];
}

void chn_start()
{
    g_buf_params.chn_error[0] = 0;
    g_buf_params.chn_error[1] = 0;
    g_buf_params.chn_error[2] = 0;
    g_buf_params.chn_error[3] = 0;

    g_buf_params.chn_error[4] = 0;
    g_buf_params.chn_error[5] = 0;
    g_buf_params.chn_error[6] = 0;
    g_buf_params.chn_error[7] = 0;
}

U32 chn_step()
{
    int cnt_err = g_buf_params.chn_error[0];
    cnt_err += g_buf_params.chn_error[1];
    cnt_err += g_buf_params.chn_error[2];
    cnt_err += g_buf_params.chn_error[3];

    cnt_err += g_buf_params.chn_error[4];
    cnt_err += g_buf_params.chn_error[5];
    cnt_err += g_buf_params.chn_error[6];
    cnt_err += g_buf_params.chn_error[7];

    g_buf_params.block_mode++;
    if (g_buf_params.block_mode == 10)
        g_buf_params.block_mode = 0;

    g_buf_params.buf_current++;
    if (cnt_err == 0)
        g_buf_params.buf_cnt_ok++;
    else
        g_buf_params.buf_cnt_error++;

    return cnt_err;
}

U32 buf_check_cnt256_single(U32* buf, U32 size)
{
    U32 ii, cnt_err = 0;

    U32 size256 = size / 2;
    __int64 data_in;
    __int64 data_ex;
    __int64* ptr = (__int64*)buf;
    U32 chn = 0;

    data_ex = g_buf_params.data_ex_cnt64;
    ptr += chn * 2;
    __int64* ptr2 = ptr + 1;
    __int64 data_in2;
    __int64 data_ex2 = data_ex + 1;

    __int64 compare1 = 0, compare2 = 0;
    __int64 result = 0;

    for (ii = 0; ii < size256 / 8; ii++)
    {
        data_in = *ptr++;
        data_in2 = *ptr++;
        result += compare1 + compare2;
        data_ex2 += 2;
        data_ex += 2;
        compare1 = data_in - data_ex;
        compare2 = data_in2 - data_ex2;

        data_in = *ptr++;
        data_in2 = *ptr++;
        result += compare1 + compare2;
        data_ex2 += 2;
        data_ex += 2;
        compare1 = data_in - data_ex;
        compare2 = data_in2 - data_ex2;

        data_in = *ptr++;
        data_in2 = *ptr++;
        result += compare1 + compare2;
        data_ex2 += 2;
        data_ex += 2;
        compare1 = data_in - data_ex;
        compare2 = data_in2 - data_ex2;

        data_in = *ptr++;
        data_in2 = *ptr++;
        result += compare1 + compare2;
        data_ex2 += 2;
        data_ex += 2;
        compare1 = data_in - data_ex;
        compare2 = data_in2 - data_ex2;
        continue;
        {
            if (g_buf_params.word_chn_cnt_error[chn] < g_buf_params.max_cnt_error)
            {
                if (data_ex != data_in)
                {
                    g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 0] = g_buf_params.buf_current;
                    g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 1] = 2 * ii;
                    g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 2] = data_ex - 2;
                    g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 3] = data_in;
                    g_buf_params.word_chn_cnt_error[chn]++;
                    g_buf_params.chn_error[chn]++;
                }

                if (g_buf_params.word_chn_cnt_error[chn] < g_buf_params.max_cnt_error)
                {
                    if (data_ex2 != data_in2)
                    {
                        g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 0] = g_buf_params.buf_current;
                        g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 1] = 2 * ii + 1;
                        g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 2] = data_ex2 - 2;
                        g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 3] = data_in2;
                        g_buf_params.word_chn_cnt_error[chn]++;
                        g_buf_params.chn_error[chn]++;
                    }
                }
            }
            else
            {
                g_buf_params.word_chn_cnt_error[chn]++;
                g_buf_params.chn_error[chn]++;
            }
        }
    }
    result += compare1 + compare2;
    g_buf_params.data_ex_cnt64 = data_ex;
    if (result)
        g_buf_params.chn_error[chn]++;

    return g_buf_params.chn_error[chn];


}

U32 buf_check_psd512(U32* buf, U32 size, U32 chn)
{

    //  n%100 - òèï áëîêà
    U32 d0, d1, ii, jj;
    U32 di0, di1;
    U32 cnt_err = 0;

    __int64* ptr = (__int64*)buf;
    U32 size512 = size / 16;
    __int64 data_ex;
    __int64 data_in;
    __int64 data_sig;
    __int64 data_ex1;
    __int64 data_ex2;

    data_ex = g_buf_params.data_ex_psd256[chn];

    ptr += chn;

    for (ii = 0; ii < size512; ii++)
    {
        data_in = *ptr; ptr += 8;

        if (data_ex != data_in)
        {
            if (g_buf_params.word_chn_cnt_error[chn] < g_buf_params.max_cnt_error)
            {
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 0] = g_buf_params.buf_current;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 1] = ii;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 2] = data_ex;
                g_buf_params.word_error[chn * 128 + g_buf_params.word_chn_cnt_error[chn] * 4 + 3] = data_in;
            }
            g_buf_params.word_chn_cnt_error[chn]++;
            g_buf_params.chn_error[chn]++;
            if (data_in)
            {
                data_ex = data_in;
            }
            else
            {
                data_ex = 1;
            }

        }

        {
            U32 data_h = data_ex >> 32;
            U32 f63 = data_h >> 31;
            U32 f62 = data_h >> 30;
            U32 f60 = data_h >> 28;
            U32 f59 = data_h >> 27;
            U32 f0 = (f63 ^ f62 ^ f60 ^ f59) & 1;
            data_ex <<= 1;
            data_ex &= ~1;
            data_ex |= f0;
        }
    }

    g_buf_params.data_ex_psd256[chn] = data_ex;

    return g_buf_params.chn_error[chn];

}
