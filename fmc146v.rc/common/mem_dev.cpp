#include "mem_dev.h"
#include "exceptinfo.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

mem_dev::mem_dev(uint32_t phys_address, uint32_t phys_size) {
  try {
    _bar = 0;
    _size = 0;
    _bar = reinterpret_cast<uint32_t *>(_map.map(phys_address, phys_address));
    if (_bar) {
      _size = phys_size;
    }
  } catch (const except_info_t &errInfo) {
    throw errInfo;
  } catch (...) {
  }
}

//-----------------------------------------------------------------------------

mem_dev::~mem_dev() {}

//-----------------------------------------------------------------------------
/*
bool mem_dev::mem_write(uint32_t offset, uint32_t val)
{
    volatile uint32_t *pmem = (volatile uint32_t*)_bar;
    if(!pmem)
        return false;
    pmem[offset >> 2] = val;
    return true;
}
*/
//-----------------------------------------------------------------------------

bool mem_dev::mem_read(uint32_t offset, uint32_t &val) {
  volatile uint32_t *pmem = (volatile uint32_t *)_bar;
  if (!pmem)
    return false;
  val = pmem[offset >> 2];
  return true;
}

//-----------------------------------------------------------------------------

uint32_t mem_dev::mem_read(uint32_t offset) {
  volatile uint32_t *pmem = (volatile uint32_t *)_bar;
  if (!pmem) {
    return 0;
  }
  return pmem[offset >> 2];
}

//-----------------------------------------------------------------------------
