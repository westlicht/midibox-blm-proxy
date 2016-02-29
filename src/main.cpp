
#include "BLM.h"
#include "Midi.h"

#include <iostream>
#include <chrono>
#include <thread>

int main()
{
    std::cout << "blm-proxy" << std::endl;

    BLM blm;

    while (true) {
        Midi::update();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        blm.dump();
        std::cout << std::endl;
    }

    return 0;
}