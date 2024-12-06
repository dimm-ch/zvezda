
#include "collog.h"

#if __cplusplus < 201402L
#define COLLOG_NO_STRING_LITERALS
#endif

using namespace collog;
using namespace concol;
using namespace concol_literals;
#ifndef COLLOG_NO_STRING_LITERALS
using namespace std::string_literals;
#endif
#ifndef CONCOL_NO_STRING_VIEW
using namespace std::string_view_literals;
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[]) try {
  color_log log("LOG_TYPE", true);
  color::set_enabled(true);
  log.set_enabled(true);
  log << "test: "
      << "OK\n"_green_bright;
  log << "test: "
      << "ERROR\n"_red_bright;

  log << color::to_string("test: {+green}OK{}\n");
  log << color::to_string("test: {+red}ERROR{}\n");

#ifndef CONCOL_NO_STRING_VIEW
  log << "test: " << color_type::green_bright << "OK\n"sv;
  log << "test: " << color_type::red_bright << "ERROR\n"sv;
#endif
  log << color::to_string("{+white}Value{}: {+yellow}%d{}\n", 128);
  log << "{+white}Value{}: " << color_type::yellow_bright << 128
      << color_ctrl::reset << '\n';

  log << color::to_string("{+white}Value{}: {+yellow}%.2f{}\n", 3.14);
  log << "{+white}Value{}: " << color_type::yellow_bright << 3.14
      << color_ctrl::reset << '\n';

  color::set_enabled(false);
  log << "noncolor\n"_red_bright;

  log.set_enabled(false);
  log << "log is disable\n";

  log.set_enabled(true);
  log << "log is enable\n";

  return 0;
} catch (...) {
  std::cerr << "\nunexpected exception\n";
  return 0;
}
