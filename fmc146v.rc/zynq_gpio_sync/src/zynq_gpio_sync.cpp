#include "zynq_gpio_sync.h"

static volatile int exit_flag = 0;

void signal_handler(int signo) {
    signo = signo;
    exit_flag = 1;
    fprintf(stderr, "\n");
}

void show_help(const std::string& program) {
    fprintf(stderr, "Usage: %s <options>\n", program.c_str());
    fprintf(stderr, "Options description:\n");
    fprintf(stderr, "-b <lid>: LID of test device\n");
    fprintf(stderr, "-s/--set <0|1>: Set <0|1> GPIO4\n");
    fprintf(stderr, "-r/--reg <T:R=V>: T - tetrad, R - register, V - value (for write)\n");
    fprintf(stderr, "--blink: Blink mode. Switche <0|1> GPIO4 in a loop\n");
    fprintf(stderr, "-h/--help: Print this message\n");
}

bool parse_value(const std::string& s, U32& out) {
    try {
        size_t pos = 0;
        int base = 10;

        if (s.size() > 2 && (s[0] == '0') && (s[1] == 'x' || s[1] == 'X'))
            base = 16;

        out = std::stoul(s, &pos, base);
        return pos == s.size();       // весь токен должен быть числом
    } catch (...) {
        return false;
    }
}

bool reg_pars(std::string line) {
    if (line.empty()) {
        return false;
    }
    
    auto colon = line.find(':');
    if (colon == std::string::npos) {
        return false;
    }

    auto eq = line.find('=', colon + 1);

    // Тетрада
    if (!parse_value(line.substr(0, colon), reg_trd)) {
        return false;
    }
    fmc146_trd = reg_trd;
    // Регистр
    if (eq == std::string::npos) {
        if (!parse_value(line.substr(colon + 1), reg_addr)) {
            return false;
        }
        return true;
    }

    if (!parse_value(line.substr(colon + 1, eq - colon - 1), reg_addr)) {
        return false;
    }

    // Значение на запись
    if (!parse_value(line.substr(eq + 1), reg_val)) {
        return false;
    }
    write_flag = true;

    return true;
}

void setMode() {
    // ставим направление GPIO2, GPIO4 = OUT
    reg = {0};
    reg.byBits.GPIO1_T = 1;
    reg.byBits.GPIO2_T = 0;
    reg.byBits.GPIO3_T = 1;
    reg.byBits.GPIO4_T = 0;

    fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
    IPC_delay(100);

    reg.byBits.GPIO4_O = value_to_set;
    reg.byBits.GPIO2_O = value_to_set;

    fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
    IPC_delay(100);

    fprintf(stderr, "GPIO2 & GPIO4 set to %d\n", value_to_set);
    return;
}

void blinkMode() {
    // ставим направление GPIO2, GPIO4 = OUT
    reg = {0};
    reg.byBits.GPIO1_T = 1;
    reg.byBits.GPIO2_T = 0;
    reg.byBits.GPIO3_T = 1;
    reg.byBits.GPIO4_T = 0;

    fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
    IPC_delay(100);

    fprintf(stderr, "Blink mode started. Press Ctrl+C to stop.\n");

    while (!exit_flag)
    {
        // инвертируем значение
        reg.byBits.GPIO4_O ^= 1;
        reg.byBits.GPIO2_O ^= 1;

        fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
        IPC_delay(100);
    }

    fprintf(stderr, "\nBlink mode stopped by user\n");
    return;
}

void regMode() {
    if (!reg_dir_flag) {
        if (write_flag) {
            fpga->RegPokeInd(reg_trd, reg_addr, reg_val);
            fprintf(stderr, "[ WRITE ] Tetrad %d, Register 0x%x, Value 0x%x\n", reg_trd, reg_addr, reg_val);
        }
        else {
            reg_val = fpga->RegPeekInd(reg_trd, reg_addr);
            fprintf(stderr, "[ READ ] Tetrad %d, Register 0x%x, Value 0x%x\n", reg_trd, reg_addr, reg_val);
        }
    }
    else {
        if (write_flag) {
            fpga->RegPokeDir(reg_trd, reg_addr, reg_val);
            fprintf(stderr, "[ WRITE ] Tetrad %d, Register 0x%x, Value 0x%x\n", reg_trd, reg_addr, reg_val);
        }
        else {
            reg_val = fpga->RegPeekDir(reg_trd, reg_addr);
            fprintf(stderr, "[ READ ] Tetrad %d, Register 0x%x, Value 0x%x\n", reg_trd, reg_addr, reg_val);
        }
    }
}

