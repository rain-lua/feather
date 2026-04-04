#include <iostream>
#include <cstdarg>

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

static const char* level_to_string(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG:    return "DEBUG";
        case LogLevel::INFO:     return "INFO";
        case LogLevel::WARN:     return "WARN";
        case LogLevel::ERROR:    return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:                 return "UNKNOWN";
    }
}

void vlog_message(LogLevel level, const char* fmt, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    std::cout << "[" << level_to_string(level) << "] " << buffer << std::endl;
}

void log_debug(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::DEBUG, fmt, args);
    va_end(args);
}

void log_info(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::INFO, fmt, args);
    va_end(args);
}

void log_warn(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::WARN, fmt, args);
    va_end(args);
}

void log_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::ERROR, fmt, args);
    va_end(args);
}

void log_critical(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::CRITICAL, fmt, args);
    va_end(args);
}