#ifndef LOGGER_H
#define LOGGER_H

#include <string>

class Logger {
public:
    static void log(const std::string &text);

private:
    static int s_seconds;
};

#endif // LOGGER_H
