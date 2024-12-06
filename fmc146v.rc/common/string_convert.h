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
string to_string(basic_string<Type> str) {
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

template <typename StringType = brd_string>
std::string to_string(StringType&& str) {
  return std::string(str.cbegin(), str.cend());
}

template <typename StringType = std::string>
typename std::enable_if<std::is_same<typename StringType::value_type,
                                     std::string::value_type>::value,
                        brd_string>::type
to_brd_string(StringType&& str) {
  return brd_string(str.cbegin(), str.cend());
}

template <typename ValueType, typename StringType>
typename std::enable_if<std::is_same<typename StringType::value_type,
                                     std::string::value_type>::value,
                        brd_string>::type
to_brd_string(ValueType value) {
  return std::to_string(value);
}

template <typename ValueType, typename StringType>
typename std::enable_if<std::is_same<typename StringType::value_type,
                                     std::wstring::value_type>::value,
                        brd_string>::type
to_brd_string(ValueType value) {
  return std::to_wstring(value);
}

template <typename ValueType>
brd_string to_brd_string(ValueType value) {
  return to_brd_string<ValueType, brd_string>(value);
}

}  // namespace InSys
