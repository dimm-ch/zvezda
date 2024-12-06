/*

Copyright (c) 2020 InSyS (devlab@insys.ru)

*/

#include <iostream>

#include "INIReader.h"

#ifndef INI_PATH
#define INI_PATH "test.ini"
#endif

static const char k_IniPath[]{INI_PATH};

int main(int argc, char* argv[]) try {
  INIReader ini(k_IniPath);
  if (ini.ParseError() < 0) {
    std::cerr << "\nCan't parse ini file!\n";
  } else {
    const char* test_array_section{"test_array"};
    if (ini.HasSection(test_array_section)) {
      const char* array_integer_value{"array_integer"};
      if (ini.HasValue(test_array_section, array_integer_value)) {
        auto array_integer =
            ini.GetArray<int16_t>(test_array_section, array_integer_value);
        std::cout << array_integer_value << ": ";
        for (const auto& value : array_integer) {
          std::cout << value;
          if (value != array_integer.back()) {
            std::cout << ", ";
          }
        }
      }
      const char* array_real_value{"array_real"};
      if (ini.HasValue(test_array_section, array_real_value)) {
        auto array_real =
            ini.GetArray<double>(test_array_section, array_real_value);
        std::cout << '\n' << array_real_value << ": ";
        for (const auto& value : array_real) {
          std::cout << value;
          if (value != array_real.back()) {
            std::cout << ", ";
          }
        }
      }
      const char* array_empty_value{"array_empty"};
      bool is_default{};
      auto array_empty =
          ini.GetArray<int>(test_array_section, array_empty_value, is_default,
                            {9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
      std::cout << '\n' << array_empty_value << ' ';
      if(is_default){
        std::cout << "(by default)";
      }
      std::cout << ": ";
      for (const auto& value : array_empty) {
        std::cout << value;
        if (value != array_empty.back()) {
          std::cout << ", ";
        }
      }
    }
  }
  return 0;
} catch (...) {
  std::cerr << "\nUnexpected exception!\n";
  return 0;
}
