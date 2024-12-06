#ifndef TRD_PROG_H
#define TRD_PROG_H

#include "brd_dev.h"
#include <string>
#include <vector>

class trd_prog
{
public:
    trd_prog(brd_dev_t dev, std::string fileName);
    virtual ~trd_prog();

private:
	size_t get_options(std::vector<std::string>& optionsList);
	unsigned get_value(std::string& value);
	bool get_args(std::string& str, std::vector<unsigned>& argList);

	int process_options(std::vector<std::string>& optionsList);

	int process_blk_read(std::vector<unsigned>& argList);
	int process_blk_write(std::vector<unsigned>& argList);

	int process_trd_read(std::vector<unsigned>& argList);
	int process_trd_write(std::vector<unsigned>& argList);
	int process_trd_set_bit(std::vector<unsigned>& argList);
	int process_trd_clr_bit(std::vector<unsigned>& argList);
	int process_trd_pause(std::vector<unsigned>& argList);
	int process_trd_wait_val(std::vector<unsigned>& argList);

	int process_trd_write_spd(std::vector<unsigned>& argList);
	int process_trd_read_spd(std::vector<unsigned>& argList);
	int process_trd_wait_spd_val(std::vector<unsigned>& argList);

	brd_dev_t m_dev;
	std::string m_configFile;
};

#endif // TRD_PROG_H
