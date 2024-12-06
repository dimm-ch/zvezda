///
/// \file brd_except.h
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 24.01.2020
///
/// \copyright InSys Copyright (c) 2020
///
///

#pragma once

#include <stdexcept>
#include <string>

namespace InSys {

class brd_runtime_error : public std::runtime_error {
  int _error;

 public:
  brd_runtime_error(int error, const char *message)
      : std::runtime_error(message), _error{error} {}
  brd_runtime_error(int error, std::string message)
      : std::runtime_error(message), _error{error} {}
  int get_error() const noexcept { return _error; }
  virtual ~brd_runtime_error() noexcept = default;
};

}  // namespace InSys
