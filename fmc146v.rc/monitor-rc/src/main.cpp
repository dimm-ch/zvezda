
#include "exceptinfo.h"
#include "fmcx_gpio.h"
#include "fmcx_iic.h"
#include "mem_dev.h"
#include "ps_i2c.h"
#include "strconv.h"
#include "time_ipc.h"

#include <cmath>
#include <csignal>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "gipcy.h"
//-----------------------------------------------------------------------------

using namespace std;

//-----------------------------------------------------------------------------

static volatile int exit_flag = 0;
void signal_handler(int signo)
{
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

//------------------------------------------------------------------------------

const double MB = (1024. * 1024.);

//------------------------------------------------------------------------------

enum axi_gpio_bits {
    alert_1 = 0x1,
    overt_1 = 0x2,
    alert_2 = 0x4,
    overt_2 = 0x8,
    iic_bus_request = 0x10,
    fpga_enable_prog = 0x20,
    gpio_mask_bits = (alert_1 | overt_1 | alert_2 | overt_2 | iic_bus_request | fpga_enable_prog),
};

//------------------------------------------------------------------------------

using job_t = std::shared_ptr<std::thread>;

//------------------------------------------------------------------------------

const std::string fmc131v_channel_prefix = "/sys/bus/iio/devices/iio:device1/";
const std::string fmc141v_channel_prefix = "/sys/bus/iio/devices/iio:device1/";
const std::string fmc144v_channel_prefix = "/sys/bus/iio/devices/iio:device1/";
const std::string fmc146v_channel_prefix = "/sys/bus/iio/devices/iio:device1/";

std::vector<std::string> fmc131v_101_channels = {
    "in_voltage0_vccint_raw",
    "in_temp0_raw",
    "in_voltage8_raw",
    "in_voltage9_raw",
    "in_voltage10_raw",
    "in_voltage11_raw",
    "in_voltage12_raw",
    "in_voltage13_raw",
    "in_voltage14_raw",
    "in_voltage15_raw",
    "in_voltage16_raw",
    "in_voltage17_raw",
    "in_voltage18_raw",
    "in_voltage19_raw",
    "in_voltage20_raw",
    "in_voltage21_raw",
    "in_voltage22_raw",
    "in_voltage23_raw",
    "in_voltage24_raw",
};

//------------------------------------------------------------------------------

std::vector<std::string> fmc141v_channels = {
    "in_voltage8_vpvn_raw",
    "in_voltage0_vccint_raw",
    "in_temp0_raw",
    "in_voltage8_raw",
    "in_voltage9_raw",
    "in_voltage10_raw",
    "in_voltage11_raw",
    "in_voltage12_raw",
    "in_voltage13_raw",
    "in_voltage14_raw",
    "in_voltage15_raw",
    "in_voltage16_raw",
    "in_voltage17_raw",
    "in_voltage18_raw",
    "in_voltage19_raw",
    "in_voltage20_raw",
    "in_voltage21_raw",
    "in_voltage22_raw",
    "in_voltage23_raw",
    "in_voltage24_raw",
};

//-----------------------------------------------------------------------------

std::vector<std::string> fmc144v_channels = {
    "in_voltage8_vpvn_raw",
    "in_voltage0_vccint_raw",
    "in_temp0_raw",
    "in_voltage9_vaux0_raw",
    "in_voltage10_vaux1_raw",
    "in_voltage11_vaux2_raw",
    "in_voltage12_vaux3_raw",
    "in_voltage13_vaux4_raw",
    "in_voltage14_vaux5_raw",
    "in_voltage15_vaux6_raw",
    "in_voltage16_vaux7_raw",
    "in_voltage17_vaux8_raw",
    "in_voltage18_vaux9_raw",
    "in_voltage19_vaux10_raw",
    "in_voltage20_vaux11_raw",
    "in_voltage21_vaux12_raw",
    "in_voltage22_vaux13_raw",
    "in_voltage23_vaux14_raw",
    "in_voltage24_vaux15_raw",
};

//-----------------------------------------------------------------------------

std::vector<std::string> fmc146v_channels = {
    "in_voltage0_vccint_raw",
    "in_voltage1_vccaux_raw",
    "in_temp0_raw",
    "in_voltage9_raw",
    "in_voltage10_raw",
    "in_voltage11_raw",
    "in_voltage12_raw",
    "in_voltage13_raw",
    "in_voltage14_raw",
    "in_voltage15_raw",
    "in_voltage16_raw",
    "in_voltage17_raw",
    "in_voltage18_raw",
    "in_voltage19_raw",
    "in_voltage20_raw",
    "in_voltage21_raw",
    "in_voltage22_raw",
    "in_voltage23_raw",
    "in_voltage24_raw",
};

//-----------------------------------------------------------------------------

bool get_xadc_channel_code(const std::string& prefix, const std::string& channel_name, bool verbose, std::string& code)
{
    std::fstream ifs;
    std::string channel = prefix + channel_name;
    ifs.open(channel.c_str(), std::ios::in);
    if (!ifs.is_open()) {
        if (verbose)
            fprintf(stderr, "%s, %d: %s():\n Can't open iio file: %s\n", __FILE__, __LINE__, __FUNCTION__, channel.c_str());
        return false;
    }

    getline(ifs, code);

    if (!code.length())
        return false;

    ifs.close();

    return true;
}

//-----------------------------------------------------------------------------

bool get_fmc141v_xadc_data(const std::string& prefix, const std::vector<std::string>& channels, mem_dev_t pcie, uint32_t& state, bool verbose)
{
    bool res = false;
    float t_fpga1 = 0;
    float t_fpga2 = 0;
    const int start_aux = 9;
    int aux_chan_idx = start_aux;

    for (const auto& chan : channels) {

        // пропустим канал VP/VN
        if (strstr(chan.c_str(), "vpvn_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                val *= 1000;
                val /= 4096;
                val *= 2;
                float t = (((float)val / 4.) - 273.15);
                fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA2 t", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(20 * 4, val);
                t_fpga2 = t;
            }
            continue;
        }

        if (strstr(chan.c_str(), "temp0_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val * 503.975 / 4096.) - 273.15;
                fprintf(stderr, "%s:\t\t[%.1f] C,\t<%s> : %s\n", "ZYNQ t", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(1 * 4, val);
            }
            continue;
        }

        if (strstr(chan.c_str(), "vccint_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val / 4096.) * 3.;
                fprintf(stderr, "%s:\t[%.1f] V,\t<%s> : %s\n", "ZYNQ VCCINT", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(2 * 4, val);
            }
            continue;
        }

        std::string aux_chan_name = "in_voltage" + std::to_string(aux_chan_idx) + "_raw";

        if (strstr(chan.c_str(), aux_chan_name.c_str())) {

            std::string code;
            res = get_xadc_channel_code(prefix, chan, verbose, code);
            if (res) {

                int32_t val = fromString<int32_t>(code);

                val *= 1000;
                val /= 4096;
                val *= 2;

                switch (aux_chan_idx) {
                case start_aux + 0: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+0.9V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(4 * 4, val);
                } break;
                case start_aux + 1: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+1.5V_ZYNQ", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(5 * 4, val);
                } break;
                case start_aux + 2: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+1.0AVCC", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(6 * 4, val);
                } break;
                case start_aux + 3: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+5V_AUX", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(7 * 4, val);
                } break;
                case start_aux + 4: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.2V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(8 * 4, val);
                } break;
                case start_aux + 5: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+1.2V_ZYNQ", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(9 * 4, val);
                } break;
                case start_aux + 6: {
                    float _fval = (float)val / 0.05f;
                    fprintf(stderr, "%s:\t[%.1f] mA,\t<%s>  : %s\n", "+12V SUM", _fval, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(10 * 4, val);
                } break;
                case start_aux + 7: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+2.5V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(11 * 4, val);
                } break;
                case start_aux + 8: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V KU1", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(12 * 4, val);
                } break;
                case start_aux + 9: {
                    val *= 8;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+12V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(13 * 4, val);
                } break;
                case start_aux + 10: {
                    float t = (((float)val / 4.0f) - 273.15f);
                    fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA2 t", t, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(14 * 4, val);
                    t_fpga2 = val;
                } break;
                case start_aux + 11: {
                    float t = (((float)val / 4.0f) - 273.15f);
                    fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA2 t", t, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(15 * 4, val);
                    t_fpga2 = val;
                } break;
                case start_aux + 12: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+3.3V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(16 * 4, val);
                } break;
                case start_aux + 13: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.8V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(17 * 4, val);
                } break;
                case start_aux + 14: {
                    val *= 2;
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+3.3V_Zynq", (float)val, code.c_str(), chan.c_str());
                } break;
                case start_aux + 15: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V KU2", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(19 * 4, val);
                } break;
                }
            } else {
                break;
            }
            ++aux_chan_idx;
        }
    }

    float t_alert = 85.;
    float t_overt = 100.;

    state = 0;

    if (fabs(t_fpga1 - t_alert) <= 0.1) {
        state |= alert_1;
    }
    if (fabs(t_fpga1 - t_overt) <= 0.1) {
        state |= overt_1;
    }
    if (fabs(t_fpga2 - t_alert) <= 0.1) {
        state |= alert_2;
    }
    if (fabs(t_fpga2 - t_overt) <= 0.1) {
        state |= overt_2;
    }

    return res;
}

