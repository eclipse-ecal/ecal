#include "configuration_reader.h"
#include "util/yaml_functions.h"

namespace 
{
  void MapConfiguration(const YAML::Node& node_, eCAL::Configuration& config_)
  {
    config_ = node_.as<eCAL::Configuration>();
  }
}


namespace eCAL
{
  namespace Config
  {    
    void YamlFileToConfig(const std::string& filename_, eCAL::Configuration& config_)
    {
      const YAML::Node yaml = YAML::LoadFile(filename_);

      MapConfiguration(yaml, config_);
    }

    void YamlStringToConfig(const std::string& yaml_string_, eCAL::Configuration& config_)
    {
      const YAML::Node yaml = YAML::Load(yaml_string_);

      MapConfiguration(yaml, config_);
    }

    bool ConfigToYamlFile(const std::string& file_name_, const eCAL::Configuration& config_)
    {
      const YAML::Node node(config_);
      std::ofstream file(file_name_);
      if (file.is_open())
      {
        file << node;
        file.close();
        return true;
      }
      
      return false;
    }

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

      eCAL::Util::MergeYamlNodes(config_node, node_to_merge);
      MapConfiguration(config_node, config_);

      return true;
    }
  }  
}