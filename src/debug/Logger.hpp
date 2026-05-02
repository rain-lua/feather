#pragma once

#include <cstdarg>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

class Logger {
public:
    static void Log(LogLevel level, const char* fmt, ...);
    static void VLogMessage(LogLevel level, const char* fmt, va_list args);

    static bool UseColor();

    static const char* LevelToString(LogLevel level);
    static const char* LevelToColor(LogLevel level);
};