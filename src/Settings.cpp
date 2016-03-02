#include "Settings.h"
#include "Debug.h"

#include <json11/json11.h>

#include <fstream>
#include <sstream>
#include <string>

using namespace json11;

void Settings::load(const filesystem::path &path)
{
    std::ifstream is(path.make_absolute().str());
    std::stringstream ss;
    ss << is.rdbuf();

    std::string err;
    _json = Json::parse(ss.str(), err);
    if (_json.is_null()) {
        throw Exception("Failed to load settings from '%s' (error: %s)", path, err);
    }

    DBG("Settings = %s", _json.dump());
}

void Settings::save(const filesystem::path &path) const
{
}

Settings &Settings::instance()
{
    static Settings instance;
    return instance;
}