//------------------------------------------------------------------------------

bool get_fmc144v_xadc_data(const std::string& prefix, const std::vector<std::string>& channels, mem_dev_t pcie, uint32_t& state, bool verbose)
{
    bool res = false;
    float t_fpga1 = 0;
    float t_fpga2 = 0;
    int aux_chan_idx = 0;

    for (const auto& chan : channels) {

        // пропустим канал VP/VN
        if (strstr(chan.c_str(), "vpvn_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                val *= 1000;
                val /= 4096;
                val *= 2;
                float t = (((float)val / 4.) - 273.15);
                fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA2 t", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(20 * 4, val);
                t_fpga2 = t;
            }
            continue;
        }

        if (strstr(chan.c_str(), "temp0_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val * 503.975 / 4096.) - 273.15;
                fprintf(stderr, "%s:\t\t[%.1f] C,\t<%s> : %s\n", "ZYNQ t", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(1 * 4, val);
            }
            continue;
        }

        if (strstr(chan.c_str(), "vccint_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val / 4096.) * 3.;
                fprintf(stderr, "%s:\t[%.1f] V,\t<%s> : %s\n", "ZYNQ VCCINT", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(2 * 4, val);
            }
            continue;
        }

        std::string aux_chan_name = "vaux" + std::to_string(aux_chan_idx);
        if (strstr(chan.c_str(), aux_chan_name.c_str())) {

            std::string code;
            res = get_xadc_channel_code(prefix, chan, verbose, code);
            if (res) {

                int32_t val = fromString<int32_t>(code);

                val *= 1000;
                val /= 4096;
                val *= 2;

                switch (aux_chan_idx) {
                case 0: {
                    float _fval = (float)val / 0.05f;
                    fprintf(stderr, "%s:\t[%.1f] mA,\t<%s>  : %s\n", "+12V_shunt", _fval, code.c_str(), chan.c_str());
                    pcie->mem_write(4 * 4, val);
                } break;
                case 1: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+0.9V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(5 * 4, val);
                } break;
                case 2: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+1.0AVCC", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(6 * 4, val);
                } break;
                case 3: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s>   : %s\n", "FMC2_VADJ", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(7 * 4, val);
                } break;
                case 4: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+2.5V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(8 * 4, val);
                } break;
                case 5: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V_1", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(9 * 4, val);
                } break;
                case 6: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.2V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(10 * 4, val);
                } break;
                case 7: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.5V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(11 * 4, val);
                } break;
                case 8: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.8V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(12 * 4, val);
                } break;
                case 9: {
                    val *= 8;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+12V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(13 * 4, val);
                } break;
                case 10: {
                    val *= 2;
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+3.3V_Zynq", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(14 * 4, val);
                } break;
                case 11: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s>   : %s\n", "FMC1_VADJ", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(15 * 4, val);
                } break;
                case 12: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+3.3V", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(16 * 4, val);
                } break;
                case 13: {
                    val *= 2;
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+3.3V_CLK", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(17 * 4, val);
                } break;
                case 14: {
                    float t = (((float)val / 4.0f) - 273.15f);
                    fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA1 t", t, code.c_str(), chan.c_str());
                    pcie->mem_write(18 * 4, val);
                    t_fpga1 = val;
                } break;
                case 15: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V_2", (float)val, code.c_str(), chan.c_str());
                    pcie->mem_write(19 * 4, val);
                } break;
                }
            } else {
                break;
            }
            ++aux_chan_idx;
        }
    }

    float t_alert = 85.;
    float t_overt = 100.;

    state = 0;

    if (fabs(t_fpga1 - t_alert) <= 0.1) {
        state |= alert_1;
    }
    if (fabs(t_fpga1 - t_overt) <= 0.1) {
        state |= overt_1;
    }
    if (fabs(t_fpga2 - t_alert) <= 0.1) {
        state |= alert_2;
    }
    if (fabs(t_fpga2 - t_overt) <= 0.1) {
        state |= overt_2;
    }

    return res;
}

