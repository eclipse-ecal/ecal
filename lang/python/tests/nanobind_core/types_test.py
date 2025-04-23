import ecal.nanobind_core as ecal_core

EMPTY_STRING = ""
EMPTY_BYTES = b""
MY_NAME = "name"
MY_ENCODING = "encoding"
MY_DESCRIPTOR = b"descriptor"

def test_constructor_empty():
    default_datatype_info = ecal_core.DataTypeInformation()
    assert default_datatype_info.name == EMPTY_STRING
    assert default_datatype_info.encoding == EMPTY_STRING
    assert default_datatype_info.descriptor == EMPTY_BYTES

def test_constructor_name():
    default_datatype_info = ecal_core.DataTypeInformation(name=MY_NAME)
    assert default_datatype_info.name == MY_NAME
    assert default_datatype_info.encoding == EMPTY_STRING
    assert default_datatype_info.descriptor == EMPTY_BYTES

def test_constructor_encoding():
    default_datatype_info = ecal_core.DataTypeInformation(encoding=MY_ENCODING)
    assert default_datatype_info.name == EMPTY_STRING
    assert default_datatype_info.encoding == MY_ENCODING
    assert default_datatype_info.descriptor == EMPTY_BYTES

def test_constructor_type():
    default_datatype_info = ecal_core.DataTypeInformation(descriptor=MY_DESCRIPTOR)
    assert default_datatype_info.name == EMPTY_STRING
    assert default_datatype_info.encoding == EMPTY_STRING
    assert default_datatype_info.descriptor == MY_DESCRIPTOR

def test_constructor_full():
    default_datatype_info = ecal_core.DataTypeInformation(
        name=MY_NAME, encoding=MY_ENCODING, descriptor=MY_DESCRIPTOR
    )
    assert default_datatype_info.name == MY_NAME
    assert default_datatype_info.encoding == MY_ENCODING
    assert default_datatype_info.descriptor == MY_DESCRIPTOR