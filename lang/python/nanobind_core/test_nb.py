import re
import sys
import nanobind_core as t
# import pytest
# from common import skip_on_pypy, collect

import unittest

class NBUnitTest(unittest.TestCase):

    def test01_noparam_functions(self):  
        # print(sys.argv)
        print('Initialise:',t.initialize())
        # Functions without any parameters
        print('===============')
        print('Lamda Functions')
        print('===============')
        print('Version String:',t.get_version_string())
        print('Version Date:',t.get_version_date())
        print('Check OK:',t.ok())
        print('Set Unitname:',t.set_unitname("Unit1"))
        print('Is Initialised:',t.is_initialised())
        # print('Finalize:',t.finalize(123))
        print('User Settings Path:',t.get_user_settings_path())
        print('Log Path:',t.get_log_path())
        print('Config Path:',t.get_config_path())
        print('Active ini File:',t.get_active_ini_file())
        # assert t.shutdown_core() is None
        # assert t.shutdown_processes() is None
        # test suite for subscriber class
        subscriber_obj = t.Subscriber("foo")
        print('=========================')
        print('Subsriber Class Functions')
        print('=========================')
        print('Create Successful:',subscriber_obj.create("trz"))
        print('Subscriber is Created:',subscriber_obj.is_created())
        print('Set Attribute:',subscriber_obj.set_attribute("Attribute1","Attrib_Value"))
        print('Clear Attribute:',subscriber_obj.clear_attribute("Attribute1"))
        print('Clear Attribute Retry:',subscriber_obj.clear_attribute("Attribu"))
#        print('Set ID:',subscriber_obj.set_id(1234))
        print('Topic Name:',subscriber_obj.get_topic_name())
        print('Publisher Count:',subscriber_obj.get_publisher_count())
        print('Datatype Info:',subscriber_obj.get_datatype_information())
        print('Destroy Successful:',subscriber_obj.destroy())
        print('Subscriber is Created:',subscriber_obj.is_created())
         # test suite for publisher class
        print('=========================')
        print('Publisher Class Functions')
        print('=========================')
        publisher_obj = t.Publisher("foo_pub")
        print('Create Successful:',publisher_obj.create("try2"))
        print('Publisher is Created:',publisher_obj.is_created())
        print('Is Subsribed:',publisher_obj.is_subscribed())
        print('Set Attribute:',publisher_obj.set_attribute("Attribute1","Attrib_Value"))
        print('Clear Attribute:',publisher_obj.clear_attribute("Attribute1"))
        print('Clear Attribute Retry:',publisher_obj.clear_attribute("Attribu"))
#        print('Set ID:',publisher_obj.set_id(1234))
        print('Share Type:',publisher_obj.share_type(True))
        print('Topic Name:',publisher_obj.get_topic_name())
        print('Subscriber Count:',publisher_obj.get_subscriber_count())
        print('Datatype Info:',publisher_obj.get_datatype_information())
        print('Destroy Successful:',publisher_obj.destroy())
        print('Subscriber is Created:',publisher_obj.is_created())
        print('Dump Successful:',publisher_obj.dump("ABC"))
        # test suite for Client class
        print('======================')
        print('Client Class Functions')
        print('======================')
        client_obj = t.ServiceClient("client1")
        print('Create Successful:',client_obj.create("ClientTry"))
        print('Set Hostname:',client_obj.set_hostname("ClientHost1"))
        print('Service Name:',client_obj.get_service_name())
        print('Is Connected:',client_obj.is_connected())
        print('Destroy Successful:',client_obj.destroy())
        # test suite for Server class
        print('======================')
        print('Server Class Functions')
        print('======================')
        server_obj = t.ServiceServer("server1")
        print('Create Successful:',server_obj.create("ServerTry"))
        print('Service Name:',server_obj.get_service_name())
        print('Is Connected:',server_obj.is_connected())
        print('Destroy Successful:',server_obj.destroy())       
        # test suite for ClientServer interaction
        print('=========================')
        print('Client-Server Interaction')
        print('=========================')
        print('Call Server:',client_obj.call("Method1","Request1",10000))
 #       print('Call Method Response:',server_obj.add_method_callback("Method1","Request1",10000))
        
        

if __name__ == '__main__':
    unittest.main()
    
# def test02_bool_functions():
#     assert t.enable_loopback(True) is None
#     assert t.pub_share_type(True) is None
#     assert t.pub_share_description(True) is None
#     assert t.enable_loopback(False) is None
#     assert t.pub_share_type(False) is None
#     assert t.pub_share_description(False) is None
    
# def test03_oneparam_functions():
#     assert t.set_unitname("Foo") == 0
    