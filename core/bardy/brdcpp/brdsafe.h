#pragma once

#include <chrono>
#include <thread>
#if __cplusplus < 201703L
#include <experimental/filesystem>
#else
#include <filesystem>
#endif
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>
#include <vector>

#include "brd_adm2if.h"
#include "brd_except.h"
#include "brd_string.h"
#include "brdapi.h"
#include "ctrladc.h"
#include "ctrldac.h"
#include "ctrlreg.h"
#include "ctrlsdram.h"
#include "ctrlstrm.h"
#include "gipcy.h"
#include "register_type.h"
#include "utypes.h"

// TODO: Типизировать исключения

#if __cplusplus < 201402L
#error                                                                         \
    "This file requires compiler and library support for the ISO C++ 2014 standard or higher."
#endif

/*
 * use example:
 * InSys::Bardy::BRD_ctrl<BRDctrl_ADC_GETFORMAT>(handle, 0, format);
 */

namespace InSys {

#if __cplusplus < 201703L
namespace fs = std::experimental::filesystem;
#else
namespace fs = std::filesystem;
#endif

namespace Bardy {

template <U32 Command, typename ValueType> class BRDctrl_Checker {
  static constexpr bool checker() noexcept {
    switch (Command) {
    // BRD_IniFile
    case BRDctrl_ADC_READINIFILE:
    case BRDctrl_DAC_READINIFILE:
      return std::is_same<ValueType, BRD_IniFile>::value;
    // U32
    case BRDctrl_STREAM_SETDRQ:
    case BRDctrl_STREAM_SETSRC:
    case BRDctrl_STREAM_CBUF_WAITBLOCK:
    case BRDctrl_SDRAM_GETSRCSTREAM:
    case BRDctrl_SDRAM_SETFIFOMODE:
    case BRDctrl_SDRAM_ENABLE:
    case BRDctrl_SDRAM_FIFOSTATUS:
    case BRDctrl_ADC_ENABLE:
    case BRDctrl_ADC_SETCHANMASK:
    case BRDctrl_ADC_GETCHANMASK:
    case BRDctrl_ADC_SETMASTER:
    case BRDctrl_ADC_GETMASTER:
    case BRDctrl_ADC_SETFORMAT:
    case BRDctrl_ADC_GETFORMAT:
    case BRDctrl_ADC_GETBLKCNT:
    case BRDctrl_ADC_SETTARGET:
    case BRDctrl_ADC_GETSRCSTREAM:
    case BRDctrl_ADC_FIFOSTATUS:
    case BRDctrl_DAC_ENABLE:
    case BRDctrl_DAC_GETSRCSTREAM:
    case BRDctrl_DAC_SETSOURCE:
    case BRDctrl_DAC_SETCYCLMODE:
    case BRDctrl_DAC_FIFOSTATUS:
      return std::is_same<ValueType, U32>::value;
    // S32
    case BRDctrl_STREAM_CBUF_ADJUST:
    case BRDctrl_STREAM_CBUF_DONE:
      return std::is_same<ValueType, S32>::value;
    // REAL64
    case BRDctrl_ADC_SETRATE:
    case BRDctrl_ADC_GETRATE:
      return std::is_same<ValueType, REAL64>::value;
    // BRD_PretrigMode
    case BRDctrl_ADC_SETPRETRIGMODE:
    case BRDctrl_ADC_GETPRETRIGMODE:
      return std::is_same<ValueType, BRD_PretrigMode>::value;
    // BRD_AdcCfg
    case BRDctrl_ADC_GETCFG:
      return std::is_same<ValueType, BRD_AdcCfg>::value;
    // BRD_EnVal
    case BRDctrl_ADC_SETACQDATA:
      return std::is_same<ValueType, BRD_EnVal>::value;
    // PBRD_Spd
    case BRDctrl_REG_READSPD:
    case BRDctrl_REG_WRITESPD:
      return std::is_same<ValueType, BRD_Spd>::value;
    // PBRD_Reg
    case BRDctrl_REG_READDIR:
    case BRDctrl_REG_WRITEDIR:
    case BRDctrl_REG_READIND:
    case BRDctrl_REG_WRITEIND:
      return std::is_same<ValueType, BRD_Reg>::value;
    // BRD_SdramCfgEx
    case BRDctrl_SDRAM_GETCFGEX:
      return std::is_same<ValueType, BRD_SdramCfgEx>::value;
    // BRDctrl_StreamCBufAlloc
    case BRDctrl_STREAM_CBUF_ALLOC:
    case BRDctrl_STREAM_CBUF_FREE:
      return std::is_same<ValueType, BRDctrl_StreamCBufAlloc>::value;
    // BRDctrl_StreamCBufStart
    case BRDctrl_STREAM_CBUF_START:
      return std::is_same<ValueType, BRDctrl_StreamCBufStart>::value;
    // nullptr_t
    case BRDctrl_SDRAM_FIFORESET:
    case BRDctrl_STREAM_RESETFIFO:
    case BRDctrl_STREAM_CBUF_STOP:
    case BRDctrl_ADC_FIFORESET:
    case BRDctrl_ADC_PREPARESTART:
    case BRDctrl_DAC_FIFORESET:
    case BRDctrl_DAC_PREPARESTART:
      return std::is_same<ValueType, std::nullptr_t>::value;

    // TODO: ADD new CHECK
    default:
      return false;
    }
  }

public:
  static constexpr bool value{checker()};
};

template <U32 Command, typename Value>
S32 BRD_ctrl(BRD_Handle handle, U32 nodeId, Value &value) noexcept {
  static_assert(BRDctrl_Checker<Command, Value>::value,
                "Data value type doesn't match BRDctrl_XXX command");
  return ::BRD_ctrl(handle, nodeId, Command, &value);
}

enum class DisplayMode : int32_t {
  Unvisible = BRDdm_UNVISIBLE,
  Visible = BRDdm_VISIBLE,
  Info = BRDdm_INFO,
  Warning = BRDdm_WARN,
  Error = BRDdm_ERROR,
  Fatal = BRDdm_FATAL,
  Console = BRDdm_CONSOLE
};

enum class OpenMode : uint32_t {
  Exclusive = BRDopen_EXCLUSIVE,
  Shared = BRDopen_SHARED,
  Spy = BRDopen_SPY
};

enum class CaptureMode : uint32_t {
  Exclusive = BRDcapt_EXCLUSIVE,
  Shared = BRDcapt_SHARED,
  Spy = BRDcapt_SPY
};

enum class SpdMode : int32_t { _16bit = 0, _32bit };

using BardyError = decltype(BRDerr_OK);

class CDeviceInfo final : public BRD_Info {
public:
  CDeviceInfo() = default;
  CDeviceInfo(uint32_t _lid, const BRD_Info &info)
      : BRD_Info{info}, lid{_lid} {}
  uint32_t lid{};
};

class CDeviceInfoList final : public std::vector<CDeviceInfo> {};

class CServiceInfo final : public BRD_ServList {
public:
  CServiceInfo() = default;
};

class CServiceInfoList final : public std::vector<CServiceInfo> {
public:
  auto find(const brd_string &serviceName) const {
    for (auto it{cbegin()}; it != cend(); ++it) {
      if (BRDC_strncmp(serviceName.data(), it->name, BRDC_strlen(it->name)) ==
          0) {
        return it;
      }
    }
    return cend();
  }
  auto find(const BRDCHAR *serviceName, std::size_t serviceNum) const {
    brd_string name{serviceName};
    name += to_brd_string(serviceNum);
    return find(name);
  }
};

class CService;
namespace Detail {
class CServiceImpl final {
  friend CService;
  static const std::size_t k_MaxNum{10};
  CServiceInfo m_ServiceInfo{};
  CDeviceInfo m_DeviceInfo{};
  BRD_Handle m_Handle{};
  CaptureMode m_CaptureMode{};

public:
  static auto max() noexcept { return k_MaxNum; }
  CServiceImpl() = default;
  CServiceImpl(const CServiceImpl &) = delete;
  CServiceImpl(CServiceImpl &&) noexcept = default;
  CServiceImpl &operator=(const CServiceImpl &) = delete;
  CServiceImpl &operator=(CServiceImpl &&) noexcept = default;
  CServiceImpl(BRD_Handle deviceHandle, const CDeviceInfo &deviceInfo,
               const CServiceInfo &serviceInfo, CaptureMode captureMode,
               const std::chrono::milliseconds &timeout)
      : m_DeviceInfo{deviceInfo}, m_ServiceInfo{serviceInfo} {
    capture(deviceHandle, serviceInfo, captureMode, timeout);
  }
  ~CServiceImpl() noexcept {
    if (m_Handle != 0)
      try {
        release();
      } catch (...) {
      }
  }
  void capture(BRD_Handle deviceHandle, const CServiceInfo &serviceInfo,
               CaptureMode captureMode = CaptureMode::Shared,
               const std::chrono::milliseconds &timeout =
                   std::chrono::milliseconds{1000}) {
    auto realCaptureMode = std::underlying_type<CaptureMode>::type(captureMode);
    m_Handle = BRD_capture(deviceHandle, 0, &realCaptureMode, serviceInfo.name,
                           uint32_t(timeout.count()));
    switch (uint32_t(m_Handle)) {
    case BRDERR(BRDerr_BAD_HANDLE):
      throw brd_runtime_error(BRDERR(BRDerr_BAD_HANDLE), "Wrong handle!");
      break;
    case BRDERR(BRDerr_CLOSED_HANDLE):
      throw brd_runtime_error(BRDERR(BRDerr_CLOSED_HANDLE),
                              "Handle was closed or isn't opened!");
      break;
    case BRDerr_BAD_PARAMETER:
    case BRDERR(BRDerr_BAD_PARAMETER):
      throw brd_runtime_error(BRDERR(BRDerr_BAD_PARAMETER), "Bad parameter!");
      break;
    default:
      break;
    }
    m_CaptureMode = CaptureMode(realCaptureMode);
  }
  void release() {
    auto status = BRD_release(m_Handle, 0);
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_Handle = {};
    } else
      switch (uint32_t(status)) {
      case BRDERR(BRDerr_BAD_HANDLE):
        throw brd_runtime_error(BRDERR(BRDerr_BAD_HANDLE), "Wrong handle!");
        break;
      case BRDERR(BRDerr_CLOSED_HANDLE):
        throw brd_runtime_error(BRDERR(BRDerr_CLOSED_HANDLE),
                                "Handle was closed or isn't opened!");
        break;
      default:
        break;
      }
  }
  auto isCaptured() const noexcept { return (m_Handle != 0); }
  auto getServiceInfo() const noexcept { return m_ServiceInfo; }
  auto getDeviceInfo() const noexcept { return m_DeviceInfo; }
  auto getCaptureMode() const noexcept { return m_CaptureMode; }
};
} // namespace Detail

struct CService {
  std::shared_ptr<Detail::CServiceImpl> d_ptr{};

public:
  CService() : d_ptr{std::make_shared<Detail::CServiceImpl>()} {}
  CService(const CService &) = default;
  CService(CService &&) noexcept = default;
  CService &operator=(const CService &) = default;
  CService &operator=(CService &&) noexcept = default;
  CService(BRD_Handle deviceHandle, const CDeviceInfo &deviceInfo,
           const CServiceInfo &serviceInfo, CaptureMode captureMode,
           const std::chrono::milliseconds &timeout)
      : d_ptr{std::make_shared<Detail::CServiceImpl>(
            deviceHandle, deviceInfo, serviceInfo, captureMode, timeout)} {}
  virtual ~CService() noexcept = default;
  auto operator->() const {
    if (d_ptr == nullptr) {
      throw std::runtime_error{"Service isn't owner!"};
    }
    return d_ptr.get();
  }
  template <uint32_t Command, typename Value>
  auto control_unsafe(Value &value) const noexcept {
    return ::BRD_ctrl(d_ptr->m_Handle, 0, Command, &value);
  }
  template <uint32_t Command, typename Value>
  auto control_unsafe(const Value &value) const noexcept {
    return control_unsafe<Command>(const_cast<Value &>(value));
  }
  template <uint32_t Command> auto control_unsafe() const noexcept {
    return control_unsafe<Command>(nullptr);
  }
  template <uint32_t Command, typename Value>
  auto control(Value &value) const noexcept {
    static_assert(BRDctrl_Checker<Command, Value>::value,
                  "Data value type not match BRDctrl_XXX command");
    return control_unsafe<Command>(value);
  }
  template <uint32_t Command, typename Value>
  auto control(const Value &value) const noexcept {
    return control<Command>(const_cast<Value &>(value));
  }
  template <uint32_t Command> auto control() const noexcept {
    return control<Command>(nullptr);
  }
  virtual brd_string getBaseName() const noexcept { return {}; }
};

using TetrNumType = int32_t;
using TetrIdType = uint32_t;

class CRegService : public CService {
public:
  static constexpr const BRDCHAR *BaseName{_BRDC("REG")};
  using SpdDevType = int32_t;
  using SpdNumType = int32_t;
  using SpdModeType = SpdMode;
  using RegisterType = int32_t;
  using ValueType = uint32_t;
  CRegService() = default;
  brd_string getBaseName() const noexcept override {
    return brd_string{BaseName};
  };
  using CService::CService;
  bool findTetr(TetrIdType tetrID) {
    RegisterType reg = ADM2IFnr_ID;
    for (int tetrNum{}; tetrNum < MAX_TETRNUM; ++tetrNum) {
      m_TetrNum = tetrNum;
      auto value = peekIndirect(reg);
      if (tetrID == value) {
        return true;
      }
    }
    return false;
  }
  void setSpdMode(SpdModeType spdMode) noexcept { m_SpdMode = spdMode; }
  auto getSpdMode() const noexcept { return m_SpdMode; }
  void writeSpd(SpdDevType dev, SpdNumType num, RegisterType reg, ValueType val,
                bool sync = {}) const {
    writeSpd(dev, num, reg, val, m_SpdMode, sync);
  }
  void writeSpd(SpdDevType dev, SpdNumType num, RegisterType reg, ValueType val,
                SpdModeType mode, bool sync = {}) const {
    BRD_Spd data{};
    data.dev = dev;
    data.mode = std::underlying_type<SpdModeType>::type(mode);
    data.num = num;
    data.sync = (sync) ? 1 : 0;
    data.reg = reg;
    data.tetr = m_TetrNum;
    data.val = val;
    auto status = control<BRDctrl_REG_WRITESPD>(data);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Write SPD register error!"};
    }
  }
  ValueType readSpd(SpdDevType dev, SpdNumType num, RegisterType reg) const {
    return readSpd(dev, num, reg, m_SpdMode);
  }
  ValueType readSpd(SpdDevType dev, SpdNumType num, RegisterType reg,
                    SpdModeType mode) const {
    BRD_Spd data{};
    data.dev = dev;
    data.mode = std::underlying_type<SpdModeType>::type(mode);
    data.num = num;
    data.sync = 0;
    data.reg = reg;
    data.tetr = m_TetrNum;
    data.val = 0;
    auto status = control<BRDctrl_REG_READSPD>(data);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Read SPD register error!"};
    }
    return data.val;
  }
  void pokeIndirect(RegisterType reg, ValueType val) const {
    BRD_Reg data{};
    data.bytes = sizeof(ValueType);
    data.reg = reg;
    data.tetr = m_TetrNum;
    data.val = val;
    auto status = control<BRDctrl_REG_WRITEIND>(data);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Write indirect register error!"};
    }
  }
  ValueType peekIndirect(RegisterType reg) const {
    BRD_Reg data{};
    data.bytes = sizeof(ValueType);
    data.reg = reg;
    data.tetr = m_TetrNum;
    data.val = 0;
    auto status = control<BRDctrl_REG_READIND>(data);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Read indirect register error!"};
    }
    return data.val;
  }
  void pokeDirect(RegisterType reg, ValueType val) const {
    BRD_Reg data{};
    data.bytes = sizeof(ValueType);
    data.reg = reg;
    data.tetr = m_TetrNum;
    data.val = val;
    auto status = control<BRDctrl_REG_WRITEDIR>(data);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Write direct register error!"};
    }
  }
  ValueType peekDirect(RegisterType reg) const {
    BRD_Reg data{};
    data.bytes = sizeof(ValueType);
    data.reg = reg;
    data.tetr = m_TetrNum;
    data.val = 0;
    auto status = control<BRDctrl_REG_READDIR>(data);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Read direct register error!"};
    }
    return data.val;
  }
  template <typename RegisterType, typename BitMask, typename BitMaskDefault,
            typename BitsType, typename AddrType, AddrType AddrValue>
  void
  setReg(SpdDevType dev, SpdNumType num,
         const InSys::Detail::SpdRegisterTypeAbstract<RegisterType, BitMask,
                                                      BitMaskDefault, BitsType,
                                                      AddrType, AddrValue> &reg,
         bool sync = {}) const {
    writeSpd(dev, num, reg.getAddress(), reg.getValue(), sync);
  }
  template <typename RegisterType, typename BitMask, typename BitMaskDefault,
            typename BitsType, typename AddrType, AddrType AddrValue>
  void getReg(SpdDevType dev, SpdNumType num,
              InSys::Detail::SpdRegisterTypeAbstract<
                  RegisterType, BitMask, BitMaskDefault, BitsType, AddrType,
                  AddrValue> &reg) const {
    reg.setValue(readSpd(dev, num, reg.getAddress()));
  }
  template <typename RegisterType, typename BitMask, typename BitMaskDefault,
            typename BitsType, typename AddrType, AddrType AddrValue>
  void setReg(const InSys::Detail::DirectRegisterTypeAbstract<
              RegisterType, BitMask, BitMaskDefault, BitsType, AddrType,
              AddrValue> &reg) const {
    pokeDirect(reg.getAddress(), reg.getValue());
  }
  template <typename RegisterType, typename BitMask, typename BitMaskDefault,
            typename BitsType, typename AddrType, AddrType AddrValue>
  void setReg(const InSys::Detail::IndirectRegisterTypeAbstract<
              RegisterType, BitMask, BitMaskDefault, BitsType, AddrType,
              AddrValue> &reg) const {
    pokeIndirect(reg.getAddress(), reg.getValue());
  }
  template <typename RegisterType, typename BitMask, typename BitMaskDefault,
            typename BitsType, typename AddrType, AddrType AddrValue>
  void getReg(InSys::Detail::DirectRegisterTypeAbstract<
              RegisterType, BitMask, BitMaskDefault, BitsType, AddrType,
              AddrValue> &reg) const {
    reg.setValue(peekDirect(reg.getAddress()));
  }
  template <typename RegisterType, typename BitMask, typename BitMaskDefault,
            typename BitsType, typename AddrType, AddrType AddrValue>
  void getReg(InSys::Detail::IndirectRegisterTypeAbstract<
              RegisterType, BitMask, BitMaskDefault, BitsType, AddrType,
              AddrValue> &reg) const {
    reg.setValue(peekIndirect(reg.getAddress()));
  }

