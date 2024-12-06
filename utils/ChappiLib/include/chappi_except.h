///
/// \file chappi_except.h
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

namespace chappi {

template <typename ErrorType>
class runtime_error : public std::runtime_error {
    ErrorType _error;

public:
    runtime_error(ErrorType error, const char* message)
        : std::runtime_error(message)
        , _error { error }
    {
    }
    runtime_error(ErrorType error, std::string message)
        : std::runtime_error(message)
        , _error { error }
    {
    }
    ErrorType get_error() const noexcept { return _error; }
    virtual ~runtime_error() noexcept = default;
};

} // namespace chappi
