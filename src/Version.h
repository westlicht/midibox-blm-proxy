#pragma once

#include <tinyformat/tinyformat.h>

#include <string>

namespace Version {

static const int Major = 0;
static const int Minor = 1;
static const int Revision = 0;

static std::string string() {
    return tfm::format("%d.%d.%d", Major, Minor, Revision);
}

} // namespace Version