private:
  TetrNumType m_TetrNum{-1};
  SpdModeType m_SpdMode{};
};

enum class StreamDrqMode : uint32_t {
  Almost = BRDstrm_DRQ_ALMOST,
  Ready = BRDstrm_DRQ_READY,
  Half = BRDstrm_DRQ_HALF
};

enum class StreamDirectionMode : uint32_t {
  In = BRDstrm_DIR_IN,
  Out = BRDstrm_DIR_OUT,
  InOut = BRDstrm_DIR_INOUT
};

struct CStreamMode {
  StreamDirectionMode direction{};
  bool isSystemMemory{};
  uint32_t blocksNum{};
  uint32_t blockSize{};
};

class CStream {
  CService m_Service{};
  bool m_IsStarted{};

public:
  CStream(const CStream &) = default;
  CStream(CStream &&) noexcept = default;
  CStream &operator=(const CStream &) = default;
  CStream &operator=(CStream &&) noexcept = default;
  CStream(const CService &service) : m_Service{service} {}
  virtual ~CStream() noexcept {
    if (m_IsStarted) {
      BardyError status{};
      stop(status);
    }
  }
  void reset(BardyError &status) noexcept {
    status = m_Service.control<BRDctrl_STREAM_RESETFIFO>();
  }
  void reset() {
    BardyError status{};
    reset(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_RESETFIFO");
    }
  }
  void start(bool cycled, BardyError &status) noexcept {
    BRDctrl_StreamCBufStart param{.isCycle = (cycled) ? 1u : 0u};
    status = m_Service.control<BRDctrl_STREAM_CBUF_START>(param);
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_IsStarted = true;
    }
  }
  void start(bool cycled) {
    BardyError status{};
    start(cycled, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_START");
    }
  }
  void stop(BardyError &status) noexcept {
    status = m_Service.control<BRDctrl_STREAM_CBUF_STOP>();
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_IsStarted = false;
    }
  }
  void stop() {
    BardyError status{};
    stop(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_STOP");
    }
  }
  auto isStarted() const noexcept { return m_IsStarted; }
  void setAjusted(bool ajusted, BardyError &status) noexcept {
    int32_t _ajusted = (ajusted) ? 1 : 0;
    status = m_Service.control<BRDctrl_STREAM_CBUF_ADJUST>(_ajusted);
  }
  void setAjusted(bool ajusted) {
    BardyError status{};
    setAjusted(ajusted, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_ADJUST");
    }
  }
  void done(int32_t number, BardyError &status) noexcept {
    status = m_Service.control<BRDctrl_STREAM_CBUF_DONE>(number);
  }
  void done(int32_t number) {
    BardyError status{};
    done(number, status);
    if (!BRD_errcmp(status, BRDerr_OK) == true) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_DONE");
    }
  }
  void waitBlock(uint32_t timeout, BardyError &status) noexcept {
    status = m_Service.control<BRDctrl_STREAM_CBUF_WAITBLOCK>(timeout);
  }
  void waitBlock(uint32_t timeout) {
    BardyError status{};
    waitBlock(timeout, status);
    if (BRD_errcmp(status, BRDerr_WAIT_TIMEOUT) == true) {
      throw brd_runtime_error(status, "BRDerr_WAIT_TIMEOUT");
    } else if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_WAITBLOCK");
    }
  }
  void setDrqMode(StreamDrqMode mode, BardyError &status) noexcept {
    auto drqMode = std::underlying_type<CaptureMode>::type(mode);
    status = m_Service.control<BRDctrl_STREAM_SETDRQ>(drqMode);
  }
  void setDrqMode(StreamDrqMode mode) {
    BardyError status{};
    setDrqMode(mode, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_SETDRQ");
    }
  }
  void setSource(uint32_t tetrad, BardyError &status) noexcept {
    status = m_Service.control<BRDctrl_STREAM_SETSRC>(tetrad);
  }
  void setSource(uint32_t tetrad) {
    BardyError status{};
    setSource(tetrad, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_STREAM_SETSRC");
    }
  }
  auto makeBuffer(const CStreamMode &streamMode) {
    static std::mutex _mutex{};
    auto streamCBufAlloc =
        [this](const CStreamMode &streamMode) -> decltype(auto) {
      const std::lock_guard<std::mutex> lock(_mutex);
      auto cbuf = new BRDctrl_StreamCBufAlloc{
          .dir = uint32_t(streamMode.direction),
          .isCont = (streamMode.isSystemMemory) ? uint32_t(1) : uint32_t(0),
          .blkNum = streamMode.blocksNum,
          .blkSize = streamMode.blockSize,
          .ppBlk = nullptr};
      cbuf->ppBlk = new void *[streamMode.blocksNum] {};
      auto status = m_Service.control<BRDctrl_STREAM_CBUF_ALLOC>(*cbuf);
      if (!BRD_errcmp(status, BRDerr_OK)) {
        delete[] cbuf->ppBlk;
        cbuf->ppBlk = nullptr;
        throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_ALLOC");
      }
      return cbuf;
    };
    auto streamCBufFree = [this](auto *cbuf) {
      const std::lock_guard<std::mutex> lock(_mutex);
      if (cbuf->ppBlk) {
        if (m_IsStarted) {
          stop();
        }
        auto status = m_Service.control<BRDctrl_STREAM_CBUF_FREE>(*cbuf);
        if (!BRD_errcmp(status, BRDerr_OK)) {
          throw brd_runtime_error(status, "BRDctrl_STREAM_CBUF_FREE");
        }
        delete[] cbuf->ppBlk;
        cbuf->ppBlk = nullptr;
      }
    };
    std::unique_ptr<BRDctrl_StreamCBufAlloc, decltype(streamCBufFree)> buffer(
        streamCBufAlloc(streamMode), streamCBufFree);
    return buffer;
  }
};

