import unittest

import ecal.nanobind_core as ecal_core

EMPTY_STRING = ""
EMPTY_BYTES = b""
MY_NAME = "name"
MY_ENCODING = "encoding"
MY_DESCRIPTOR = b"descriptor"

# The constructor may take arguments name (str), encoding (str) and descriptor (bytes)
# This test ensures that all these constructor calls are valid      
class TestDataTypeInformationConstructor(unittest.TestCase):
    def test_constructor_empty(self):
        default_datatype_info = ecal_core.DataTypeInformation()
        self.assertEqual(default_datatype_info.name, EMPTY_STRING)
        self.assertEqual(default_datatype_info.encoding, EMPTY_STRING)
        self.assertEqual(default_datatype_info.descriptor, EMPTY_BYTES)

    def test_constructor_name(self):
        default_datatype_info = ecal_core.DataTypeInformation(name=MY_NAME)
        self.assertEqual(default_datatype_info.name, MY_NAME)
        self.assertEqual(default_datatype_info.encoding, EMPTY_STRING)
        self.assertEqual(default_datatype_info.descriptor, EMPTY_BYTES)

    def test_constructor_encoding(self):
        default_datatype_info = ecal_core.DataTypeInformation(encoding=MY_ENCODING)
        self.assertEqual(default_datatype_info.name, EMPTY_STRING)
        self.assertEqual(default_datatype_info.encoding, MY_ENCODING)
        self.assertEqual(default_datatype_info.descriptor, EMPTY_BYTES)

    def test_constructor_type(self):
        default_datatype_info = ecal_core.DataTypeInformation(descriptor=MY_DESCRIPTOR)
        self.assertEqual(default_datatype_info.name, EMPTY_STRING)
        self.assertEqual(default_datatype_info.encoding, EMPTY_STRING)
        self.assertEqual(default_datatype_info.descriptor, MY_DESCRIPTOR)    

    def test_constructor_full(self):
        default_datatype_info = ecal_core.DataTypeInformation(name = MY_NAME, encoding=MY_ENCODING, descriptor=MY_DESCRIPTOR)
        self.assertEqual(default_datatype_info.name, MY_NAME)
        self.assertEqual(default_datatype_info.encoding, MY_ENCODING)
        self.assertEqual(default_datatype_info.descriptor, MY_DESCRIPTOR)    


if __name__ == '__main__':
    unittest.main()