#include "Logger.hpp"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <unistd.h>

#define CLR_RESET       "\033[0m"

#define CLR_TIMER       "\033[37m"

#define CLR_DEBUG       "\033[34m"
#define CLR_INFO        "\033[32m"
#define CLR_WARN        "\033[33m"
#define CLR_ERROR       "\033[31m"
#define CLR_CRITICAL    "\033[1;31m"

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
        case LogLevel::DEBUG:    return CLR_DEBUG;
        case LogLevel::INFO:     return CLR_INFO;
        case LogLevel::WARN:     return CLR_WARN;
        case LogLevel::ERROR:    return CLR_ERROR;
        case LogLevel::CRITICAL: return CLR_CRITICAL;
        default:                 return CLR_RESET;
    }
}

void Logger::VLogMessage(LogLevel level, const char* fmt, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    const char* levelColor = UseColor() ? LevelToColor(level) : "";
    const char* timerColor = UseColor() ? CLR_TIMER : "";
    const char* reset = UseColor() ? CLR_RESET : "";

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t in_time_t = std::chrono::system_clock::to_time_t(now);

    std::tm tm_buf;
    localtime_r(&in_time_t, &tm_buf);

    std::ostream& out = (level == LogLevel::ERROR || level == LogLevel::CRITICAL) ? std::cerr : std::cout;

    if (UseColor()) {
        out << timerColor;
    }

    out << '['
        << std::put_time(&tm_buf, "%H:%M:%S")
        << '.'
        << std::setw(3)
        << std::setfill('0')
        << ms.count()
        << ']';

    if (UseColor()) {
        out << reset;
    }

    if (UseColor()) {
        out << ' '
            << levelColor
            << '[' << LevelToString(level) << ']'
            << reset
            << ' ';
    } else {
        out << " [" << LevelToString(level) << "] ";
    }

    out << buffer << std::endl;
}

void Logger::Log(LogLevel level, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    VLogMessage(level, fmt, args);
    va_end(args);
}