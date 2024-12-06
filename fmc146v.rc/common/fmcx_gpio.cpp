#include "fmcx_gpio.h"
#include "strconv.h"
#include "exceptinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

fmcx_gpio::fmcx_gpio(mem_dev_t hw, uint32_t id) : _hw(hw), _id(id)
{
}

//-----------------------------------------------------------------------------

fmcx_gpio::~fmcx_gpio()
{
}

//-----------------------------------------------------------------------------
