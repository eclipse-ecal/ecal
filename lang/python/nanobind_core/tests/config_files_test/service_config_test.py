import unittest
import nanobind_core as ecal_module

class TestServiceConfiguration(unittest.TestCase):
    def setUp(self):
        # Create an instance of the ServiceConfiguration class for each test
        self.config = ecal_module.ServiceConfiguration()

    def test_default_values(self):
        # Test that the default values are as expected
        self.assertFalse(self.config.protocol_v0)  # protocol_v0 should be False by default
        self.assertTrue(self.config.protocol_v1)   # protocol_v1 should be True by default

    def test_modify_protocol_v0(self):
        # Test modifying protocol_v0
        self.config.protocol_v0 = True
        self.assertTrue(self.config.protocol_v0)

        # Reset to default
        self.config.protocol_v0 = False
        self.assertFalse(self.config.protocol_v0)

    def test_modify_protocol_v1(self):
        # Test modifying protocol_v1
        self.config.protocol_v1 = False
        self.assertFalse(self.config.protocol_v1)

        # Reset to default
        self.config.protocol_v1 = True
        self.assertTrue(self.config.protocol_v1)

if __name__ == '__main__':
    unittest.main()
