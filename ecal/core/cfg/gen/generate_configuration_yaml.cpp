#include <fstream>
#include <sstream>

#include "config/default_configuration.h"
#include "ecal_def.h"

int main() {
    std::ofstream outFile(ECAL_DEFAULT_CFG);
    
    if (!eCAL::Config::dumpConfigToFile(eCAL::GetConfiguration()))
    {
        std::cerr << "Failed to write configuration to file: " << ECAL_DEFAULT_CFG << std::endl;
        return 1;
    }

    return 0;
}