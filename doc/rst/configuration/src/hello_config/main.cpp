#include <ecal/ecal.h>
#include <ecal/ecal_config.h>
#include <string>
#include <stdexcept>

int main(int argc, char** argv)
{
  // Create a configuration object with the command line arguments
  eCAL::Configuration custom_config(argc, argv);

  // Use the .yaml file of the system if available
  custom_config.InitConfigWithDefaultYaml();

  // Set the values in a try/catch block, as wrong configuration leads to exceptions
  try
  {
      // In case you decided to specify an own .yaml file to use
      // Configuration based on previous ini file will be overwritten
    custom_config.InitConfigFromFile("C:\\eCAL_local.yaml");

    // Set the communication layer to network
    custom_config.registration.network_enabled = true;

    // Set a custom udp multicast group, correct IP address necessary
    custom_config.transport_layer.udp.network.group = std::string("239.0.1.1");

    // Increase the send buffer, size increase in 1024 bytes steps
    custom_config.transport_layer.udp.send_buffer = (5242880 + 10 * 1024);
  }
  catch (std::invalid_argument& e)
  {
    throw std::runtime_error("Error while configuring eCALConfig: " + std::string(e.what()));
  }

  // Initialize eCAL with the prepared configuration object
  eCAL::Initialize(custom_config, "UserConfigExample", eCAL::Init::Default);

  // ...
  // Use eCAL for your needs
  // ...

  // Finalize eCAL API
  eCAL::Finalize();

  return 0;
}