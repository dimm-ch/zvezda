#pragma once

#if __cplusplus < 201703L
#include <boost/optional.hpp>
#else
#include <optional>
#endif
#include <boost/program_options.hpp>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <string>

#include "brdsafe.h"
#include "concol.h"

#if __cplusplus < 201402L
#error \
    "This file requires compiler and library support for the ISO C++ 2014 standard or higher."
#endif

namespace InSys
{

  inline std::string makeVersionString(const std::string &name,
                                       unsigned int major, unsigned int minor,
                                       unsigned int patch)
  {
    std::string version{name};
    version += " version " + std::to_string(major) + "." + std::to_string(minor) +
               "." + std::to_string(patch) + "\n";
    version += "Copyright (C) 1989-2021 JSC InSys.\n";
    version += "Report bugs to <devlab@insys.ru>\n";
    return version;
  }

  inline void updateAlive(uint8_t nums)
  {
    using namespace std::string_literals;
    static int count{};
    if (++count <= nums)
    {
      concol::color{std::string(count, '>') + "\r"s}.print_green_bright();
    }
    else if (count <= nums * 2)
    {
      concol::color{std::string(count - nums, ' ') + "\r"s}.print_black();
    }
    if (count == nums * 2)
    {
      count = 0;
    }
    std::flush(std::cout);
  }

  constexpr long double operator"" _kHz(long double value)
  {
    return value * 1000;
  }
  constexpr long double operator"" _MHz(long double value)
  {
    return value * 1000000;
  }
  constexpr long double operator"" _GHz(long double value)
  {
    return value * 1000000000;
  }

  inline void color_printf_thread_safe(const std::string &message)
  {
    static std::mutex _mutex{};
    const std::lock_guard<std::mutex> lock(_mutex);
    concol::color::printf(message);
  }

  inline void awaitExit() noexcept
  {
    concol::color{"\nPress any key to exit...\n"}.print_cyan_bright();
    std::fflush(stdout);
    CKeyboard::getKeyCode();
    Bardy::CBardy::cleanup();
  }

  inline void terminateHandler() noexcept
  {
    concol::color::set_ostream(stderr);
    concol::color{"\nTerminate program!\n"}.print_red_bright();
    concol::color::set_ostream(stdout);
  }

  inline void abortHandler(int code) noexcept
  {
    concol::color::set_ostream(stderr);
    switch (code)
    {
    case SIGINT:
      concol::color{"\nExternal interrupt program.\n"}.print_red_bright();
      break;
    case SIGILL:
      concol::color{"\nInvalid program instruction.\n"}.print_red_bright();
      break;
    case SIGFPE:
      concol::color{"\nDivision by zero.\n"}.print_red_bright();
      break;
    case SIGTERM:
      concol::color{"\nProgram termination request.\n"}.print_red_bright();
      break;
    case SIGABRT:
      concol::color{"\nAbnormal termination condition.\n"}.print_red_bright();
      break;
    case SIGSEGV:
      concol::color{"\nInvalid memory access.\n"}.print_red_bright();
      break;
    }
    concol::color::set_ostream(stdout);
    Bardy::CBardy::cleanup();
    CKeyboard::cleanup();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::exit(code);
  }

#define ERROR_MESSAGE(fmt, ...) \
  concol::color::printf("{+red}" fmt "{}\n", ##__VA_ARGS__)

#define ERROR_ASSERT(success, message)                          \
  if (!(success))                                               \
  {                                                             \
    concol::color::printf("{red}Error:{+red} " message "{}\n"); \
    awaitExit();                                                \
    return EXIT_FAILURE;                                        \
  }

  template <typename FunctorType>
  auto exceptCatcher(FunctorType &&fn) noexcept
  try
  {
    return fn();
  }
  catch (const brd_runtime_error &e)
  {
    ERROR_MESSAGE("\n%s \nBardy error code: 0x%X\n", e.what(), e.get_error());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return EXIT_FAILURE;
  }
  catch (const std::exception &e)
  {
    ERROR_MESSAGE("\n%s\n", e.what());
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return EXIT_FAILURE;
  }
  catch (...)
  {
    ERROR_MESSAGE("\nUnexpected exception!\n");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return EXIT_FAILURE;
  }

  namespace po = boost::program_options;

#if __cplusplus < 201703L
  template <typename Type>
  using optional = boost::optional<Type>;
#else
  template <typename Type>
  using optional = std::optional<Type>;
#endif

  class CExamOptions
  {
  public:
    CExamOptions() { addOptions(); }
    ~CExamOptions() noexcept = default;
    CExamOptions(int argc, char *argv[])
    {
      addOptions();
      parseOptions(argc, argv);
    }
    void parseOptions(int argc, char *argv[])
    {
      _name = fs::path(argv[0]).filename().replace_extension("").string();
      po::store(po::parse_command_line(argc, argv, _description), _vm);
      po::notify(_vm);
    }
    optional<std::string> version() const
    {
      if (_vm.count("version"))
      {
        return makeVersionString(_name, EXEC_MAJOR_VERSION, EXEC_MINOR_VERSION,
                                 EXEC_PATCH_VERSION);
      }
      return {};
    }
    optional<po::options_description> help() const
    {
      if (_vm.count("help"))
      {
        return _description;
      }
      return {};
    }
    bool nocolor() const
    {
      if (_vm.count("nocolor"))
      {
        return true;
      }
      return false;
    }
    bool verbose() const
    {
      if (_vm.count("verbose"))
      {
        return true;
      }
      return false;
    }
    std::string name() const { return _name; }

  private:
    std::string _name{};

    void addOptions()
    {
      // clang-format off
      _description.add_options()
      ("nocolor", "Non colorized output.")
      ("verbose", "Verbose output information.")
      ("version,v", "Print version number and exit.")
      ("help,h", "Print usage information and exit.");
      // clang-format on
    }

  protected:
    po::options_description _description{"usage"};
    po::variables_map _vm{};
  };

  class CExamLogger
  {
    static inline bool m_enabled{};

  public:
    CExamLogger() = delete;
    static void setEnabled(bool enabled)
    {
      m_enabled = enabled;
    }
    template <typename... Args>
    static void printf(const char *fmt, const Args &...args)
    {
      static std::mutex _mutex{};
      const std::lock_guard<std::mutex> lock(_mutex);
      if (m_enabled)
      {
        concol::color::printf(fmt, args...);
      }
    }
  };

} // namespace InSys

#ifdef __linux__
#define CONSOLE_CLEAR() std::printf("\e[1;1H\e[2J");
#else
#define CONSOLE_CLEAR()
#endif