void testMode() {
    fprintf(stderr, "\n=== TEST: GPIO1 -> GPIO3 ===\n");

    reg = {0};

    // начальные значения
    reg.byBits.GPIO1_T = 0; // GPIO1 = OUT
    reg.byBits.GPIO2_T = 0;
    reg.byBits.GPIO3_T = 1; // GPIO3 = IN
    reg.byBits.GPIO4_T = 0;

    fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
    IPC_delay(100);

    // =============================
    // ТЕСТ 1 — подать 1 → ожидать 1
    // =============================
    reg.byBits.GPIO1_O = 1;
    reg.byBits.GPIO4_O = 0; // по задаче не принципиально
    fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
    IPC_delay(100);

    REG_GPIO readback;
    readback.asWhole = fpga->RegPeekInd(fmc146_trd, GPIO_REG);

    fprintf(stderr, "Write GPIO1 = 1, Read GPIO3_I = %d\n", readback.byBits.GPIO3_I);

    if (readback.byBits.GPIO3_I != 1)
        fprintf(stderr, "[ ERROR ] expected GPIO3_I = 1, got %d\n", readback.byBits.GPIO3_I);
    else
        fprintf(stderr, "[  OK!  ] GPIO3_I sees logical 1\n");

    // =============================
    // ТЕСТ 2 — подать 0 → ожидать 0
    // =============================
    reg.byBits.GPIO1_O = 0;
    fpga->RegPokeInd(fmc146_trd, GPIO_REG, reg.asWhole);
    IPC_delay(100);

    readback.asWhole = fpga->RegPeekInd(fmc146_trd, GPIO_REG);

    fprintf(stderr, "Write GPIO1 = 0, Read GPIO3_I = %d\n", readback.byBits.GPIO3_I);

    if (readback.byBits.GPIO3_I != 0)
        fprintf(stderr, "[ ERROR ] expected GPIO3_I = 0, got %d\n", readback.byBits.GPIO3_I);
    else
        fprintf(stderr, "[  OK!  ] GPIO3_I sees logical 0\n");

    fprintf(stderr, "=== TEST FINISHED ===\n");

    return;
}

bool bardy_setup_device() {
    if(!Bardy::initBardy(brd_count)) {
        fprintf(stderr, "[ ERROR ] Bardy init failed\n");
        return false;
    }

    if(!Bardy::boardsLID(lids)) {
        fprintf(stderr, "[ ERROR ] cannot get LID list\n");
        return false;
    }

    for(const auto& lid : lids) {
        BRD_Info info;
        if(Bardy::boardInfo(lid, info)) {
            uint16_t  devid = ((info.boardType >> 16) & 0xffff);
            if((devid == 0x5533) & ulid == -1) {
                ulid = lid;
            }
        }
    }

    if(ulid == -1) {
        fprintf(stderr, "[ ERROR ] no lid found\n");
        return false;
    }

    fpga = get_device(ulid);
    if (!fpga) {
        fprintf(stderr, "[ ERROR ] cannot open device for LID %d\n", ulid);
        return false;
    }

    if (fmc146_trd == -1) {
        if(!fpga->get_trd_number(TETR_ID, fmc146_trd)) {
            fprintf(stderr, "[ ERROR ] dev%d: TRD ID: 0x%X - not found\n", ulid, TETR_ID);
            return false;
        }
        else {
            fprintf(stderr, "dev%d: TRD ID: 0x%X - OK, NUMBER: 0x%04X\n", ulid,TETR_ID, fmc146_trd);
        }
    }

    return true;
}

int main(int argc, char* argv[]) {

    signal(SIGINT, signal_handler);

    if (argc < 2) {
        show_help(argv[0]);
        return 0;
    }

    auto it = arg_map.find(std::string(argv[1]));

    if (it == arg_map.end() || it->second.mode == Mode::Help){
        show_help(argv[0]);
        return 0;
    }

    switch (it->second.mode) {
        case Mode::Set: {
            if (argc < 3) {
                show_help(argv[0]);
                return 0;
            }

            std::string_view value{argv[2]};

            if (value != "1" && value != "0") {
                show_help(argv[0]);
                return 0;
            }
            break;
        }
        case Mode::RegDir:
            reg_dir_flag = true;
        case Mode::RegInd:
            if (argc < 3 || !reg_pars(argv[2])) {
                show_help(argv[0]);
                return 0;
            }
            break;
    }

    ulid = get_from_cmdline(argc, argv, "-b", -1);

    try {
        if(!bardy_setup_device()) {
            return 1;
        }
        switch (it->second.mode) {
            case Mode::Set:
                value_to_set = std::atoi(argv[2]);
                setMode();
                break;
            case Mode::Blink:
                blinkMode();
                break;
            case Mode::Test:
                testMode();
                break;
            case Mode::RegDir:
            case Mode::RegInd:
                regMode();
                break;
        }
    }
    catch( const except_info_t& errInfo ) {
        fprintf(stderr, "%s", errInfo.info.c_str());
    }
    catch( ... ) {
        fprintf(stderr, "%s", "Unknown exception in the program!");
    }

    return 0;
}