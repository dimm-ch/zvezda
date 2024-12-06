#pragma once

#include <string>

#include "utypes.h"

namespace std {
template <typename Type = BRDCHAR>
string to_string(const Type brdstr[]) {
  auto str = basic_string<Type>(brdstr);
#if __cplusplus >= 201703L
  if constexpr (is_same_v<Type, string::value_type>) {
    return str;
  }
#endif
  return string(str.cbegin(), str.cend());
}
template <typename Type = BRDCHAR>
string to_string(const basic_string<Type>& str) {
#if __cplusplus >= 201703L
  if constexpr (is_same_v<Type, string::value_type>) {
    return str;
  }
#endif
  return string(str.cbegin(), str.cend());
}
}  // namespace std

namespace InSys {

using brd_string = std::basic_string<BRDCHAR>;

inline brd_string to_brd_string(const std::string& str) {
  return brd_string(str.cbegin(), str.cend());
}

template <typename ValueType, typename StringType = brd_string>
typename std::enable_if<std::is_same<typename StringType::value_type,
                                     std::string::value_type>::value,
                        brd_string>::type
to_brd_string(ValueType value) {
  return std::to_string(value);
}

template <typename ValueType, typename StringType = brd_string>
typename std::enable_if<std::is_same<typename StringType::value_type,
                                     std::wstring::value_type>::value,
                        brd_string>::type
to_brd_string(ValueType value) {
  return std::to_wstring(value);
}

}  // namespace InSys