class CSdram {
  CService m_Service{};
  bool m_IsEnabled{};

public:
  CSdram(const CSdram &) = default;
  CSdram(CSdram &&) noexcept = default;
  CSdram &operator=(const CSdram &) = default;
  CSdram &operator=(CSdram &&) noexcept = default;
  CSdram(const CService &service) : m_Service{service} {}
  virtual ~CSdram() noexcept {
    if (m_IsEnabled) {
      BardyError status{};
      setEnabled(false, status);
    }
  }
  void reset(BardyError &status) noexcept {
    status = m_Service.control<BRDctrl_SDRAM_FIFORESET>();
  }
  void reset() {
    BardyError status{};
    reset(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_FIFORESET");
    }
  }
  void setEnabled(bool enabled, BardyError &status) noexcept {
    uint32_t _enabled = (enabled) ? 1 : 0;
    status = m_Service.control<BRDctrl_SDRAM_ENABLE>(_enabled);
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_IsEnabled = enabled;
    }
  }
  void setEnabled(bool enabled) {
    BardyError status{};
    setEnabled(enabled, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_ENABLE");
    }
  }
  auto isEnabled() const noexcept { return m_IsEnabled; }
  auto isOverflow(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = m_Service.control<BRDctrl_SDRAM_FIFOSTATUS>(fifoStatus);
    return ((fifoStatus & 0x80) == 0x80 ||
            (fifoStatus & 0x8000) == 0x8000); // FIXME: проверить бит
  }
  auto isOverflow() {
    BardyError status{};
    auto overflowed = isOverflow(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_FIFOSTATUS");
    }
    return overflowed;
  }
  auto isUnderflow(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = m_Service.control<BRDctrl_SDRAM_FIFOSTATUS>(fifoStatus);
    return ((fifoStatus & 0x100) == 0x100);
  }
  auto isUnderflow() {
    BardyError status{};
    auto underflowed = isUnderflow(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_FIFOSTATUS");
    }
    return underflowed;
  }
  auto isEmpty(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = m_Service.control<BRDctrl_SDRAM_FIFOSTATUS>(fifoStatus);
    return (((~fifoStatus) & 0x04) == 0x04);
  }
  auto isEmpty() {
    BardyError status{};
    auto empty = isEmpty(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_FIFOSTATUS");
    }
    return empty;
  }
  auto isFull(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = m_Service.control<BRDctrl_SDRAM_FIFOSTATUS>(fifoStatus);
    return (((~fifoStatus) & 0x40) == 0x40);
  }
  auto isFull() {
    BardyError status{};
    auto full = isFull(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_FIFOSTATUS");
    }
    return full;
  }
  auto getSourceStream(BardyError &status) noexcept {
    uint32_t tetrad{};
    status = m_Service.control<BRDctrl_SDRAM_GETSRCSTREAM>(tetrad);
    return tetrad;
  }
  auto getSourceStream() {
    BardyError status{};
    auto tetrad = getSourceStream(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_GETSRCSTREAM");
    }
    return tetrad;
  }
  auto getConfigEx(BRD_SdramCfgEx &config, BardyError &status) noexcept {
    config.Size = sizeof(BRD_SdramCfgEx);
    status = m_Service.control<BRDctrl_SDRAM_GETCFGEX>(config);
  }
  auto getConfigEx(BardyError &status) noexcept {
    BRD_SdramCfgEx config{};
    getConfigEx(config, status);
    return config;
  }
  auto getConfigEx() {
    BardyError status{};
    BRD_SdramCfgEx config{};
    getConfigEx(config, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_GETCFGEX");
    }
    return config;
  }
  void setFifoMode(bool enabled, BardyError &status) noexcept {
    uint32_t _enabled = (enabled) ? 1 : 0;
    status = m_Service.control<BRDctrl_SDRAM_SETFIFOMODE>(_enabled);
  }
  void setFifoMode(bool enabled) {
    BardyError status{};
    setFifoMode(enabled, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_SDRAM_SETFIFOMODE");
    }
  }
};