//------------------------------------------------------------------------------

bool get_fmc146v_xadc_data(const std::string& prefix, const std::vector<std::string>& channels, mem_dev_t pcie, uint32_t& state, bool verbose)
{
    bool res = false;
    float t_fpga1 = 0;
    float t_fpga2 = 0;
    const int start_aux = 9;
    int aux_chan_idx = start_aux;

    for (const auto& chan : channels) {

        if (strstr(chan.c_str(), "temp0_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val * 503.975 / 4096.) - 273.15;
                fprintf(stderr, "%s:\t\t[%.1f] C,\t<%s> : %s\n", "ZYNQ t", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(1 * 4, t);
            }
            continue;
        }

        if (strstr(chan.c_str(), "vccint_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val / 4096.) * 3.;
                fprintf(stderr, "%s:\t[%.1f] V,\t<%s> : %s\n", "ZYNQ VCCINT", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(2 * 4, t);
            }
            continue;
        }

        if (strstr(chan.c_str(), "in_voltage1_vccaux_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val / 4096.) * 3.;
                fprintf(stderr, "%s:\t[%.1f] V,\t<%s> : %s\n", "ZYNQ VCCAUX", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(3 * 4, t);
            }
            continue;
        }

        std::string aux_chan_name = "in_voltage" + std::to_string(aux_chan_idx);
        if (strstr(chan.c_str(), aux_chan_name.c_str())) {

            std::string code;
            res = get_xadc_channel_code(prefix, chan, verbose, code);
            if (res) {

                int32_t val = fromString<int32_t>(code);

                val *= 1000;
                val /= 4096;
                val *= 2;

                switch (aux_chan_idx) {
                case start_aux + 0: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.2V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(4 * 4, val);
                } break;
                case start_aux + 1: {
                    float _fval = (float)val / 0.05f;
                    fprintf(stderr, "%s:\t[%.1f] mA,\t<%s>  : %s\n", "+12V_shunt", _fval, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(5 * 4, _fval);
                } break;
                case start_aux + 2: {
                    fprintf(stderr, "%s:\t[unused port]\t<%s>  : %s\n", "VAUX[2]", "---", chan.c_str());
                    if (pcie)
                        pcie->mem_write(6 * 4, 0);
                } break;
                case start_aux + 3: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "FMC2_VADJ", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(7 * 4, val);
                } break;
                case start_aux + 4: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+2.5V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(8 * 4, val);
                } break;
                case start_aux + 5: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V_1", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(9 * 4, val);
                } break;
                case start_aux + 6: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+1.0AVCC", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(10 * 4, val);
                } break;
                case start_aux + 7: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+0.9V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(11 * 4, val);
                } break;
                case start_aux + 8: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.8V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(12 * 4, val);
                } break;
                case start_aux + 9: {
                    float t = (((float)val / 4.) - 273.15);
                    fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA2 t", t, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(13 * 4, t);
                    t_fpga2 = t;
                } break;
                case start_aux + 10: {
                    val *= 2;
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+3.3V_Zynq", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(14 * 4, val);
                } break;
                case start_aux + 11: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "FMC1_VADJ", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(15 * 4, val);
                } break;
                case start_aux + 12: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+3.3V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(16 * 4, val);
                } break;
                case start_aux + 13: {
                    float t = (((float)val / 4.0f) - 273.15f);
                    fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA1 t", t, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(17 * 4, t);
                    t_fpga1 = t;
                } break;
                case start_aux + 14: {
                    val *= 8;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+12V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(18 * 4, val);
                } break;
                case start_aux + 15: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V_2", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(19 * 4, val);
                } break;
                }
            } else {
                break;
            }
            ++aux_chan_idx;
        }
    }

    float t_alert = 85.;
    float t_overt = 100.;

    state = 0;

    if (fabs(t_fpga1 - t_alert) <= 0.1) {
        state |= alert_1;
    }
    if (fabs(t_fpga1 - t_overt) <= 0.1) {
        state |= overt_1;
    }
    if (fabs(t_fpga2 - t_alert) <= 0.1) {
        state |= alert_2;
    }
    if (fabs(t_fpga2 - t_overt) <= 0.1) {
        state |= overt_2;
    }

    return res;
}

