
#include "BLM.h"
#include "LaunchpadController.h"
#include "Midi.h"
#include "Timer.h"

#include <cxxopts/cxxopts.h>

#include <iostream>
#include <chrono>
#include <thread>

int main(int argc, char *argv[])
{
    cxxopts::Options options(argv[0], " - MIDIbox BLM proxy");

    bool listDevices = false;

    options.add_options()
    ("h,help", "Print help")
    ("l,list-midi-devices", "List MIDI devices", cxxopts::value<bool>(listDevices))
//    ("task", tfm::format("Processing task (default: %s)", settings.task), cxxopts::value<std::string>(settings.task), "")
//    ("startFrame", tfm::format("First frame to process (default: %d)", settings.startFrame), cxxopts::value<int>(settings.startFrame), "")
//    ("endFrame", tfm::format("Last frame to process (default: %d)", settings.endFrame), cxxopts::value<int>(settings.endFrame), "")
//    ("input", "Input files", cxxopts::value<std::vector<std::string>>())
    ;

    // Parse command line arguments
    try {
        options.parse(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "Error during command line parsing: " << e.what() << std::endl;
        return 1;
    }

    // Show help if requested
    if (options.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    if (listDevices) {
        Midi::update();
        std::cout << "Midi Inputs:" << std::endl;
        for (const auto port : Midi::inputPorts()) {
            std::cout << "  " << port << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Midi Outputs:" << std::endl;
        for (const auto port : Midi::outputPorts()) {
            std::cout << "  " << port << std::endl;
        }
        std::cout << std::endl;
        return 0;
    }

    BLM blm;
    LaunchpadController controller;
    blm.setController(&controller);

    while (true) {
        Midi::update();
        Timer::updateTimers();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        //blm.dump();
        //std::cout << std::endl;
    }

    blm.setController(nullptr);

    return 0;
}