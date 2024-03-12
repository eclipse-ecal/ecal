import unittest
import nanobind_core as ecal_module

class TestECALConfiguration(unittest.TestCase):

    def setUp(self):
        """Set up the main configuration object for testing."""
        self.config = ecal_module.Configuration()

    def test_default_initialization(self):
        """Test that each component has the correct default values."""
        # Check transport layer configuration
#        self.assertIsInstance(self.config.transport_layer, ecal_module.TransportLayerConfiguration)
        
        # Check registration configuration
        self.assertIsInstance(self.config.registration, ecal_module.Registration.RegistrationConfiguration)

        # Check monitoring configuration
        self.assertIsInstance(self.config.monitoring, ecal_module.MonitoringConfiguration)

        # Check subscriber configuration
        self.assertIsInstance(self.config.subscriber, ecal_module.Subscriber.SubscriberConfiguration)

        # Check publisher configuration
        self.assertIsInstance(self.config.publisher, ecal_module.Publisher.PublisherConfiguration)

        # Check time synchronization configuration
        self.assertIsInstance(self.config.timesync, ecal_module.TimeConfiguration)

        # Check service configuration
        self.assertIsInstance(self.config.service, ecal_module.ServiceConfiguration)

        # Check application configuration
        self.assertIsInstance(self.config.application, ecal_module.ApplicationConfiguration)

        # Check logging configuration
        self.assertIsInstance(self.config.logging, ecal_module.LoggingConfiguration)

        # Check CLI configuration
        self.assertIsInstance(self.config.command_line_arguments, ecal_module.CliConfiguration)

    def test_init_from_config(self):
        """Test initializing from default configuration."""
        self.config.InitFromConfig()
        # Verify that InitFromConfig can be called without errors
        # Assuming InitFromConfig populates with default values

if __name__ == "__main__":
    unittest.main()
