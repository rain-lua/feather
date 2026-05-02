#include "Logger.hpp"

#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <chrono>
#include <iomanip>
#include <ctime>

#define CLR_RESET   "\033[0m"
#define CLR_CYAN    "\033[36m"
#define CLR_GREEN   "\033[32m"
#define CLR_YELLOW  "\033[33m"
#define CLR_RED     "\033[31m"
#define CLR_MAGENTA "\033[35m"

bool Logger::UseColor() {
    return isatty(STDOUT_FILENO);
}

const char* Logger::LevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARN:     return "WARN";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

const char* Logger::LevelToColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG:    return CLR_CYAN;
        case LogLevel::INFO:     return CLR_GREEN;
        case LogLevel::WARN:     return CLR_YELLOW;
        case LogLevel::ERROR:    return CLR_RED;
        case LogLevel::CRITICAL: return CLR_MAGENTA;
        default:                 return CLR_RESET;
    }
}

void Logger::VLogMessage(LogLevel level, const char* fmt, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    const char* color = UseColor() ? LevelToColor(level) : "";
    const char* reset = UseColor() ? CLR_RESET : "";

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf;
    localtime_r(&in_time_t, &tm_buf);

    std::ostream& out = (level == LogLevel::ERROR || level == LogLevel::CRITICAL) ? std::cerr : std::cout;

    if (UseColor()) {
        out << "\033[34m" << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "]" << "\033[0m ";
    } else {
        out << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "] ";
    }

    out << color << "[" << LevelToString(level) << "] " << buffer << reset << std::endl;
}

void Logger::Log(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    VLogMessage(level, fmt, args);
    va_end(args);
}