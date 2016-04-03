#pragma once

#include <tinyformat/tinyformat.h>

#include <exception>
#include <string>

//! Exception class.
class Exception : public std::runtime_error {
public:
    template<typename... Args>
    Exception(const char *fmt, const Args &... args) :
        std::runtime_error(tfm::format(fmt, args...))
    {}
};

//! Debug system.
class Debug {
public:
    static bool enabled; //< True if debug output is enabled.
};

//! Prints a debug message.
template<typename... Args>
static inline void DBG(const char *fmt, const Args &... args) {
    if (Debug::enabled) {
        std::cout << tfm::format(fmt, args...) << std::endl;
    }
}
