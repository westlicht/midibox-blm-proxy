
#include "BLM.h"
#include "LaunchpadController.h"
#include "Midi.h"
#include "Timer.h"
#include "Settings.h"
#include "Debug.h"

#include <filesystem/path.h>
#include <cxxopts/cxxopts.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <csignal>


static bool terminate = false;

static void signalHandler(int sig)
{
    std::cout << "Terminating ..." << std::endl;
    terminate = true;
}

int main(int argc, char *argv[])
{
    cxxopts::Options options(argv[0], " - MIDIbox BLM proxy");

    bool listDevices = false;
    bool debugMode = false;
    bool textMode = false;

    options.add_options()
    ("h,help", "Print help")
    ("l,list-midi-devices", "List MIDI devices", cxxopts::value<bool>(listDevices))
    ("d,debug-mode", "Enable debug mode", cxxopts::value<bool>(debugMode))
    ("t,text-mode", "Output BLM on console", cxxopts::value<bool>(textMode))
    ;

    // Parse command line arguments
    try {
        options.parse(argc, argv);
    } catch (const std::exception &e) {
        std::cout << "Error during command line parsing: " << e.what() << std::endl;
        return 1;
    }

    // Show help if requested
    if (options.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    Debug::enabled = debugMode;

    try {
        // List MIDI devices
        if (listDevices) {
            Midi::update();
            std::cout << "Midi Inputs:" << std::endl;
            for (const auto &port : Midi::inputPorts()) {
                std::cout << "  " << port << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Midi Outputs:" << std::endl;
            for (const auto &port : Midi::outputPorts()) {
                std::cout << "  " << port << std::endl;
            }
            std::cout << std::endl;
            return 0;
        }

        // Register signal handlers
        std::signal(SIGINT, signalHandler);
        std::signal(SIGTERM, signalHandler);

        // Load settings
        std::vector<filesystem::path> candidates;
        filesystem::path current = filesystem::path(argv[0]).parent_path();
        candidates.emplace_back(current / "blm-proxy.conf");
        candidates.emplace_back(current / ".." / "blm-proxy.conf");
        candidates.emplace_back(filesystem::path("/etc") / "blm-proxy.conf");

        Settings settings;
        for (auto path : candidates) {
            if (path.exists() && path.is_file()) {
                DBG("Loading settings from %s", path);
                Settings::instance().load(path);
            }
        }

        // Create controller
        std::unique_ptr<Controller> controller;
        std::string controllerType = Settings::instance().json()["controller"]["type"].string_value();
        if (controllerType == "launchpad") {
            controller.reset(new LaunchpadController());
        } else {
            Settings::instance().error("controller.type", "Missing or invalid controller type, currently only supports \"launchpad\"!");
        }

        // Create BLM
        BLM blm;
        blm.setController(controller.get());

        // Main loop
        while (!terminate) {
            Midi::update();
            Timer::updateTimers();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            if (textMode) {
                blm.dump();
                std::cout << std::endl;
            }
        }

        blm.setController(nullptr);

    } catch (const std::exception &e) {
        std::cout << "!!! Exception !!!" << std::endl;
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}