#include <ecal_config_helper.h>
#include <ecal/ecal.h>
#include <tclap/CmdLine.h>

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <map>
#include <set>

struct ProcessStatus
{
  std::chrono::steady_clock::time_point first_seen;
  std::chrono::steady_clock::time_point last_seen;
  bool seen = false;
};

int main(int argc, char* argv[])
{
  try
  {
    TCLAP::CmdLine cmd("eCAL Monitoring", ' ', "1.0");
    TCLAP::ValueArg<std::string> mode_arg("m", "mode", "eCAL transport mode", true, "", "string");
    cmd.add(mode_arg);
    cmd.parse(argc, argv);

    std::string node_name = std::string(argv[0]);
    setup_ecal_configuration(mode_arg.getValue(), false, node_name);

    std::cout << "\n[Monitoring] Started process monitor (mode=" << mode_arg.getValue()
              << ", node=" << node_name << ")" << std::endl;

    const std::vector<std::string> process_names = {
      "/app/src/build/test_publisher",
      "/app/src/build/test_subscriber",
      "/app/src/build/crash_subscriber"
    };

    std::map<std::string, ProcessStatus> monitored_processes;
    for (const auto& name : process_names)
    {
      monitored_processes[name] = ProcessStatus{};
    }

    const int max_duration_sec = 30;
    auto start_time = std::chrono::steady_clock::now();

    while (eCAL::Ok())
    {
      auto now = std::chrono::steady_clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
      if (elapsed >= max_duration_sec)
      {
        std::cout << "\n[Monitoring] Exiting after " << max_duration_sec << " seconds." << std::endl;
        break;
      }

      eCAL::Monitoring::SMonitoring mon;
      if (!eCAL::Monitoring::GetMonitoring(mon, eCAL::Monitoring::Entity::Process))
      {
        std::cerr << "\n[Monitoring] Failed to get monitoring data." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        continue;
      }

      std::set<std::string> current;
      for (const auto& proc : mon.processes)
      {
        current.insert(proc.process_name);
      }

      for (auto& [proc_name, status] : monitored_processes)
      {
        if (current.count(proc_name))
        {
          if (!status.seen)
          {
            status.first_seen = now;
            status.seen = true;
            std::cout << "[✓] Process " << proc_name << " appeared." << std::endl;
          }
          status.last_seen = now;
        }
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n[Monitoring] Summary after " << max_duration_sec << " seconds (not always accurate output):\n";
    for (const auto& [proc_name, status] : monitored_processes)
    {
      if (status.seen)
      {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(status.last_seen - status.first_seen).count();
        std::cout << " - " << proc_name << ": ran for " << duration << " seconds.\n";
      }
      else
      {
        std::cout << " - " << proc_name << ": never appeared.\n";
      }
    }

    eCAL::Finalize();
  }
  catch (const TCLAP::ArgException& e)
  {
    std::cerr << "\nTCLAP error: " << e.error() << " (arg: " << e.argId() << ")" << std::endl;
    return 1;
  }

  return 0;
}
