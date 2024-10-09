#include "config/default_configuration.h"

int main() {
    if (!eCAL::Config::dumpConfigToFile())
    {
        std::cerr << "Failed to write default configuration to file." << std::endl;
        return 1;
    }

    return 0;
}