class CAdcService : public CService {
  bool m_IsEnabled{};

public:
  static constexpr const BRDCHAR *BaseName{_BRDC("ADC")};
  using CService::CService;
  CAdcService() = default;
  virtual ~CAdcService() noexcept {
    if (m_IsEnabled) {
      BardyError status{};
      setEnabled(false, status);
    }
  }
  brd_string getBaseName() const noexcept override {
    return brd_string{BaseName};
  };
  auto sdram() { return CSdram{*this}; }
  auto stream() { return CStream{*this}; }
  void init(const brd_string &filename, const brd_string &section,
            BardyError &status) noexcept {
    BRD_IniFile iniFile{};
    BRDC_strcpy(iniFile.fileName, filename.c_str());
    BRDC_strcpy(iniFile.sectionName, section.c_str());
    status = control<BRDctrl_ADC_READINIFILE>(iniFile);
  }
  void init(const brd_string &filename, const brd_string &section) {
    BardyError status{};
    init(filename, section, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_READINIFILE");
    }
  }
  void prepare(BardyError &status) noexcept {
    status = control<BRDctrl_ADC_PREPARESTART>();
  }
  void prepare() {
    BardyError status{};
    prepare(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_PREPARESTART");
    }
  }
  void reset(BardyError &status) noexcept {
    status = control<BRDctrl_ADC_FIFORESET>();
  }
  void reset() {
    BardyError status{};
    reset(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_FIFORESET");
    }
  }
  void setEnabled(bool enabled, BardyError &status) noexcept {
    uint32_t _enabled = (enabled) ? 1 : 0;
    status = control<BRDctrl_ADC_ENABLE>(_enabled);
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_IsEnabled = enabled;
    }
  }
  void setEnabled(bool enabled) {
    BardyError status{};
    setEnabled(enabled, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_ENABLE");
    }
  }
  auto isEnabled() const noexcept { return m_IsEnabled; }
  auto isOverflow(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_ADC_FIFOSTATUS>(fifoStatus);
    return ((fifoStatus & 0x80) == 0x80);
  }
  auto isOverflow() {
    BardyError status{};
    auto overflowed = isOverflow(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_FIFOSTATUS");
    }
    return overflowed;
  }
  auto isUnderflow(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_ADC_FIFOSTATUS>(fifoStatus);
    return ((fifoStatus & 0x100) == 0x100);
  }
  auto isUnderflow() {
    BardyError status{};
    auto underflowed = isUnderflow(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_FIFOSTATUS");
    }
    return underflowed;
  }
  auto isEmpty(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_ADC_FIFOSTATUS>(fifoStatus);
    return (((~fifoStatus) & 0x04) == 0x04);
  }
  auto isEmpty() {
    BardyError status{};
    auto empty = isEmpty(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_FIFOSTATUS");
    }
    return empty;
  }
  auto isFull(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_ADC_FIFOSTATUS>(fifoStatus);
    return (((~fifoStatus) & 0x40) == 0x40);
  }
  auto isFull() {
    BardyError status{};
    auto full = isFull(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_FIFOSTATUS");
    }
    return full;
  }
  auto getSourceStream(BardyError &status) noexcept {
    uint32_t tetrad{};
    status = control<BRDctrl_ADC_GETSRCSTREAM>(tetrad);
    return tetrad;
  }
  auto getSourceStream() {
    BardyError status{};
    auto tetrad = getSourceStream(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_ADC_GETSRCSTREAM");
    }
    return tetrad;
  }
};

