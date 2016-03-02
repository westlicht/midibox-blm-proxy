#pragma once

#include <filesystem/path.h>
#include <json11/json11.h>

class Settings {
public:
    void load(const filesystem::path &path);
    void save(const filesystem::path &path) const;

    const json11::Json &json() const { return _json; }
          json11::Json &json()       { return _json; }

    static Settings &instance();

private:
    json11::Json _json;
};

