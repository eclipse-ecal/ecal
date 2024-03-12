import unittest
import nanobind_core as ecal_module

class TestMonitoringConfiguration(unittest.TestCase):
    def setUp(self):
        # Create an instance of the Configuration class for each test
        self.config = ecal_module.MonitoringConfiguration()

    def test_default_values(self):
        # Test that the default values are as expected
        self.assertEqual(self.config.filter_excl, "^__.*$")
        self.assertEqual(self.config.filter_incl, "")

    def test_modify_values(self):
        # Test modifying the attributes
        self.config.filter_excl = "new_exclusion_pattern"
        self.config.filter_incl = "new_inclusion_pattern"

        # Assert the values have changed as expected
        self.assertEqual(self.config.filter_excl, "new_exclusion_pattern")
        self.assertEqual(self.config.filter_incl, "new_inclusion_pattern")

    def test_reset_values(self):
        # Reset the values and check if they return to default
        self.config.filter_excl = "temporary_pattern"
        self.config.filter_incl = "another_temporary_pattern"
        
        # Resetting attributes
        self.config.filter_excl = "^__.*$"
        self.config.filter_incl = ""

        # Assert the values have reset to default
        self.assertEqual(self.config.filter_excl, "^__.*$")
        self.assertEqual(self.config.filter_incl, "")

if __name__ == '__main__':
    unittest.main()
