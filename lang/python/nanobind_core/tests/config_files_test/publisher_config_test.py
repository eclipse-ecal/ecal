import unittest
import nanobind_core as ecal

class TestPublisherConfiguration(unittest.TestCase):

    def setUp(self):
        """Set up configurations for each test."""
        self.shm_config = ecal.Publisher.Layer.SHM_Configuration()
        self.udp_config = ecal.Publisher.Layer.UDP_Configuration()
        self.tcp_config = ecal.Publisher.Layer.TCP_Configuration()
        self.layer_config = ecal.Publisher.Layer.LayerConfiguration()
        self.publisher_config = ecal.Publisher.PublisherConfiguration()

    def test_shm_configuration_defaults(self):
        """Test default values for SHM layer configuration."""
        self.assertTrue(self.shm_config.enable)
        self.assertFalse(self.shm_config.zero_copy_mode)
        self.assertEqual(self.shm_config.acknowledge_timeout_ms, 0)
        self.assertEqual(self.shm_config.memfile_buffer_count, 1)
#        self.assertEqual(self.shm_config.memfile_min_size_bytes, 4096)
#        self.assertEqual(self.shm_config.memfile_reserve_percent, 50)

    def test_udp_configuration_defaults(self):
        """Test default values for UDP layer configuration."""
        self.assertTrue(self.udp_config.enable)

    def test_tcp_configuration_defaults(self):
        """Test default values for TCP layer configuration."""
        self.assertTrue(self.tcp_config.enable)

    def test_layer_configuration_defaults(self):
        """Test default values for LayerConfiguration."""
        self.assertTrue(self.layer_config.shm.enable)
        self.assertTrue(self.layer_config.udp.enable)
        self.assertTrue(self.layer_config.tcp.enable)

    def test_publisher_configuration_defaults(self):
        """Test default values for PublisherConfiguration."""
        self.assertTrue(self.publisher_config.share_topic_type)
        self.assertTrue(self.publisher_config.share_topic_description)
#        self.assertIsInstance(self.publisher_config.layer_priority_local, list)
#        self.assertIsInstance(self.publisher_config.layer_priority_remote, list)
#        self.assertEqual(self.publisher_config.layer_priority_local[0], ecal.Publisher.Layer.TLayer.tlayer_shm)
#        self.assertEqual(self.publisher_config.layer_priority_remote[0], ecal.Publisher.Layer.TLayer.tlayer_udp_mc)

    def test_modify_shm_configuration(self):
        """Test modifying SHM configuration values."""
        self.shm_config.enable = False
        self.shm_config.zero_copy_mode = True
        self.shm_config.acknowledge_timeout_ms = 100
        self.shm_config.memfile_buffer_count = 2
#        self.shm_config.memfile_min_size_bytes = 3333
#        self.shm_config.memfile_reserve_percent = 75

        self.assertFalse(self.shm_config.enable)
        self.assertTrue(self.shm_config.zero_copy_mode)
        self.assertEqual(self.shm_config.acknowledge_timeout_ms, 100)
        self.assertEqual(self.shm_config.memfile_buffer_count, 2)
#        self.assertEqual(self.shm_config.memfile_min_size_bytes, 3333)
#        self.assertEqual(self.shm_config.memfile_reserve_percent, 75)

    def test_modify_share_topic_settings(self):
        """Test modifying share topic settings in PublisherConfiguration."""
        self.publisher_config.share_topic_type = False
        self.publisher_config.share_topic_description = False

        self.assertFalse(self.publisher_config.share_topic_type)
        self.assertFalse(self.publisher_config.share_topic_description)

if __name__ == "__main__":
    unittest.main()
