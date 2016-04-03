#pragma once

#include <filesystem/path.h>
#include <json11/json11.h>

#include <string>

//! Application settings.
class Settings {
public:
    //! Loads the application settings from the given file.
    void load(const filesystem::path &path);

    //! Notifies an error in the configuration file.
    //! Throws an exception with an error description.
    void error(const std::string &path, const std::string &message);

    //! Returns the JSON object containing the application settings.
    const json11::Json &json() const { return _json; }
          json11::Json &json()       { return _json; }

    //! Returns the global application settings instance.
    static Settings &instance();

private:
    filesystem::path _path;
    json11::Json _json;
};

