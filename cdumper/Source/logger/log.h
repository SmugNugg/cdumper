#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <windows.h>
#include <string>

class Logger {
public:
    enum class Level {
        INFO,
        SUCCESS,
        WARNING,
        PROBLEM
    };

    static void log(Level level, const std::wstring& message);

private:
    static void setConsoleColor(WORD color);
    static WORD getColorForLevel(Level level);
};

#endif // LOG_H