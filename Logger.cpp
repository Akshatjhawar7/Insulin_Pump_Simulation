#include "Logger.h"
#include <iostream>
#include <iomanip>
#include <sstream>

int Logger::s_seconds = 0;

void Logger::log(const std::string &text) {
    int mm = s_seconds / 60;
    int ss = s_seconds % 60;

    std::ostringstream oss;
    oss << "["
        << std::setw(2) << std::setfill('0') << mm
        << ":"
        << std::setw(2) << std::setfill('0') << ss
        << "] "
        << text;

    std::cout << oss.str() << std::endl;
    s_seconds += 5;
}
