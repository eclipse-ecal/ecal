import unittest
import nanobind_core as ecal
import time
from threading import Event

class AtomicSignalable:
    def __init__(self, initial=0):
        self.value = initial
        self.event = Event()

    def increment(self):
        self.value += 1
        self.event.set()

    def get(self):
        return self.value

    def wait_for(self, condition, timeout):
        start_time = time.time()
        while not condition(self.value):
            if time.time() - start_time > timeout:
                break
            self.event.wait(timeout)
        return condition(self.value)

class TestClientServer(unittest.TestCase):

    def test_client_connect_event(self):
        ecal.initialize()

        client = ecal.ServiceClient("service")

        event_connected_fired = AtomicSignalable(0)
        event_disconnected_fired = AtomicSignalable(0)

        def client_event_callback(name, data):
            if data.type == ecal.eCAL_Client_Event.client_event_connected:
                print("event connected fired")
                event_connected_fired.increment()
            elif data.type == ecal.eCAL_Client_Event.client_event_disconnected:
                print("event disconnected fired")
                event_disconnected_fired.increment()

        client.add_event_callback(ecal.eCAL_Client_Event.client_event_connected, client_event_callback)
        client.add_event_callback(ecal.eCAL_Client_Event.client_event_disconnected, client_event_callback)

        time.sleep(1)
        self.assertEqual(event_connected_fired.get(), 0)
        self.assertEqual(event_disconnected_fired.get(), 0)

        server1 = ecal.ServiceServer("service")

        event_connected_fired.wait_for(lambda v: v >= 1, 5)
        self.assertEqual(event_connected_fired.get(), 1)
        self.assertEqual(event_disconnected_fired.get(), 0)

        server2 = ecal.ServiceServer("service")

        event_connected_fired.wait_for(lambda v: v >= 2, 5)
        self.assertEqual(event_connected_fired.get(), 2)
        self.assertEqual(event_disconnected_fired.get(), 0)

        event_disconnected_fired.wait_for(lambda v: v >= 2, 20)
        self.assertEqual(event_connected_fired.get(), 2)
        self.assertEqual(event_disconnected_fired.get(), 2)

        ecal.finalize()

    def test_server_connect_event(self):
        ecal.initialize()

        server = ecal.ServiceServer("service")

        event_connected_fired = AtomicSignalable(0)
        event_disconnected_fired = AtomicSignalable(0)

        def event_callback(name, data):
            if data.type == ecal.eCAL_Server_Event.server_event_connected:
                print("event connected fired")
                event_connected_fired.increment()
            elif data.type == ecal.eCAL_Server_Event.server_event_disconnected:
                print("event disconnected fired")
                event_disconnected_fired.increment()

        server.add_event_callback(ecal.eCAL_Server_Event.server_event_connected, event_callback)
        server.add_event_callback(ecal.eCAL_Server_Event.server_event_disconnected, event_callback)

        time.sleep(1)
        self.assertEqual(event_connected_fired.get(), 0)
        self.assertEqual(event_disconnected_fired.get(), 0)

        client1 = ecal.ServiceClient("service")

        event_connected_fired.wait_for(lambda v: v >= 1, 5)
        self.assertEqual(event_connected_fired.get(), 1)
        self.assertEqual(event_disconnected_fired.get(), 0)

        client2 = ecal.ServiceClient("service")

        time.sleep(2)
        self.assertEqual(event_connected_fired.get(), 1)
        self.assertEqual(event_disconnected_fired.get(), 0)

        event_disconnected_fired.wait_for(lambda v: v >= 1, 10)
        self.assertEqual(event_connected_fired.get(), 1)
        self.assertEqual(event_disconnected_fired.get(), 1)

        ecal.finalize()

if __name__ == '__main__':
    unittest.main()
