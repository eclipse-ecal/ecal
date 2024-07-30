#include <fstream>
#include <sstream>

#include "config/default_configuration.h"
#include "ecal_def.h"

int main() {
    std::ofstream outFile(ECAL_DEFAULT_CFG);
    
    if (outFile.is_open()) {
        auto config_ss = eCAL::Config::getConfigAsYamlSS();
        outFile << config_ss.str();
        outFile.close();
    }
    else {
        std::cerr << "Error: Could not generate file " << ECAL_DEFAULT_CFG << std::endl;
        return 1;
    }

    return 0;
}