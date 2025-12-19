#include <ecal/config/configuration.h>
#include <gtest/gtest.h>
#include <ecal/ecal.h>

namespace eCAL
{
  void PrintTo(const Configuration& config, std::ostream* os);
}

eCAL::Configuration GetTestingConfig();

eCAL::Configuration EnableUDP(const eCAL::Configuration& config_);

eCAL::Configuration EnableTCP(const eCAL::Configuration& config_);

eCAL::Configuration EnableSHM(const eCAL::Configuration& config_);

struct TestParams
{
  eCAL::Configuration config;
  std::string ecal_process_name;
};

void PrintTo(const TestParams& config, std::ostream* os);

// test fixture class
class TestFixture : public ::testing::TestWithParam<TestParams>
{
protected:
  eCAL::Configuration config;

  void SetUp() override
  {
    const auto& param = GetParam();
    config = param.config;
    config.communication_mode = eCAL::eCommunicationMode::local;
    config.registration.local.transport_type = eCAL::Registration::Local::eTransportType::shm;
    eCAL::Initialize(config, param.ecal_process_name);
  }

  void TearDown() override
  {
    eCAL::Finalize();
  }
};
