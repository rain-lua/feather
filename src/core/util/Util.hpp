#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include "../compositor/managers/WindowManager.hpp"

void spawn(const char* name);
void kill(Window *window);

static inline uint32_t toXKBKeycode(uint32_t keycode) {
    return keycode + 8;
}

#endif