enum class DacSourceMode : uint32_t { Fifo, Sdram };

class CDacService : public CService {
  bool m_IsEnabled{};

public:
  static constexpr const BRDCHAR *BaseName{_BRDC("DAC")};
  using CService::CService;
  CDacService() = default;
  virtual ~CDacService() noexcept {
    if (m_IsEnabled) {
      BardyError status{};
      setEnabled(false, status);
    }
  }
  brd_string getBaseName() const noexcept override {
    return brd_string{BaseName};
  };
  auto sdram() { return CSdram{*this}; }
  auto stream() { return CStream{*this}; }
  void init(const brd_string &filename, const brd_string &section,
            BardyError &status) noexcept {
    BRD_IniFile iniFile{};
    BRDC_strcpy(iniFile.fileName, filename.c_str());
    BRDC_strcpy(iniFile.sectionName, section.c_str());
    status = control<BRDctrl_DAC_READINIFILE>(iniFile);
  }
  void init(const brd_string &filename, const brd_string &section) {
    BardyError status{};
    init(filename, section, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_READINIFILE");
    }
  }
  void prepare(BardyError &status) noexcept {
    status = control<BRDctrl_DAC_PREPARESTART>();
  }
  void prepare() {
    BardyError status{};
    prepare(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_PREPARESTART");
    }
  }
  void reset(BardyError &status) noexcept {
    status = control<BRDctrl_DAC_FIFORESET>();
  }
  void reset() {
    BardyError status{};
    reset(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_FIFORESET");
    }
  }
  void setEnabled(bool enabled, BardyError &status) noexcept {
    uint32_t _enabled = (enabled) ? 1 : 0;
    status = control<BRDctrl_DAC_ENABLE>(_enabled);
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_IsEnabled = enabled;
    }
  }
  void setEnabled(bool enabled) {
    BardyError status{};
    setEnabled(enabled, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_ENABLE");
    }
  }
  auto isEnabled() const noexcept { return m_IsEnabled; }
  auto isOverflow(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_DAC_FIFOSTATUS>(fifoStatus);
    return ((fifoStatus & 0x80) == 0x80);
  }
  auto isOverflow() {
    BardyError status{};
    auto overflowed = isOverflow(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_FIFOSTATUS");
    }
    return overflowed;
  }
  auto isUnderflow(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_DAC_FIFOSTATUS>(fifoStatus);
    return ((fifoStatus & 0x100) == 0x100);
  }
  auto isUnderflow() {
    BardyError status{};
    auto underflowed = isUnderflow(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_FIFOSTATUS");
    }
    return underflowed;
  }
  auto isEmpty(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_DAC_FIFOSTATUS>(fifoStatus);
    return (((~fifoStatus) & 0x04) == 0x04);
  }
  auto isEmpty() {
    BardyError status{};
    auto empty = isEmpty(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_FIFOSTATUS");
    }
    return empty;
  }
  auto isFull(BardyError &status) noexcept {
    uint32_t fifoStatus{};
    status = control<BRDctrl_DAC_FIFOSTATUS>(fifoStatus);
    return (((~fifoStatus) & 0x40) == 0x40);
  }
  auto isFull() {
    BardyError status{};
    auto full = isFull(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_FIFOSTATUS");
    }
    return full;
  }
  auto getSourceStream(BardyError &status) noexcept {
    uint32_t tetrad{};
    status = control<BRDctrl_DAC_GETSRCSTREAM>(tetrad);
    return tetrad;
  }
  auto getSourceStream() {
    BardyError status{};
    auto tetrad = getSourceStream(status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_GETSRCSTREAM");
    }
    return tetrad;
  }
  void setSourceMode(DacSourceMode mode, BardyError &status) noexcept {
    uint32_t _mode{};
    switch (mode) {
    case DacSourceMode::Fifo:
      _mode = 0;
      break;
    case DacSourceMode::Sdram:
      _mode = 2;
      break;
    default:
      status = BRDerr_BAD_PARAMETER;
      return;
    }
    status = control<BRDctrl_DAC_SETSOURCE>(_mode);
  }
  void setSourceMode(DacSourceMode mode) {
    BardyError status{};
    setSourceMode(mode, status);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error(status, "BRDctrl_DAC_SETSOURCE");
    }
  }
};

