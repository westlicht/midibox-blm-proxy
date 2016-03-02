#pragma once

#include <tinyformat/tinyformat.h>

#include <exception>
#include <string>

class Exception : public std::runtime_error {
public:
    template<typename... Args>
    Exception(const char *fmt, const Args &... args) :
        std::runtime_error(tfm::format(fmt, args...))
    {}
};

class Debug {
public:
    static bool enabled;
};

template<typename... Args>
static inline void DBG(const char *fmt, const Args &... args) {
    if (Debug::enabled) {
        std::cout << tfm::format(fmt, args...) << std::endl;
    }
}
