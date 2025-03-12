#include <ecal/ecal.h>
#include <ecal/config.h>
#include <string>
#include <stdexcept>

int main(int argc, char** argv)
{
  // Create a configuration object with the command line arguments
  eCAL::Configuration custom_config;

  // Use the ecal.yaml file of the system, local data or current folder if available
  custom_config.InitFromConfig();

  // In case you decide to specify an own .yaml file to use.
  // Configuration based on previous ini file will be overwritten.
  // The new Configuration will use default values if the file is not found
  // in the default directories (ECAL_DATA, cwd, %LOCALAPPDATA%/eCAL, $HOME/.ecal)
  custom_config.InitFromFile("./ecal_custom.yaml");

  // Furthermore you can adapt the configuration by overwriting values from the file, e.g.
  // set the communication mode to network
  custom_config.communication_mode = eCAL::eCommunicationMode::network;

  // Increase the send buffer, size increase in 1024 bytes steps
  custom_config.transport_layer.udp.send_buffer = (5242880 + 10 * 1024);
  
  // Set the network addresses in a try/catch block, as wrong configuration 
  // leads to exceptions
  try
  {
    // Set a custom udp multicast group, correct IP address necessary
    custom_config.transport_layer.udp.network.group = std::string("239.0.1.1"); 
  }
  catch (std::invalid_argument& e)
  {
    // Continue with the program, but let the user of the exception and the used network group
    std::cout << "Error while configuring network group: " + std::string(e.what()) << "\n";
    std::cout << "Using default network group: " << custom_config.transport_layer.udp.network.group.Get() << "\n";
  }

  // Initialize eCAL with the prepared configuration object
  eCAL::Initialize(custom_config, "UserConfigExample", eCAL::Init::Default);

  // ...
  // Use eCAL for your needs
  // ...

  while (eCAL::Ok())
  {
    // ...
  }

  // Finalize eCAL API
  eCAL::Finalize();

  return 0;
}