#include "log.h"

void Logger::setConsoleColor(WORD color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

WORD Logger::getColorForLevel(Logger::Level level) {
    switch (level) {
    case Logger::Level::INFO: return 11;      // bright Cyan
    case Logger::Level::SUCCESS: return 10;   // bright Green
    case Logger::Level::WARNING: return 14;   // bright Yellow
    case Logger::Level::PROBLEM: return 12;   // bright Red
    default: return 7;                        // White
    }
}

void Logger::log(Logger::Level level, const std::wstring& message) {
    setConsoleColor(8);
    std::wcout << L"[";
    setConsoleColor(getColorForLevel(level));
    switch (level) {
    case Logger::Level::INFO: std::wcout << L"INFO"; break;
    case Logger::Level::SUCCESS: std::wcout << L"SUCCESS"; break;
    case Logger::Level::WARNING: std::wcout << L"WARNING"; break;
    case Logger::Level::PROBLEM: std::wcout << L"ERROR"; break;
    }
    setConsoleColor(8);
    std::wcout << L"] ";
    setConsoleColor(14);
    std::wcout << message;
    setConsoleColor(8);
    std::wcout << L")" << std::endl;
    setConsoleColor(7);
}