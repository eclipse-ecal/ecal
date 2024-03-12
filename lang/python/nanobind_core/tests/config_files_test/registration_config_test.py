import unittest
import nanobind_core as ecal

class TestRegistrationConfiguration(unittest.TestCase):
    
    def setUp(self):
        """Set up the test environment before each test."""
        self.shm_config = ecal.Registration.Layer.SHM.SHMConfiguration()
        self.udp_config = ecal.Registration.Layer.UDP.UDPConfiguration()
        self.layer_config = ecal.Registration.Layer.LayerConfiguration()
        self.registration_config = ecal.Registration.RegistrationConfiguration()

    def test_shm_configuration_default(self):
        """Test default values for SHMConfiguration."""
        self.assertFalse(self.shm_config.enable)
        self.assertEqual(self.shm_config.domain, "ecal_mon")
        self.assertEqual(self.shm_config.queue_size, 1024)

    def test_udp_configuration_default(self):
        """Test default values for UDPConfiguration."""
        self.assertTrue(self.udp_config.enable)
        self.assertEqual(self.udp_config.port, 14000)

    def test_layer_configuration_default(self):
        """Test default values for LayerConfiguration."""
        self.assertFalse(self.layer_config.shm.enable)
        self.assertEqual(self.layer_config.shm.domain, "ecal_mon")
        self.assertEqual(self.layer_config.shm.queue_size, 1024)
        self.assertTrue(self.layer_config.udp.enable)
        self.assertEqual(self.layer_config.udp.port, 14000)

    def test_registration_configuration_default(self):
        """Test default values for RegistrationConfiguration."""
        self.assertEqual(self.registration_config.registration_timeout, 10000)
        self.assertEqual(self.registration_config.registration_refresh, 1000)
        self.assertFalse(self.registration_config.network_enabled)
        self.assertTrue(self.registration_config.loopback)
        self.assertEqual(self.registration_config.host_group_name, "")
        self.assertIsInstance(self.registration_config.layer,  ecal.Registration.Layer.LayerConfiguration)

    def test_set_shm_configuration(self):
        """Test setting values in SHMConfiguration."""
        self.shm_config.enable = True
        self.shm_config.domain = "new_domain"
        self.shm_config.queue_size = 2048

        self.assertTrue(self.shm_config.enable)
        self.assertEqual(self.shm_config.domain, "new_domain")
        self.assertEqual(self.shm_config.queue_size, 2048)

    def test_set_udp_configuration(self):
        """Test setting values in UDPConfiguration."""
        self.udp_config.enable = False
        self.udp_config.port = 15000

        self.assertFalse(self.udp_config.enable)
        self.assertEqual(self.udp_config.port, 15000)

    def test_set_registration_configuration(self):
        """Test setting values in RegistrationConfiguration."""
        self.registration_config.registration_timeout = 20000
        self.registration_config.registration_refresh = 2000
        self.registration_config.network_enabled = True
        self.registration_config.loopback = False
        self.registration_config.host_group_name = "custom_host_group"

        self.assertEqual(self.registration_config.registration_timeout, 20000)
        self.assertEqual(self.registration_config.registration_refresh, 2000)
        self.assertTrue(self.registration_config.network_enabled)
        self.assertFalse(self.registration_config.loopback)
        self.assertEqual(self.registration_config.host_group_name, "custom_host_group")

if __name__ == "__main__":
    unittest.main()