//------------------------------------------------------------------------------

bool get_fmc131v_xadc_data(const std::string& prefix, const std::vector<std::string>& channels, mem_dev_t pcie, uint32_t& state, bool verbose)
{
    bool res = false;
    float t_fpga1 = 0;
    float t_fpga2 = 0;
    const int start_aux = 9;
    int aux_chan_idx = start_aux;

    for (const auto& chan : channels) {

        if (strstr(chan.c_str(), "voltage8_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                val *= 1000;
                val /= 4096;
                val *= 2;
                float t = (((float)val / 4.) - 273.15);
                fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s [0x%X - 0x%X]\n", "FPGA2 t", t, code.c_str(), chan.c_str(), *((uint32_t*)&t), val);
                if (pcie)
                    pcie->mem_write(20 * 4, t);
                t_fpga2 = t;
            }
            continue;
        }

        if (strstr(chan.c_str(), "temp0_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val * 503.975 / 4096.) - 273.15;
                fprintf(stderr, "%s:\t\t[%.1f] C,\t<%s> : %s\n", "ZYNQ t", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(1 * 4, t);
            }
            continue;
        }

        if (strstr(chan.c_str(), "vccint_raw")) {
            std::string code;
            if (get_xadc_channel_code(prefix, chan, verbose, code)) {
                int32_t val = fromString<int32_t>(code);
                float t = ((float)val / 4096.) * 3.;
                fprintf(stderr, "%s:\t[%.1f] V,\t<%s> : %s\n", "ZYNQ VCCINT", t, code.c_str(), chan.c_str());
                if (pcie)
                    pcie->mem_write(2 * 4, t);
            }
            continue;
        }

        std::string aux_chan_name = "in_voltage" + std::to_string(aux_chan_idx);
        if (strstr(chan.c_str(), aux_chan_name.c_str())) {

            std::string code;
            res = get_xadc_channel_code(prefix, chan, verbose, code);
            if (res) {

                int32_t val = fromString<int32_t>(code);

                val *= 1000;
                val /= 4096;
                val *= 2;

                switch (aux_chan_idx) {
                case start_aux + 0: {
                    float _fval = (float)val / 0.05f;
                    fprintf(stderr, "%s:\t[%.1f] mA,\t<%s>  : %s\n", "+12V_shunt", _fval, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(4 * 4, _fval);
                } break;
                case start_aux + 1: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+0.9V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(5 * 4, val);
                } break;
                case start_aux + 2: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+1.0AVCC", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(6 * 4, val);
                } break;
                case start_aux + 3: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s>   : %s\n", "FMC2_VADJ", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(7 * 4, val);
                } break;
                case start_aux + 4: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+2.5V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(8 * 4, val);
                } break;
                case start_aux + 5: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V_1", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(9 * 4, val);
                } break;
                case start_aux + 6: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.2V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(10 * 4, val);
                } break;
                case start_aux + 7: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.5V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(11 * 4, val);
                } break;
                case start_aux + 8: {
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+1.8V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(12 * 4, val);
                } break;
                case start_aux + 9: {
                    val *= 8;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+12V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(13 * 4, val);
                } break;
                case start_aux + 10: {
                    val *= 2;
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+3.3V_Zynq", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(14 * 4, val);
                } break;
                case start_aux + 11: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s>   : %s\n", "FMC1_VADJ", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(15 * 4, val);
                } break;
                case start_aux + 12: {
                    val *= 2;
                    fprintf(stderr, "%s:\t\t[%.1f] mV,\t<%s> : %s\n", "+3.3V", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(16 * 4, val);
                } break;
                case start_aux + 13: {
                    val *= 2;
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+3.3V_CLK", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(17 * 4, val);
                } break;
                case start_aux + 14: {
                    float t = (((float)val / 4.0f) - 273.15f);
                    fprintf(stderr, "%s:\t[%.1f] C,\t<%s> : %s\n", "FPGA1 t", t, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(18 * 4, t);
                    t_fpga1 = t;
                } break;
                case start_aux + 15: {
                    fprintf(stderr, "%s:\t[%.1f] mV,\t<%s> : %s\n", "+0.95V_2", (float)val, code.c_str(), chan.c_str());
                    if (pcie)
                        pcie->mem_write(19 * 4, val);
                } break;
                }
            } else {
                break;
            }
            ++aux_chan_idx;
        }
    }

    float t_alert = 85.;
    float t_overt = 100.;

    state = 0;

    if (fabs(t_fpga1 - t_alert) <= 0.1) {
        state |= alert_1;
    }
    if (fabs(t_fpga1 - t_overt) <= 0.1) {
        state |= overt_1;
    }
    if (fabs(t_fpga2 - t_alert) <= 0.1) {
        state |= alert_2;
    }
    if (fabs(t_fpga2 - t_overt) <= 0.1) {
        state |= overt_2;
    }

    return res;
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    std::string device_name;
    if (argc == 1) {
        device_name = "/dev/mem";
    } else {
        device_name = argv[1];
    }

    fprintf(stderr, "---------------------------\n");
    fprintf(stderr, "    MONITOR FMC146V RC     \n");
    fprintf(stderr, "---------------------------\n");

    setlocale(LC_ALL, "Russian");
    signal(SIGINT, signal_handler);

    fprintf(stderr, "press Esc to exit (linux)\n");
    fprintf(stderr, "press Ctrl+[ or to exit (windows + ssh)\n");

    bool is_verbose = is_option(argc, argv, "-v");
    bool is_debug_off = is_option(argc, argv, "-d");

    IPC_init();
    IPC_initKeyboard();

    try {

        fprintf(stderr, "%s(): Build date: %s - %s\n", __FUNCTION__, __DATE__, __TIME__);

        // работа c AXI GPIO контроллером xilinx в PL
        mem_dev_t dev_gpio = std::make_shared<mem_dev>(0x41030000, 0x1000);
        // работа c окном DDR3 Zynq - PCIe - HOST (доступ к памяти см. адресное пространство Zynq в hwmon.cpp)
        mem_dev_t dev_pcie = std::make_shared<mem_dev>(0x28200000, 0x1000);

        // Драйвер контроллера GPIO
        fmcx_gpio_t gpio_ctrl = std::make_shared<fmcx_gpio>(dev_gpio, 0);
        // GPIO BITS:
        // 0 - alert1
        // 1 - overt1
        // 2 - alert2
        // 3 - overt2
        // 4 - i2c bus req
        // 5 - fpga prog
        gpio_ctrl->enable_out(gpio_mask_bits, axi_gpio_id::GPIO_ID_0);
        gpio_ctrl->enable_in(0x1, axi_gpio_id::GPIO_ID_1);
        gpio_ctrl->set_bit((axi_gpio_bits::fpga_enable_prog | axi_gpio_bits::iic_bus_request), axi_gpio_id::GPIO_ID_0);
        ipc_delay(1000);
        uint32_t val = gpio_ctrl->read(axi_gpio_id::GPIO_ID_1);
        fprintf(stderr, "GPIO VAL: 0x%x\n", val);

        /*
                // работа c AXI IIC контроллером xilinx в PL (доступ к регистрам через mem_dev)
                mem_dev_t dev_i2c  = std::make_shared<mem_dev>(0x41040000, 0x1000);
                // Драйвер контроллера AXI IIC PL
                fmcx_iic_t iic = std::make_shared<fmcx_iic>(dev_i2c, 0);
                fprintf(stderr, "IIC SR: 0x%x\n", iic->read_fn(iic_registers::SR));
                iic->set_output_freq(100000);
        */
        // Драйвер контроллера IIC PS
#if 0        
        ps_iic_t iic = std::make_shared<i2c>("/dev/i2c-1", 0x70);

        // Программируем выход TCA9554 на доступ к генератору
        bool tca_ok = false;
        std::vector<uint8_t> rdata;
        std::vector<uint8_t> wdata;
        wdata.push_back(0x1 << 1);
        if(iic->write(0x70, wdata)) {
            ipc_delay(10);
            if(!iic->read(0x70, rdata, 1)) {
                fprintf(stderr, "Error read data from TCA9548!\n");
                return -1;
            }
            if(wdata != rdata) {
                if(rdata.empty()) {
                    fprintf(stderr, "Read data is empty!\n"); 
                } else {
                    fprintf(stderr, "W/R Data to TCA9548 error!\n");
                    fprintf(stderr, "0x%x -- 0x%x\n", (int)wdata.at(0), (int)rdata.at(0));
                }
            } else {
                fprintf(stderr, "Write data to TCA9548 - OK. 0x%x - 0x%x\n", (int)wdata[0], (int)rdata[0]);
                tca_ok = true;
            }
        } else {
            fprintf(stderr, "Error write data to TCA9548!\n");
        }
#endif
        // отпустим IIC BUS REQUEST
        gpio_ctrl->set_bit(axi_gpio_bits::fpga_enable_prog, axi_gpio_id::GPIO_ID_0);

        // Опрашиваем SYSMON
        int pass_count = 0;
        while (!exit_flag ) {

            
            if(IPC_kbhit()){
			int ch = 0 | IPC_getch();
			if(ch == 0xe0 && IPC_kbhit()){ // extended character (0xe0, xxx)
				ch = (ch<<8) | IPC_getch(); // get extended xharaxter info
			}
            switch(ch){
				case 0x1b:
                    fprintf(stderr,"ESC pressed\n");
                    exit_flag = 1;
                    return 0;
                    break;
			}
			fprintf(stderr,"\n");

            }

            uint32_t state = 0;
            // bool ok = get_fmc141v_xadc_data(fmc141v_channel_prefix, fmc141v_channels, nullptr, state, is_verbose);
            bool ok = get_fmc146v_xadc_data(fmc146v_channel_prefix, fmc146v_channels, nullptr, state, is_verbose);
            if (ok) {
                state |= axi_gpio_bits::fpga_enable_prog; // Разрешим программированием FPGA1, 2
                if (is_debug_off) {
                    if (pass_count > 5) {
                        state |= (alert_1 | alert_2);
                    }
                    if (pass_count > 10) {
                        state |= (overt_1 | overt_2);
                        exit_flag = 1;
                    }
                }
                fprintf(stderr, "\n%d: STATE = 0x%X\n", pass_count, state);
                gpio_ctrl->write(axi_gpio_id::GPIO_ID_0, state);
            } else {
                fprintf(stderr, "Error read sysmon data!\n");
            }
            ipc_delay(3000);
            ++pass_count;
        }

        if (exit_flag && is_debug_off) {
            fprintf(stderr, "Press ENTER to exit...\n");
            getchar();
        }

        return 0;

    } catch (const except_info_t& errInfo) {

        fprintf(stderr, "%s", errInfo.info.c_str());

    } catch (...) {

        fprintf(stderr, "%s", "Unknown exception in the program!");
    }

    return 0;
}

//------------------------------------------------------------------------------
