// Read an INI file into easy-to-access name/value pairs.

// inih and INIReader are released under the New BSD license (see LICENSE.txt).
// Go to the project home page for more info:
//
// https://github.com/benhoyt/inih

#ifndef __INIREADER_H__
#define __INIREADER_H__

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

// Read an INI file into easy-to-access name/value pairs. (Note that I've gone
// for simplicity here rather than speed, but it should be pretty decent.)
class INIReader {
 public:
  // Construct INIReader and parse given filename. See ini.h for more info
  // about the parsing.
  explicit INIReader(const std::string& filename);

  // [chernenko] повторное перечитывание ini файла
  int Parse();

  // Return the result of ini_parse(), i.e., 0 on success, line number of
  // first error on parse error, or -1 on file open error.
  int ParseError() const;

  // Get a string value from INI file, returning default_value if not found.
  std::string Get(const std::string& section, const std::string& name,
                  const std::string& default_value) const;

  // Get a string value from INI file, returning default_value if not found,
  // empty, or contains only whitespace.
  std::string GetString(const std::string& section, const std::string& name,
                        const std::string& default_value) const;

  // Get an integer (long) value from INI file, returning default_value if
  // not found or not a valid integer (decimal "1234", "-1234", or hex "0x4d2").
  long GetInteger(const std::string& section, const std::string& name,
                  long default_value) const;

  // Get a real (floating point double) value from INI file, returning
  // default_value if not found or not a valid floating point value
  // according to strtod().
  double GetReal(const std::string& section, const std::string& name,
                 double default_value) const;

  // Get a boolean value from INI file, returning default_value if not found or
  // if not a valid true/false value. Valid true values are "true", "yes", "on",
  // "1", and valid false values are "false", "no", "off", "0" (not case
  // sensitive).
  bool GetBoolean(const std::string& section, const std::string& name,
                  bool default_value) const;

  // Return true if the given section exists (section must contain at least
  // one name=value pair).
  bool HasSection(const std::string& section) const;

  // Return true if a value exists with the given section and field names.
  bool HasValue(const std::string& section, const std::string& name) const;

  // Get a vector value from INI file, returning default_value if not found,
  // empty, or contains only whitespace.
  template <typename Type>
  std::vector<Type> GetArray(const std::string& section,
                             const std::string& name, bool& is_default,
                             const std::vector<Type>& default_value = {}) {
    auto str = Get(section, name, {});
    if (str.empty()) {
      is_default = true;
      return default_value;
    }
    std::replace(str.begin(), str.end(), ',', ' ');
    std::istringstream sstream(str);
    sstream.setf(std::ios::skipws);
    std::vector<Type> result{};
    while (!sstream.eof()) {
      Type value{};
      sstream >> value;
      if (sstream.fail()) {
        is_default = true;
        return default_value;
      }
      result.push_back(value);
    }
    return result;
  }

  template <typename Type>
  std::vector<Type> GetArray(const std::string& section,
                             const std::string& name,
                             const std::vector<Type>& default_value = {}) {
    bool is_default{};
    return GetArray(section, name, is_default, default_value);
  }

 private:
  int _error;
  std::string _filename;
  std::map<std::string, std::string> _values;
  static std::string MakeKey(const std::string& section,
                             const std::string& name);
  static int ValueHandler(void* user, const char* section, const char* name,
                          const char* value);
};

#endif  // __INIREADER_H__
