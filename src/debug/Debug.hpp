#ifndef DEBUG_H
#define DEBUG_H

enum class LogLevel {
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

void log_message(LogLevel level, const char* fmt, ...);

void log_debug(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void log_info(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void log_warn(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void log_error(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
void log_critical(const char* fmt, ...) __attribute__((format(printf, 1, 2)));

#endif