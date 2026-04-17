#include "Debug.hpp"
#include "../include/Defines.hpp"

#include <iostream> 
#include <cstdarg>    
#include <cstdio>     
#include <unistd.h>  
#include <chrono>   
#include <iomanip>   
#include <ctime> 

#define CLR_RESET   "\033[0m"
#define CLR_CYAN "\033[36m"
#define CLR_GREEN   "\033[32m"
#define CLR_YELLOW  "\033[33m"
#define CLR_RED     "\033[31m"
#define CLR_MAGENTA "\033[35m"

static bool use_color() {
    return isatty(STDOUT_FILENO);
}

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

static const char* level_to_color(LogLevel level) {
    switch(level) {
        case LogLevel::DEBUG:    return CLR_CYAN;
        case LogLevel::INFO:     return CLR_GREEN;
        case LogLevel::WARN:     return CLR_YELLOW;
        case LogLevel::ERROR:    return CLR_RED;
        case LogLevel::CRITICAL: return CLR_MAGENTA;
        default:                 return CLR_RESET;
    }
}

static void vlog_message(LogLevel level, const char* fmt, va_list args) {
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    const char* level_str = level_to_string(level);
    const char* color = use_color() ? level_to_color(level) : "";
    const char* reset = use_color() ? CLR_RESET : "";

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_r(&in_time_t, &tm_buf); 

    std::ostream& out =
        (level == LogLevel::ERROR || level == LogLevel::CRITICAL)
        ? std::cerr
        : std::cout;

    if (use_color()) {
        out << "\033[34m" 
            << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "]"
            << "\033[0m ";
    } else {
        out << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "] ";
    }

    out << color
        << "[" << level_str << "] "
        << buffer
        << reset
        << std::endl;
}

void log_message(LogLevel level, const char* fmt, ...) {
    if(!ENABLE_DEBUG){
        return;
    }

    va_list args;
    va_start(args, fmt);
    vlog_message(level, fmt, args);
    va_end(args);
}

void log_debug(const char* fmt, ...) {
    if(!ENABLE_DEBUG){
        return;
    }

    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::DEBUG, fmt, args);
    va_end(args);
}

void log_info(const char* fmt, ...) {
    if(!ENABLE_DEBUG){
        return;
    }

    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::INFO, fmt, args);
    va_end(args);
}

void log_warn(const char* fmt, ...) {
    if(!ENABLE_DEBUG){
        return;
    }

    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::WARN, fmt, args);
    va_end(args);
}

void log_error(const char* fmt, ...) {
    if(!ENABLE_DEBUG){
        return;
    }

    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::ERROR, fmt, args);
    va_end(args);
}

void log_critical(const char* fmt, ...) {
    if(!ENABLE_DEBUG){
        return;
    }
    
    va_list args;
    va_start(args, fmt);
    vlog_message(LogLevel::CRITICAL, fmt, args);
    va_end(args);
}