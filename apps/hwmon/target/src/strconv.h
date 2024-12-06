#ifndef STRCONV_H
#define STRCONV_H

#include <vector>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <iomanip>

//------------------------------------------------------------------------------

template <typename T> std::string toString(T val)
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}

//------------------------------------------------------------------------------

template<typename T> T fromString(const std::string& s)
{
    std::istringstream iss(s);
    T res;
    if(strstr(s.c_str(),"0x")) {
        iss >> std::hex >> res;
    } else {
        iss >> res;
    }
    return res;
}

//------------------------------------------------------------------------------

#endif // STRCONV_H
