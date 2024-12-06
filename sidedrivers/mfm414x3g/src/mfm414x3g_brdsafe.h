#pragma once

#include "brdsafe.h"

#include "mfm414x3g.h"

namespace InSys {

namespace mfm414x3g {

template <U32 Command, typename Value>
class BRDctrl_Checker : public Bardy::BRDctrl_Checker<Command, Value> {
  static constexpr bool checker() noexcept {
    if (Bardy::BRDctrl_Checker<Command, Value>::value) {
      return Bardy::BRDctrl_Checker<Command, Value>::value;
    }
    switch (Command) {
        // TODO: перенести в brdsafe
      case BRDctrl_ADC_GETCFG:
        return std::is_same<Value, BRD_AdcCfg>::value;
      case BRDctrl_ADC_GETBLKCNT:
      case BRDctrl_ADC_SETEVENTCNT:
        return std::is_same<Value, U32>::value;
      case BRDctrl_ADC_SETSTARTLEVEL:
        return std::is_same<Value, BRD_StartLevel_FM414x3G>::value;
      case BRDctrl_ADC_SETCOMPENABLE:
      case BRDctrl_ADC_GETCOMPENABLE:
      case BRDctrl_ADC_SETCOMPLEVEL:
      case BRDctrl_ADC_GETCOMPLEVEL:
      case BRDctrl_ADC_SETCOMPINVERSE:
      case BRDctrl_ADC_GETCOMPINVERSE:
        return std::is_same<Value, BRD_ValChan>::value;
      case BRDctrl_ADC_SETACQDATA:
        return std::is_same<Value, BRD_EnVal>::value;
      default:
        return false;
    }
  }

 public:
  static constexpr bool value{checker()};
};

template <U32 Command, typename Value>
S32 BRD_ctrl(BRD_Handle handle, U32 nodeId, Value& value) noexcept {
  static_assert(BRDctrl_Checker<Command, Value>::value,
                "Data value type not match BRDctrl_XXX command");
  return ::BRD_ctrl(handle, nodeId, Command, &value);
}

}  // namespace mfm414x3g

}  // namespace InSys
