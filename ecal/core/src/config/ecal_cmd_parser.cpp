#include "config/ecal_cmd_parser.h"

#include "ecal/ecal_defs.h"
#include "ecal_def.h"
#if ECAL_CORE_COMMAND_LINE
  #include "util/advanced_tclap_output.h"
#endif

#include "ecal_global_accessors.h"

namespace eCAL
{
  namespace Config
  {
    CmdParser::CmdParser(int argc_ , char **argv_)
    : m_dump_config{false}
    , m_config_keys{std::vector<std::string>()}
    {
#if ECAL_CORE_COMMAND_LINE
      if ((argc_ > 0) && (argv_ != nullptr))
      {
        // define command line object
        TCLAP::CmdLine cmd("", ' ', ECAL_VERSION);

        // define command line arguments
        TCLAP::SwitchArg             dump_config_arg     ("", "ecal-dump-config",    "Dump current configuration.", false);
        TCLAP::ValueArg<std::string> default_ini_file_arg("", "ecal-ini-file",       "Load default configuration from that file.", false, ECAL_DEFAULT_CFG, "string");
        TCLAP::MultiArg<std::string> set_config_key_arg  ("", "ecal-set-config-key", "Overwrite a specific configuration key (ecal-set-config-key \"section/key:value\".", false, "string");

        TCLAP::UnlabeledMultiArg<std::string> dummy_arg("__dummy__", "Dummy", false, ""); // Dummy arg to eat all unrecognized arguments

        cmd.add(dump_config_arg);
        cmd.add(default_ini_file_arg);
        cmd.add(set_config_key_arg);
        cmd.add(dummy_arg);

        CustomTclap::AdvancedTclapOutput advanced_tclap_output(&std::cout, 75);
        advanced_tclap_output.setArgumentHidden(&dummy_arg, true);
        cmd.setOutput(&advanced_tclap_output);

        // parse command line
        cmd.parse(argc_, argv_);

        // set globals
        if (dump_config_arg.isSet())
        {
          m_dump_config = true;
        }
        if (default_ini_file_arg.isSet())
        {
          g_default_ini_file = default_ini_file_arg.getValue();
        }
        if (set_config_key_arg.isSet())
        {
          m_config_keys = set_config_key_arg.getValue();
        }
      }
#endif  
      if (argv_ != nullptr)
      {
        for (size_t i = 0; i < static_cast<size_t>(argc_); ++i) if (argv_[i] != nullptr) m_task_parameter.emplace_back(argv_[i]);
      }
    }
        
    CmdParser::~CmdParser() = default;
  }
}
