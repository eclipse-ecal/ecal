import unittest
import nanobind_core as ecal_module

class TestApplicationConfiguration(unittest.TestCase):

    def setUp(self):
        """Set up the application configuration object for testing."""
        self.app_config = ecal_module.ApplicationConfiguration()

    def test_default_initialization(self):
        """Test that the default values are correctly initialized."""
        # Test Sys::Configuration
        self.assertIsInstance(self.app_config.sys, ecal_module.SysConfiguration)
        self.assertEqual(self.app_config.sys.filter_excl, "^eCALSysClient$|^eCALSysGUI$|^eCALSys$")

        # Test Startup::Configuration
        self.assertIsInstance(self.app_config.startup, ecal_module.StartupConfiguration)
        self.assertEqual(self.app_config.startup.terminal_emulator, "")

    def test_modify_sys_configuration(self):
        """Test modifying the Sys::Configuration."""
        self.app_config.sys.filter_excl = "^MyApp$|^AnotherApp$"
        self.assertEqual(self.app_config.sys.filter_excl, "^MyApp$|^AnotherApp$")

    def test_modify_startup_configuration(self):
        """Test modifying the Startup::Configuration."""
        self.app_config.startup.terminal_emulator = "gnome-terminal"
        self.assertEqual(self.app_config.startup.terminal_emulator, "gnome-terminal")

    def test_nested_initialization(self):
        """Ensure nested configurations remain accessible."""
        sys_config = self.app_config.sys
        startup_config = self.app_config.startup
        self.assertIsInstance(sys_config, ecal_module.SysConfiguration)
        self.assertIsInstance(startup_config, ecal_module.StartupConfiguration)

if __name__ == "__main__":
    unittest.main()
