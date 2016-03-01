
#include "BLM.h"
#include "LaunchpadController.h"
#include "Midi.h"
#include "Timer.h"

#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    std::cout << "blm-proxy" << std::endl;

#if 0
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
#endif

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