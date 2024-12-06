///
/// \file elapsed_time.hpp
/// \author Alexander Chernenko (chernenko.a@insys.ru)
/// \brief
/// \version 0.1
/// \date 28.05.2019
///
/// \copyright InSys Copyright (c) 2019
///
///

#pragma once

#include <iostream>
#include <chrono>
#include <string_view>

class CElapsedTime {
  std::string_view m_Message{};
  std::chrono::time_point<std::chrono::high_resolution_clock> m_Start{};

 public:
  CElapsedTime(std::string_view const &message = std::string_view{}) noexcept
      : m_Message{message},
        m_Start{std::chrono::high_resolution_clock::now()} {}

  inline void restart() { m_Start = std::chrono::high_resolution_clock::now(); }

  inline auto elapsed_s() {
    auto end{std::chrono::high_resolution_clock::now()};
    auto elapsed{std::chrono::duration<double>(end - m_Start)};
    return elapsed;
  }

  static auto debug_s(std::string_view const &info,
                      std::chrono::duration<double> const &elapsed_s) {
    std::cout << info << ": " << elapsed_s.count() << " sec." << std::endl;
  }

  inline auto delta_s(std::chrono::duration<double> const &end,
                      std::chrono::duration<double> const &start) {
    auto elapsed{std::chrono::duration<double>(end - start)};
    return elapsed;
  }

  ~CElapsedTime() noexcept {
    if (!m_Message.empty()) {
      debug_s(m_Message, elapsed_s());
    }
  }
};