namespace Detail {
class CDeviceImpl final {
  CDeviceInfo m_DeviceInfo{};
  BRD_Handle m_Handle{};

public:
  CDeviceImpl() = default;
  CDeviceImpl(const CDeviceImpl &) = delete;
  CDeviceImpl(CDeviceImpl &&) noexcept = default;
  CDeviceImpl &operator=(const CDeviceImpl &) = delete;
  CDeviceImpl &operator=(CDeviceImpl &&) noexcept = default;
  CDeviceImpl(const CDeviceInfo &deviceInfo, OpenMode openMode)
      : m_DeviceInfo{deviceInfo} {
    open(openMode);
  }
  ~CDeviceImpl() noexcept {
    if (m_Handle != 0)
      try {
        close();
      } catch (...) {
      }
  }
  void open(OpenMode openMode = OpenMode::Shared) {
    using namespace std::string_literals;
    m_Handle =
        BRD_open(m_DeviceInfo.lid,
                 std::underlying_type<OpenMode>::type(openMode), nullptr);
    switch (uint32_t(m_Handle)) {
    case BRDERR(BRDerr_BAD_LID):
      throw brd_runtime_error(BRDERR(BRDerr_BAD_LID),
                              "Wrong LID: "s +
                                  std::to_string(m_DeviceInfo.lid));
      break;
    case BRDERR(BRDerr_SHELL_UNINITIALIZED):
      throw brd_runtime_error(BRDERR(BRDerr_SHELL_UNINITIALIZED),
                              "Bardy isn't initialized!");
      break;
    case BRDERR(BRDerr_ALREADY_OPENED):
      throw brd_runtime_error(
          BRDERR(BRDerr_ALREADY_OPENED),
          "Can't open device, because it is already opened!");
      break;
    case BRDerr_BAD_MODE:
      throw brd_runtime_error(BRDerr_BAD_MODE, "Wrong device open mode!");
      break;
    default:
      break;
    }
  }
  void close() {
    auto status = BRD_close(m_Handle);
    if (BRD_errcmp(status, BRDerr_OK)) {
      m_Handle = {};
    } else
      switch (uint32_t(status)) {
      case BRDERR(BRDerr_BAD_HANDLE):
        throw brd_runtime_error(BRDERR(BRDerr_BAD_HANDLE), "Wrong handle!");
        break;
      case BRDERR(BRDerr_CLOSED_HANDLE):
        throw brd_runtime_error(BRDERR(BRDerr_CLOSED_HANDLE),
                                "Handle was closed or isn't opened!");
      default:
        break;
      }
  }
  auto getDeviceInfo() const noexcept { return m_DeviceInfo; }
  auto getServiceInfoList() const {
    CServiceInfoList serviceInfoList{};
    serviceInfoList.resize(CServiceImpl::max());
    uint32_t realSize{};
    auto status = BRD_serviceList(m_Handle, 0, serviceInfoList.data(),
                                  serviceInfoList.size(), &realSize);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Get service list error!"};
    }
    serviceInfoList.resize(realSize);
    return serviceInfoList;
  }
  template <typename ServiceType>
  auto getService(const CServiceInfo &serviceInfo,
                  CaptureMode captureMode = CaptureMode::Shared,
                  const std::chrono::milliseconds &timeout =
                      std::chrono::milliseconds{1000}) const {
    if (BRDC_strncmp(ServiceType::BaseName, serviceInfo.name,
                     BRDC_strlen(ServiceType::BaseName)) != 0) {
      throw std::runtime_error{"Wrong service type!"};
    }
    return ServiceType(m_Handle, getDeviceInfo(), serviceInfo, captureMode,
                       timeout);
  }
};

} // namespace Detail

