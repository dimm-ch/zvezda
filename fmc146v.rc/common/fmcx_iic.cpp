#include "fmcx_iic.h"
#include "time_ipc.h"
#include "exceptinfo.h"

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __linux__
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
using namespace std;
//-----------------------------------------------------------------------------

fmcx_iic::fmcx_iic(mem_dev_t hw, uint32_t id) : _hw(hw), _id(id) {
  reset();
  enable();
}

//-----------------------------------------------------------------------------

fmcx_iic::~fmcx_iic() {
  reset();
  disable();
}

//-----------------------------------------------------------------------------

void fmcx_iic::reset() {
  //! СБросим контроллер
  write_fn(0xA, SOFTR);
}

//-----------------------------------------------------------------------------

void fmcx_iic::enable() {
  write_fn(0xF, RX_FIFO_PIRQ); //! Установим глубину RX_FIFO
  write_fn(CR_EN | CR_TX_FIFO_Reset, CR); //! СБросим TX_FIFO
  write_fn(CR_EN, CR); //! Разрешим работу контроллера
}

//-----------------------------------------------------------------------------

void fmcx_iic::disable() { write_fn(0, CR); }

//-----------------------------------------------------------------------------

bool fmcx_iic::check_tx_fifo_empty(size_t timeout) {
  //! Ожидаем, готовности TX_FIFO
  ipc_time_t start = ipc_get_time();
  uint32_t status = read_fn(ISR);
  while (!(status & ISR_TX_FIFO_EMPTY)) {
    if (ipc_get_difftime(start, ipc_get_time()) >= timeout) {
      fprintf(stderr, "TX_FIFO full! Timeout exit.\n");
      return false;
    }
    status = read_fn(SR);
  }
  write_fn((status | ISR_TX_FIFO_EMPTY), ISR);
  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::check_tx_fifo_full(size_t timeout) {
  //! Ожидаем, готовности TX_FIFO
  ipc_time_t start = ipc_get_time();
  uint32_t status = read_fn(SR);
  while (status & STAT_TX_FIFO_Full) {
    if (ipc_get_difftime(start, ipc_get_time()) >= timeout) {
      fprintf(stderr, "TX_FIFO full! Timeout exit.\n");
      return false;
    }
    status = read_fn(SR);
  }
  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::check_rx_fifo(size_t timeout) {
  //! Ожидаем, готовности RX_FIFO
  ipc_time_t start = ipc_get_time();
  uint32_t status = read_fn(SR);
  while (status & STAT_RX_FIFO_Empty) {
    if (ipc_get_difftime(start, ipc_get_time()) >= timeout) {
      fprintf(stderr, "RX_FIFO not filled! Timeout exit.\n");
      return false;
    }
    status = read_fn(SR);
  }
  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::wait_bus_busy(size_t timeout) {
  //! Ожидаем, готовности RX_FIFO
  ipc_time_t start = ipc_get_time();
  uint32_t status = read_fn(SR);
  while (status & STAT_BB) {
    if (ipc_get_difftime(start, ipc_get_time()) >= timeout) {
      fprintf(stderr, "IIC bus busy! Timeout exit. SR: 0x%x ISR: 0x%x\n",
              read_fn(SR), read_fn(ISR));
      return false;
    }
    status = read_fn(SR);
  }
  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::write(const uint8_t i2c_addr, const std::vector<uint8_t> &data) {
  if (data.empty())
    return false;

  enable();

  //! Проверим состояние шины и FIFO
  if (!wait_bus_busy()) {
    return false;
  }

  //! Старт, адрес, запись
  uint32_t fifo_start = (IIC_START | (i2c_addr << 1) | IIC_WRITE);
  if (!write_fifo(fifo_start))
    return false;

  //! Запишем все данные в TX_FIFO, кроме последнего байта
  for (size_t ii = 0; ii < data.size() - 1; ++ii) {
    uint32_t fifo_data = data.at(ii);
    if (!write_fifo(fifo_data))
      return false;
  }

  //! Cтоп, последний байт данных
  uint32_t fifo_stop = (IIC_STOP | data.at(data.size() - 1));
  if (!write_fifo(fifo_stop))
    return false;

  if (!check_tx_fifo_empty(500))
    return false;

  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::read(const uint8_t i2c_addr, std::vector<uint8_t> &data,
                    size_t N) {
  enable();

  //! Проверим состояние шины и FIFO
  if (!wait_bus_busy()) {
    return false;
  }

  //! Старт, адрес, если data.empty() то чтение, иначе - запишем данные из data,
  //! а потом читаем
  uint32_t fifo_start =
      (IIC_START | (i2c_addr << 1) | (data.empty() ? IIC_READ : IIC_WRITE));
  if (!write_fifo(fifo_start))
    return false;

  //! Запишем все входные данные в TX_FIFO
  for (size_t ii = 0; ii < data.size(); ++ii) {
    uint32_t fifo_data = data.at(ii);
    if (!write_fifo(fifo_data))
      return false;
  }

  //! Команда на чтение данных
  if (!data.empty()) {
    uint32_t fifo_data = (IIC_START | (i2c_addr << 1) | IIC_READ);
    if (!write_fifo(fifo_data))
      return false;
  }

  //! Стоп и число байт, которые необходимо получить
  uint32_t fifo_stop = (IIC_STOP | N);
  if (!write_fifo(fifo_stop))
    return false;

  //! Считаем N-байт данных из RX_FIFO
  data.clear();
  for (size_t ii = 0; ii < N; ++ii) {

    //! Ожидаем, готовности RX_FIFO
    uint32_t fifo_data = 0;
    if (!read_fifo(fifo_data))
      return false;

    uint8_t byte = static_cast<uint8_t>(fifo_data);
    data.push_back(byte);
  }

  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::set_output_freq(int freq) {
  float axi_freq = 250000000.0f;
  float iic_freq = freq;
  float scl_internal_delay = 0.0f;
  uint32_t val = (uint32_t)(((axi_freq / (2.0 * iic_freq)) - 7.0f -
                             scl_internal_delay + 0.5f));
  write_fn(val & 0xffffffff, THIGH);
  write_fn(val & 0xffffffff, TLOW);
  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::check_sr() {
  uint32_t status = read_fn(SR);
  if (status & STAT_BB) {
    fprintf(stderr, "IIC bus busy! Try again later\n");
    return false;
  }

  if (!(status & STAT_TX_FIFO_Empty)) {
    fprintf(stderr, "TX_FIFO not empty!\n");
    return false;
  }

  if (!(status & STAT_RX_FIFO_Empty)) {
    fprintf(stderr, "RX_FIFO not empty!\n");
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::check_isr(bool is_write) {
  //! Проверим состояние шины и прерывания
  uint32_t isr = read_fn(ISR);

  if (isr & ISR_ARB_LOST) {
    fprintf(stderr, "Arbitration lost! ISR = 0x%x\n", isr);
    return false;
  }

  if (is_write) {
    if (isr & ISR_TX_ERROR) {
      fprintf(stderr, "Master transmit error! ISR = 0x%x\n", isr);
      return false;
    }
  } else {
    if (!(isr & ISR_TX_ERROR)) {
      uint32_t cr = read_fn(CR);
      if (!(cr & CR_TXAK)) {
        fprintf(stderr, "Master receive error! ISR = 0x%x\n", isr);
        return false;
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::i2c_read_byte_data(uint8_t i2c_address, uint8_t devreg,
                                  uint8_t &value) {
  std::vector<uint8_t> data;
  data.push_back(devreg);
  bool res = read(i2c_address, data, 1);
  if (res) {
    if (data.empty())
      return false;
    value = data.at(0);
  }
  return res;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::i2c_write_byte_data(uint8_t i2c_address, uint8_t devreg,
                                   uint8_t value) {
  std::vector<uint8_t> data;
  data.push_back(devreg);
  data.push_back(value);
  return write(i2c_address, data);
}

//-----------------------------------------------------------------------------

bool fmcx_iic::i2c_read_word_data(uint8_t i2c_address, uint16_t devreg,
                                  uint8_t &value) {
  std::vector<uint8_t> data;
  data.push_back((devreg & 0xff));
  data.push_back((devreg >> 8) & 0xff);
  bool res = read(i2c_address, data, 1);
  if (res) {
    if (data.empty())
      return false;
    value = data.at(0);
  }
  return res;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::i2c_write_word_data(uint8_t i2c_address, uint16_t devreg,
                                   uint8_t value) {
  std::vector<uint8_t> data;
  data.push_back((devreg & 0xff));
  data.push_back((devreg >> 8) & 0xff);
  data.push_back(value);
  return write(i2c_address, data);
}

//-----------------------------------------------------------------------------

bool fmcx_iic::read_fifo(uint32_t &value, size_t timeout) {
  //! Ожидаем, готовности RX_FIFO
  if (!check_rx_fifo(timeout))
    return false;

  //! Читаем очередной байт из RX_FIFO
  value = read_fn(RX_FIFO);

  return true;
}

//-----------------------------------------------------------------------------

bool fmcx_iic::write_fifo(uint32_t val, size_t timeout) {
  //! Ожидаем, готовности TX_FIFO
  if (!check_tx_fifo_full(timeout))
    return false;

  write_fn(val, TX_FIFO);

  return true;
}

//-----------------------------------------------------------------------------
