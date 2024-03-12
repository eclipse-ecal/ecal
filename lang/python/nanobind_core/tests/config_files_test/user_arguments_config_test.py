import unittest
import nanobind_core as ecal_module

class TestCliConfiguration(unittest.TestCase):
    def setUp(self):
        # Create an instance of the CliConfiguration class for each test
        self.config = ecal_module.CliConfiguration()

    def test_default_values(self):
        # Check default values for user_yaml and dump_config
        self.assertEqual(self.config.user_yaml, "")       # Default for user_yaml should be an empty string
        self.assertFalse(self.config.dump_config)         # Default for dump_config should be False

    def test_modify_user_yaml(self):
        # Modify and test user_yaml
        self.config.user_yaml = "config.yaml"
        self.assertEqual(self.config.user_yaml, "config.yaml")

        # Reset to default and verify
        self.config.user_yaml = ""
        self.assertEqual(self.config.user_yaml, "")

    def test_modify_dump_config(self):
        # Modify and test dump_config
        self.config.dump_config = True
        self.assertTrue(self.config.dump_config)

        # Reset to default and verify
        self.config.dump_config = False
        self.assertFalse(self.config.dump_config)

if __name__ == '__main__':
    unittest.main()
