#ifndef UTIL_H
#define UTIL_H

#include <cstdint>

void spawn(const char* name);

static inline uint32_t toXKBKeycode(uint32_t keycode) {
    return keycode + 8;
}

#endif