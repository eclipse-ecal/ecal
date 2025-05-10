#include "MonitoringLibrary.h"
#include <iostream>
#include <ecal/monitoring.h>

MonitoringLibrary::MonitoringLibrary() : logging_enabled(false) {}

MonitoringLibrary::~MonitoringLibrary() {}

void MonitoringLibrary::Initialize(bool enable_logging)
{
    logging_enabled = enable_logging;
    std::cout << "[MonitoringLibrary] Initialized. Logging: " 
              << (logging_enabled ? "Enabled" : "Disabled") << std::endl;
}

void MonitoringLibrary::UpdateMonitoring()
{
    std::string monitoring_string;
    eCAL::Monitoring::GetMonitoring(monitoring_string, eCAL::Monitoring::Entity::All);

    if (!monitoring_data.ParseFromString(monitoring_string))
    {
        std::cerr << "[MonitoringLibrary] Failed to parse monitoring data." << std::endl;
        return;
    }

    if (logging_enabled)
    {
        LogMonitoringInfo();
    }
}

std::vector<std::string> MonitoringLibrary::GetActivePublishers() const
{
    std::vector<std::string> publishers;
    for (const auto& topic : monitoring_data.topics())
    {
        if (topic.direction() == "publisher")
        {
            publishers.push_back(topic.topic_name());
        }
    }
    return publishers;
}

std::vector<std::string> MonitoringLibrary::GetActiveSubscribers() const
{
    std::vector<std::string> subscribers;
    for (const auto& topic : monitoring_data.topics())
    {
        if (topic.direction() == "subscriber")
        {
            subscribers.push_back(topic.topic_name());
        }
    }
    return subscribers;
}

std::vector<std::string> MonitoringLibrary::GetActiveProcesses() const
{
    std::vector<std::string> processes;
    for (const auto& process : monitoring_data.processes())
    {
        processes.push_back(process.process_name());
    }
    return processes;
}

std::vector<std::string> MonitoringLibrary::GetActiveTopics() const
{
    std::vector<std::string> topics;
    for (const auto& topic : monitoring_data.topics())
    {
        topics.push_back(topic.topic_name() + " (" + topic.direction() + ")");
    }
    return topics;
}

void MonitoringLibrary::SetLoggingEnabled(bool enabled)
{
    logging_enabled = enabled;
}

void MonitoringLibrary::LogMonitoringInfo() const
{
    std::cout << "=== [Monitoring] Active Topics ===" << std::endl;
    for (const auto& topic : monitoring_data.topics())
    {
        std::cout << "Topic: " << topic.topic_name()
                  << " | Direction: " << topic.direction()
                  << " | Host: " << topic.host_name() << std::endl;
    }

    std::cout << "=== [Monitoring] Active Processes ===" << std::endl;
    for (const auto& process : monitoring_data.processes())
    {
        std::cout << "Process: " << process.process_name()
                  << " | Host: " << process.host_name()
                  << " | State: " << process.state().severity() << std::endl;
    }
}
