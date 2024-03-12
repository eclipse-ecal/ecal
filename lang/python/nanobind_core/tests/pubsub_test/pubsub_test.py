import unittest
from unittest.mock import patch, call
import nanobind_core as ecal_core
import time

PAYLOAD_SIZE = 25
CMN_REGISTRATION_REFRESH = 1000
DATA_FLOW_TIME = 500

# Global variable to store the callback received bytes
g_callback_received_bytes = 0

def create_payload(size):
    s = "Hello World "
    while len(s) < size:
        s += s
    s = s[:size]
    return s

def on_receive(topic_name, msg, time):
    global g_callback_received_bytes
    g_callback_received_bytes = len(msg)

class TestECALPubSub(unittest.TestCase):
    def setUp(self):
        ecal_core.initialize()
        ecal_core.enable_loopback(True)
        self.pub = ecal_core.Publisher("foo")
        self.sub = ecal_core.Subscriber("foo")
        self.send_s = create_payload(PAYLOAD_SIZE)
        self.recv_s = ""
    
    def tearDown(self):
        self.pub.destroy()
        self.sub.destroy()
        ecal_core.finalize(0)

    def test_simple_message1(self):
        # Let's match them
        time.sleep(2 * CMN_REGISTRATION_REFRESH / 1000)  # Convert milliseconds to seconds

        # Send content
        self.assertEqual(len(self.send_s), self.pub.send(self.send_s,0))

        # Receive content with DATA_FLOW_TIME ms timeout
        self.recv_s = self.sub.receive(50)
        self.assertEqual(len(self.send_s), len(self.recv_s))

        # Receive content with DATA_FLOW_TIME ms timeout
        # should return because no new publishing
        self.recv_s = self.sub.receive(50)
        self.assertEqual(0, len(self.recv_s))

    def test_simple_message2(self):
        # Let's match them
        time.sleep(2 * CMN_REGISTRATION_REFRESH / 1000)  # Convert milliseconds to seconds

        # Send content
        self.assertEqual(len(self.send_s), self.pub.send(self.send_s,0))

        # Receive content with DATA_FLOW_TIME ms timeout
        self.recv_s = self.sub.receive(50)
        self.assertEqual(len(self.send_s), len(self.recv_s))

    def test_simple_message_cb(self):
        global g_callback_received_bytes

        # Add callback
        self.sub.add_receive_callback(on_receive)

        # Let them match
        time.sleep(2 * CMN_REGISTRATION_REFRESH / 1000)

        # Send content
        g_callback_received_bytes = 0
        self.assertEqual(len(self.send_s), self.pub.send(self.send_s, 0))

        # Let the data flow
        time.sleep(DATA_FLOW_TIME / 1000)

        # Check callback receive
        self.assertEqual(len(self.send_s), g_callback_received_bytes)

        # Remove receive callback
        self.sub.rem_receive_callback()

        # Send content
        g_callback_received_bytes = 0
        self.assertEqual(len(self.send_s), self.pub.send(self.send_s, 0))

        # Let the data flow
        time.sleep(DATA_FLOW_TIME / 1000)

        # Check callback receive
        self.assertEqual(0, g_callback_received_bytes)

        # Add callback again
        self.sub.add_receive_callback(on_receive)

        # Send content
        g_callback_received_bytes = 0
        self.assertEqual(len(self.send_s), self.pub.send(self.send_s, 0))

        # Let the data flow
        time.sleep(DATA_FLOW_TIME / 1000)

        # Check callback receive
        self.assertEqual(len(self.send_s), g_callback_received_bytes)

        self.sub.destroy()
        
        # Send content
        g_callback_received_bytes = 0
        self.assertEqual(len(self.send_s), self.pub.send(self.send_s, 0))

        # Let the data flow
        time.sleep(DATA_FLOW_TIME / 1000)

        # Check callback receive
        self.assertEqual(0, g_callback_received_bytes)

if __name__ == "__main__":
    unittest.main()
