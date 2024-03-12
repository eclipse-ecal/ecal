import unittest
import nanobind_core as ecal

class TestConsoleConfiguration(unittest.TestCase):
    
    def setUp(self):
        """Create an instance of ConsoleConfiguration before each test."""
        self.console_config = ecal.ConsoleConfiguration()

    def test_default_values(self):
        """Test the default values of the ConsoleConfiguration."""
        self.assertTrue(self.console_config.enable)
        self.assertEqual(self.console_config.filter_log_con, ecal.eCAL_Logging_Filter.log_filter_default)

    def test_enable_property(self):
        """Test setting the enable property."""
        self.console_config.enable = False
        self.assertFalse(self.console_config.enable)

        self.console_config.enable = True
        self.assertTrue(self.console_config.enable)

    def test_filter_log_con_property(self):
        """Test setting the filter_log_con property."""
        self.console_config.filter_log_con = ecal.eCAL_Logging_Filter.log_level_info
        self.assertEqual(self.console_config.filter_log_con, ecal.eCAL_Logging_Filter.log_level_info)


class TestFileConfiguration(unittest.TestCase):
    
    def setUp(self):
        """Create an instance of FileConfiguration before each test."""
        self.file_config = ecal.FileConfiguration()

    def test_default_values(self):
        """Test the default values of the FileConfiguration."""
        self.assertFalse(self.file_config.enable)
        self.assertEqual(self.file_config.path, "")
        self.assertEqual(self.file_config.filter_log_file, ecal.eCAL_Logging_Filter.log_level_none)

    def test_enable_property(self):
        """Test setting the enable property."""
        self.file_config.enable = True
        self.assertTrue(self.file_config.enable)

        self.file_config.enable = False
        self.assertFalse(self.file_config.enable)

    def test_path_property(self):
        """Test setting the path property."""
        self.file_config.path = "/tmp/my_log_file.log"
        self.assertEqual(self.file_config.path, "/tmp/my_log_file.log")

    def test_filter_log_file_property(self):
        """Test setting the filter_log_file property."""
        self.file_config.filter_log_file = ecal.eCAL_Logging_Filter.log_level_error
        self.assertEqual(self.file_config.filter_log_file, ecal.eCAL_Logging_Filter.log_level_error)


class TestUDPConfiguration(unittest.TestCase):
    
    def setUp(self):
        """Create an instance of UDPConfiguration before each test."""
        self.udp_config = ecal.UDPConfiguration()

    def test_default_values(self):
        """Test the default values of the UDPConfiguration."""
        self.assertTrue(self.udp_config.enable)
        self.assertEqual(self.udp_config.port, 14001)
        self.assertEqual(self.udp_config.filter_log_udp, ecal.eCAL_Logging_Filter.log_filter_default)

    def test_enable_property(self):
        """Test setting the enable property."""
        self.udp_config.enable = False
        self.assertFalse(self.udp_config.enable)

        self.udp_config.enable = True
        self.assertTrue(self.udp_config.enable)

    def test_port_property(self):
        """Test setting the port property."""
        self.udp_config.port = 15000
        self.assertEqual(self.udp_config.port, 15000)

    def test_filter_log_udp_property(self):
        """Test setting the filter_log_udp property."""
        self.udp_config.filter_log_udp = ecal.eCAL_Logging_Filter.log_level_warning
        self.assertEqual(self.udp_config.filter_log_udp, ecal.eCAL_Logging_Filter.log_level_warning)


class TestSinksConfiguration(unittest.TestCase):
    
    def setUp(self):
        """Create an instance of SinksConfiguration before each test."""
        self.sinks_config = ecal.SinksConfiguration()

    def test_default_values(self):
        """Test the default values of the SinksConfiguration."""
        self.assertTrue(self.sinks_config.console.enable)
        self.assertFalse(self.sinks_config.file.enable)
        self.assertTrue(self.sinks_config.udp.enable)
        self.assertEqual(self.sinks_config.udp.port, 14001)

    def test_console_configuration(self):
        """Test the console configuration inside sinks."""
        self.sinks_config.console.enable = False
        self.assertFalse(self.sinks_config.console.enable)

    def test_file_configuration(self):
        """Test the file configuration inside sinks."""
        self.sinks_config.file.enable = True
        self.sinks_config.file.path = "/tmp/test_log.log"
        self.assertTrue(self.sinks_config.file.enable)
        self.assertEqual(self.sinks_config.file.path, "/tmp/test_log.log")

    def test_udp_configuration(self):
        """Test the UDP configuration inside sinks."""
        self.sinks_config.udp.port = 16000
        self.assertEqual(self.sinks_config.udp.port, 16000)


class TestLoggingConfiguration(unittest.TestCase):
    
    def setUp(self):
        """Create an instance of LoggingConfiguration before each test."""
        self.logging_config = ecal.LoggingConfiguration()

    def test_default_values(self):
        """Test the default values of the LoggingConfiguration."""
        self.assertIsInstance(self.logging_config.sinks, ecal.SinksConfiguration)

    def test_modify_sinks(self):
        """Test modifying sinks in LoggingConfiguration."""
        self.logging_config.sinks.console.enable = False
        self.assertFalse(self.logging_config.sinks.console.enable)
        self.logging_config.sinks.file.path = "/var/log/my_app.log"
        self.assertEqual(self.logging_config.sinks.file.path, "/var/log/my_app.log")


if __name__ == "__main__":
    unittest.main()
