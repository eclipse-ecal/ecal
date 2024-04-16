#include <ecal/ecal_config.h>

int main(int argc, char** argv)
{
  // Create a configuration object with the command line arguments
  eCAL::Config::eCALConfig custom_config(argc, argv);

  // Set the communication layer to network
  custom_config.transport_layer_options.network_enabled    = true;
  
  // Initialize eCAL with the config
  eCAL::Initialize(custom_config, "UserConfig", eCAL::Init::Default);

  // ...
  // Use eCAL for your needs
  // ...

  // Finalize eCAL API
  eCAL::Finalize();
}