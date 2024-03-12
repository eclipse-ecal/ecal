import unittest
import nanobind_core as ecal

class TestSubscriberConfiguration(unittest.TestCase):

    def setUp(self):
        """Initialize configurations for each test."""
        self.shm_config = ecal.Subscriber.Layer.SHM_Configuration()
        self.udp_config = ecal.Subscriber.Layer.UDP_Configuration()
        self.tcp_config = ecal.Subscriber.Layer.TCP_Configuration()
        self.layer_config = ecal.Subscriber.Layer.LayerConfiguration()
        self.subscriber_config = ecal.Subscriber.SubscriberConfiguration()

    def test_shm_configuration_default(self):
        """Test default value for SHM layer configuration."""
        self.assertTrue(self.shm_config.enable)

    def test_udp_configuration_default(self):
        """Test default value for UDP layer configuration."""
        self.assertTrue(self.udp_config.enable)

    def test_tcp_configuration_default(self):
        """Test default value for TCP layer configuration."""
        self.assertFalse(self.tcp_config.enable)

    def test_layer_configuration_default(self):
        """Test default values for LayerConfiguration."""
        self.assertTrue(self.layer_config.shm.enable)
        self.assertTrue(self.layer_config.udp.enable)
        self.assertFalse(self.layer_config.tcp.enable)

    def test_subscriber_configuration_default(self):
        """Test default values for SubscriberConfiguration."""
        self.assertTrue(self.subscriber_config.drop_out_of_order_messages)
        self.assertIsInstance(self.subscriber_config.layer, ecal.Subscriber.Layer.LayerConfiguration)

    def test_set_shm_configuration(self):
        """Test setting a new value in SHMConfiguration."""
        self.shm_config.enable = False
        self.assertFalse(self.shm_config.enable)

    def test_set_udp_configuration(self):
        """Test setting a new value in UDPConfiguration."""
        self.udp_config.enable = False
        self.assertFalse(self.udp_config.enable)

    def test_set_tcp_configuration(self):
        """Test setting a new value in TCPConfiguration."""
        self.tcp_config.enable = True
        self.assertTrue(self.tcp_config.enable)

    def test_set_layer_configuration(self):
        """Test setting values in LayerConfiguration."""
        self.layer_config.shm.enable = False
        self.layer_config.udp.enable = False
        self.layer_config.tcp.enable = True

        self.assertFalse(self.layer_config.shm.enable)
        self.assertFalse(self.layer_config.udp.enable)
        self.assertTrue(self.layer_config.tcp.enable)

    def test_set_subscriber_configuration(self):
        """Test setting values in SubscriberConfiguration."""
        self.subscriber_config.drop_out_of_order_messages = False
        self.subscriber_config.layer.shm.enable = False
        self.subscriber_config.layer.udp.enable = False
        self.subscriber_config.layer.tcp.enable = True

        self.assertFalse(self.subscriber_config.drop_out_of_order_messages)
        self.assertFalse(self.subscriber_config.layer.shm.enable)
        self.assertFalse(self.subscriber_config.layer.udp.enable)
        self.assertTrue(self.subscriber_config.layer.tcp.enable)

if __name__ == "__main__":
    unittest.main()
