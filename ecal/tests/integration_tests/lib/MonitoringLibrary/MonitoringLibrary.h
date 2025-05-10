// MonitoringLibrary.h
#pragma once

#include <ecal/ecal.h>
#include <ecal/types/monitoring.h>
#include <ecal/types.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

//Monitoring and Protobuf
#include <ecal/monitoring.h>
#include <ecal/core/pb/monitoring.pb.h>


class MonitoringLibrary
{
public:
    MonitoringLibrary();
    ~MonitoringLibrary();

    // Initialize Monitoring Library (optional custom logging)
    void Initialize(bool enable_logging = false);

    // Update monitoring information (call this regularly in tests)
    void UpdateMonitoring();

    // Get active Publishers and Subscribers
    std::vector<std::string> GetActivePublishers() const;
    std::vector<std::string> GetActiveSubscribers() const;

    // Get active processes by name
    std::vector<std::string> GetActiveProcesses() const;

    // Get details of all active topics (publisher and subscriber)
    std::vector<std::string> GetActiveTopics() const;

    // Enable or disable logging (can be changed at runtime)
    void SetLoggingEnabled(bool enabled);

private:
    bool logging_enabled;
    eCAL::pb::Monitoring monitoring_data;
    
    // Internal helper methods
    void LogMonitoringInfo() const;
};