class CDevice {
  std::shared_ptr<Detail::CDeviceImpl> d_ptr{};

public:
  CDevice() : d_ptr{std::make_shared<Detail::CDeviceImpl>()} {}
  CDevice(const CDevice &) = default;
  CDevice(CDevice &&) noexcept = default;
  CDevice &operator=(const CDevice &) = default;
  CDevice &operator=(CDevice &&) noexcept = default;
  CDevice(const CDeviceInfo &deviceInfo, OpenMode openMode)
      : d_ptr{std::make_shared<Detail::CDeviceImpl>(deviceInfo, openMode)} {}
  virtual ~CDevice() noexcept = default;
  auto operator->() const {
    if (d_ptr == nullptr) {
      throw std::runtime_error{"Device isn't owner!"};
    }
    return d_ptr.get();
  }
};

namespace Detail {

template <typename Fake> class CBardy final {
  static int32_t m_DeviceNums;
  static int32_t m_DisplayMode;

public:
  static void displayMode(DisplayMode displayMode,
                          bool enabled = true) noexcept {
    auto mode{std::underlying_type<DisplayMode>::type(displayMode)};
    if (enabled) {
      m_DisplayMode |= mode;
    } else {
      m_DisplayMode &= ~mode;
    }
    BRD_displayMode(m_DisplayMode);
  }
  CBardy(const brd_string &config) { init(config); }
  ~CBardy() noexcept { cleanup(); }
  CBardy(const CBardy &) = delete;
  CBardy(CBardy &&) = delete;
  CBardy &operator=(const CBardy &) = delete;
  CBardy &operator=(CBardy &&) noexcept = delete;
  static void init(const brd_string &config) {
    auto status = BRD_init(config.c_str(), &m_DeviceNums);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Bardy initialization error"};
    }
  }
  static auto getDeviceInfoList() {
    std::vector<uint32_t> lidArray{};
    BRD_LidList brdLidList{};
    lidArray.resize(m_DeviceNums);
    brdLidList.item = uint32_t(lidArray.size());
    brdLidList.pLID = lidArray.data();
    auto status =
        BRD_lidList(brdLidList.pLID, brdLidList.item, &brdLidList.itemReal);
    if (!BRD_errcmp(status, BRDerr_OK)) {
      throw brd_runtime_error{status, "Get LID list error!"};
    }
    lidArray.resize(brdLidList.itemReal);
    brdLidList.item = uint32_t(lidArray.size());
    BRD_Info brdinfo{};
    brdinfo.size = sizeof(brdinfo);
    CDeviceInfoList deviceInfoList{};
    for (auto &lid : lidArray) {
      status = BRD_getInfo(lid, &brdinfo);
      if (!BRD_errcmp(status, BRDerr_OK)) {
        throw brd_runtime_error{status, "Get device info error!"};
      }
      deviceInfoList.emplace_back(lid, brdinfo);
    }
    return deviceInfoList;
  }
  static auto getDevice(const CDeviceInfo &deviceInfo,
                        OpenMode openMode = OpenMode::Shared) {
    return CDevice(deviceInfo, openMode);
  }
  static void cleanup() noexcept {
    m_DeviceNums = {};
    m_DisplayMode = {};
    BRD_cleanup();
  }
  static int32_t getDeviceNums() noexcept { return m_DeviceNums; }
};

template <typename Fake> int32_t CBardy<Fake>::m_DeviceNums = 0;
template <typename Fake> int32_t CBardy<Fake>::m_DisplayMode = 0;

} // namespace Detail

using CBardy = Detail::CBardy<void>;

} // namespace Bardy

class CKeyboard final {
public:
  enum KeyCode : int {
    KeyCode_Escape = 0x1B,
    KeyCode_Space = 0x20,
    KeyCode_S = 0x53,
  };
  CKeyboard() noexcept { IPC_initKeyboard(); }
  ~CKeyboard() noexcept { cleanup(); }
  static inline void cleanup() noexcept { IPC_cleanupKeyboard(); }
  static inline bool isKeyPressed() noexcept { return (IPC_kbhit() > 0); }
  static inline int getKeyCode() noexcept { return IPC_getch(); }
};

template <typename Type, uint8_t size = 8 * sizeof(Type)>
std::string to_binary_string(Type arg) {
  std::string str{};
  std::size_t base = size;
  auto to_binary_string = [&base, &str](Type arg, auto &&to_binary_string) {
    str += std::string(((1 << (size - 1)) & arg) ? "1" : "0");
    if (--base == 0)
      return;
    if (base % 4 == 0) {
      str += " ";
    }
    to_binary_string(Type(arg << 1), to_binary_string);
  };
  to_binary_string(arg, to_binary_string);
  return str;
}

struct SuffixISVI {
  std::string deviceName{};
  uint32_t channelsNumber{};
  std::vector<uint32_t> channelsNumberList{};
  uint32_t samplesNumber{};
  double samplingRate{};
  uint32_t bytesPerSample{};
  uint32_t samplesPerByte{};
  bool isComplex{};
  double voltageRange{};
  uint32_t bitRange{};
};

