#include "configuration_reader.h"

namespace 
{
  void MapConfiguration(const YAML::Node& node_, eCAL::Configuration& config_)
  {
    YAML::AssignValue<eCAL::TransportLayer::Configuration>(config_.transport_layer, node_, "transport_layer");
    YAML::AssignValue<eCAL::Publisher::Configuration>(config_.publisher, node_, "publisher");
    YAML::AssignValue<eCAL::Subscriber::Configuration>(config_.subscriber, node_, "subscriber");
    YAML::AssignValue<eCAL::Registration::Configuration>(config_.registration, node_, "registration");
    YAML::AssignValue<eCAL::Monitoring::Configuration>(config_.monitoring, node_, "monitoring");
    YAML::AssignValue<eCAL::Time::Configuration>(config_.timesync, node_, "time");
    YAML::AssignValue<eCAL::Service::Configuration>(config_.service, node_, "service");
    YAML::AssignValue<eCAL::Application::Configuration>(config_.application, node_, "application");
    YAML::AssignValue<eCAL::Logging::Configuration>(config_.logging, node_, "logging");
  }
}


namespace eCAL
{
  namespace Config
  {    
    void YamlFileToConfig(const std::string& filename_, eCAL::Configuration& config_)
    {
      YAML::Node yaml = YAML::LoadFile(filename_);

      MapConfiguration(yaml, config_);
    };

    void YamlStringToConfig(const std::string& yaml_string_, eCAL::Configuration& config_)
    {
      YAML::Node yaml = YAML::Load(yaml_string_);

      MapConfiguration(yaml, config_);
    };

    bool ConfigToYamlFile(const std::string& file_name_, const eCAL::Configuration& config_)
    {
      YAML::Node node(config_);
      std::ofstream file(file_name_);
      if (file.is_open())
      {
        file << node;
        file.close();
        return true;
      }
      
      return false;
    };

    void MergeYamlNodes(YAML::Node& base, const YAML::Node& other) 
    {
      std::stack<std::pair<YAML::Node, YAML::Node>> nodes;
      nodes.push(std::make_pair(base, other));

      while (!nodes.empty()) 
      {
        std::pair<YAML::Node, YAML::Node> nodePair = nodes.top();
        nodes.pop();

        YAML::Node baseNode = nodePair.first;
        YAML::Node otherNode = nodePair.second;

        for (YAML::const_iterator it = otherNode.begin(); it != otherNode.end(); ++it) 
        {
          YAML::Node key = it->first;
          YAML::Node value = it->second;
          
          std::string key_as_string = "";

          switch (key.Type())
          {
            case YAML::NodeType::Scalar:
              key_as_string = key.as<std::string>();
              break;
            default:
              continue;
              break;
          }

          if (baseNode[key_as_string]) 
          {
            if (value.IsMap() && baseNode[key_as_string].IsMap()) 
            {
              nodes.push(std::make_pair(baseNode[key_as_string], value)); // Push nested nodes to stack
            } 
            else 
            {
              baseNode[key_as_string] = value; // Overwrite value for non-map nodes
            }
          } 
          else 
          {
            baseNode[key_as_string] = value; // Add new key-value pairs
          }
        }
      }
    };

    bool MergeYamlIntoConfiguration(const std::string& file_name_ , eCAL::Configuration& config_)
    {
      YAML::Node config_node(config_);
      
      YAML::Node node_to_merge;
      try
      {
        node_to_merge = YAML::LoadFile(file_name_);
      }
      catch (std::exception& e)
      {
        std::cout << "Error during reading yml file: " << e.what() << "\n";
        return false;
      }

      eCAL::Config::MergeYamlNodes(config_node, node_to_merge);
      MapConfiguration(config_node, config_);

      return true;
    }
  }  
}