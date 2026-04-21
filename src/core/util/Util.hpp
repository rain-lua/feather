#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include "../compositor/managers/WindowManager.hpp"

void Spawn(const char* name);

static inline uint32_t ToXKBKeycode(uint32_t keycode) {
    return keycode + 8;
}

#endif