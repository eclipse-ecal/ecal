import unittest
import nanobind_core as ecal_module

class TestConfiguration(unittest.TestCase):
    def setUp(self):
        # Create an instance of the Configuration class for each test
        self.config = ecal_module.TimeConfiguration()

    def test_default_values(self):
        # Test that the default values are as expected
        self.assertEqual(self.config.timesync_module_rt, "ecaltime-localtime")
        self.assertEqual(self.config.timesync_module_replay, "")

    def test_modify_values(self):
        # Test modifying the attributes
        self.config.timesync_module_rt = "ecaltime-linuxptp"
        self.config.timesync_module_replay = "replay-module"

        # Assert the values have changed as expected
        self.assertEqual(self.config.timesync_module_rt, "ecaltime-linuxptp")
        self.assertEqual(self.config.timesync_module_replay, "replay-module")

    def test_reset_values(self):
        # Reset the values and check if they return to default
        self.config.timesync_module_rt = "new-module"
        self.config.timesync_module_replay = "another-module"
        
        # Resetting attributes
        self.config.timesync_module_rt = "ecaltime-localtime"
        self.config.timesync_module_replay = ""

        # Assert the values have reset to default
        self.assertEqual(self.config.timesync_module_rt, "ecaltime-localtime")
        self.assertEqual(self.config.timesync_module_replay, "")

if __name__ == '__main__':
    unittest.main()