class CFlagISVI final {
  enum class Flags : int { NoLock = 0, Updated = -1, Busy = 1 };

public:
  brd_string m_FileName = _BRDC("data.flg");
  CFlagISVI(const brd_string &dataFileName)
      : m_FileName{fs::path(dataFileName).replace_extension(".flg")} {
    set(Flags::NoLock, false);
  }
  ~CFlagISVI() noexcept = default;
  void set(Flags flag, bool isNewParam) const noexcept {
    IPC_handle handle{};
    do {
      handle =
          IPC_openFile(m_FileName.c_str(),
                       IPC_CREATE_FILE | IPC_FILE_WRONLY | IPC_FILE_NOBUFFER);
    } while (!handle);
    int data[]{int(flag), int(isNewParam)};
    IPC_writeFile(handle, data, sizeof(data));
    IPC_closeFile(handle);
  }
  Flags read() const noexcept {
    IPC_handle handle{};
    do {
      handle =
          IPC_openFile(m_FileName.c_str(), IPC_OPEN_FILE | IPC_FILE_RDONLY);
    } while (!handle);
    int flag = -1;
    IPC_readFile(handle, &flag, sizeof(flag));
    IPC_closeFile(handle);
    return Flags(flag);
  }
  bool ready() const noexcept {
    auto flag = read();
    return !(flag == Flags::Busy);
  }
  void update(bool newParams = {}) const noexcept {
    set(Flags::Updated, int(newParams));
  }
};

class CWriterISVI final {
  brd_string m_FileName{_BRDC("data.bin")};
  IPC_handle m_Handle{};
  SuffixISVI m_Suffix{};

public:
  CWriterISVI(const brd_string &name) : m_FileName{name} { open(); }
  ~CWriterISVI() noexcept {
    if (m_Handle) {
      close();
    }
  }
  void open(bool append = {}) {
    if (append) {
      m_Handle =
          IPC_openFile(m_FileName.c_str(),
                       IPC_OPEN_FILE | IPC_FILE_WRONLY | IPC_FILE_NOBUFFER);
    } else {
      m_Handle =
          IPC_openFile(m_FileName.c_str(),
                       IPC_CREATE_FILE | IPC_FILE_WRONLY | IPC_FILE_NOBUFFER);
    }
    if (m_Handle == nullptr) {
      throw std::runtime_error("Can't open file " + to_string(m_FileName));
    }
  }
  void close() noexcept {
    IPC_closeFile(m_Handle);
    m_Handle = {};
  }
  int write(void *data, int size) const noexcept {
    return IPC_writeFile(m_Handle, data, size);
  }
  void setSuffix(const SuffixISVI &suffix) { m_Suffix = suffix; }
  void writeSuffix() { writeSuffix(m_Suffix); }
  // TODO: Добавить и отладить запись суффиксов
  void writeSuffix(const SuffixISVI &suffix) {
    if (!m_Handle) {
      open(true);
    }
    auto status = IPC_setPosFile(m_Handle, 0, IPC_FILE_END);
    if (status < 0) {
      throw std::runtime_error("Error while adding suffix to file " +
                               std::to_string(m_FileName));
    }
    std::string suffixString{};
    suffixString += "\r\nDEVICE_NAME_________ ";
    suffixString += m_Suffix.deviceName;
    suffixString += "\r\nNUMBER_OF_CHANNELS__ ";
    suffixString += std::to_string(m_Suffix.channelsNumber);
    suffixString += "\r\nNUMBERS_OF_CHANNELS_ ";
    for (auto channel : m_Suffix.channelsNumberList) {
      suffixString += std::to_string(channel);
      suffixString += ",";
    }
    suffixString += "\r\nNUMBER_OF_SAMPLES___ ";
    suffixString += std::to_string(m_Suffix.samplesNumber);
    suffixString += "\r\nSAMPLING_RATE_______ ";
    suffixString += std::to_string(m_Suffix.samplingRate);
    suffixString += "\r\nBYTES_PER_SAMPLES___ ";
    suffixString += std::to_string(m_Suffix.bytesPerSample);
    suffixString += "\r\nSAMPLES_PER_BYTES___ ";
    suffixString += std::to_string(m_Suffix.samplesPerByte);
    suffixString += "\r\nIS_COMPLEX_SIGNAL?__ ";
    suffixString += (m_Suffix.isComplex) ? "YES" : "NO";
    suffixString += "\r\nSHIFT_FREQUENCY_____ 0"; // FIXME: исправить
    suffixString += "\r\nGAINS_______________ 0"; // FIXME: исправить
    suffixString += "\r\nVOLTAGE_OFFSETS_____ 0"; // FIXME: исправить
    suffixString += "\r\nVOLTAGE_RANGE_______ ";
    suffixString += std::to_string(m_Suffix.voltageRange);
    suffixString += "\r\nBIT_RANGE___________ ";
    suffixString += std::to_string(m_Suffix.bitRange);
    auto writed = IPC_writeFile(m_Handle, (void *)suffixString.c_str(),
                                suffixString.size());
    if (writed != suffixString.size()) {
      throw std::runtime_error("Can't write suffixe to file " +
                               std::to_string(m_FileName));
    }
    close();
  }
};

class CReaderISVI final {
  brd_string m_FileName{_BRDC("data.flg")};
  IPC_handle m_Handle{};
  SuffixISVI m_Suffix{};

public:
  CReaderISVI(const brd_string &name) : m_FileName{name} { open(); }
  ~CReaderISVI() noexcept {
    if (m_Handle) {
      close();
    }
  }
  void open() {
    m_Handle =
        IPC_openFile(m_FileName.c_str(), IPC_OPEN_FILE | IPC_FILE_RDONLY);
    if (m_Handle == nullptr) {
      throw std::runtime_error("Can't open file " + to_string(m_FileName));
    }
  }
  void close() noexcept {
    IPC_closeFile(m_Handle);
    m_Handle = {};
  }
  int read(void *data, int size) const noexcept {
    return IPC_readFile(m_Handle, data, size);
  }
  bool isOpened() const noexcept { return !(m_Handle == nullptr); }
  auto size() {
    bool needOpenClose{};
    if (isOpened()) {
      needOpenClose = true;
    }
    if (needOpenClose) {
      open();
    }
    long long fileSize{};
    auto status = IPC_getFileSize(m_Handle, &fileSize);
    if (status != IPC_OK) {
      throw std::runtime_error("Can't get file size from " +
                               to_string(m_FileName));
    }
    if (needOpenClose) {
      close();
    }
    return fileSize;
  }
};

} // namespace InSys
