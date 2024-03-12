import unittest
import ctypes
import nanobind_core as eCAL

ECAL_VERSION = "v6.0.0-nightly-52-g063aa0199-dirty"  # Define these constants as per your eCAL version
ECAL_DATE = "24.05.2024"
ECAL_VERSION_MAJOR = 6  # Define these constants as per your eCAL version
ECAL_VERSION_MINOR = 0
ECAL_VERSION_PATCH = 0

class TestCoreCppCore(unittest.TestCase):
    def test_get_version(self):
        # Get eCAL version string
        self.assertEqual(ECAL_VERSION, eCAL.get_version_string())

        # Get eCAL version date
        self.assertEqual(ECAL_DATE, eCAL.get_version_date())

        # Get eCAL version as separated integer values
        status, major, minor, patch = eCAL.get_version()
        self.assertEqual(0, status)
 
        self.assertEqual(ECAL_VERSION_MAJOR, major)
        self.assertEqual(ECAL_VERSION_MINOR, minor)
        self.assertEqual(ECAL_VERSION_PATCH, patch)

    def test_initialize_finalize(self):
        # Is eCAL API initialized?
        self.assertEqual(0, eCAL.is_initialized())

        # Initialize eCAL API
        self.assertEqual(0, eCAL.initialize())

        # Is eCAL API initialized?
        self.assertEqual(1, eCAL.is_initialized())

        # Initialize eCAL API again, we expect return value 1 for yet initialized
        self.assertEqual(1, eCAL.initialize())

        # Finalize eCAL API, we expect return value 0 even if it will not be really finalized because it's 2 times initialized and 1 time finalized
        self.assertEqual(0, eCAL.finalize(0))

        # Is eCAL API initialized? Yes, it's still initialized
        self.assertEqual(1, eCAL.is_initialized())

        # Finalize eCAL API, we expect return value 0 because now it will be finalized
        self.assertEqual(0, eCAL.finalize(0))

        # Is eCAL API initialized? No
        self.assertEqual(0, eCAL.is_initialized())

        # Finalize eCAL API, we expect return value 1 because it was finalized before
        self.assertEqual(1, eCAL.finalize(0))

    def test_multiple_initialize_finalize(self):
        # Try to initialize/finalize multiple times
        for _ in range(4):
            # Initialize eCAL API
            self.assertEqual(0, eCAL.initialize())

            # Finalize eCAL API
            self.assertEqual(0, eCAL.finalize(0))

    def test_set_get_unit_name(self):
        # Initialize eCAL API with empty unit name (eCAL will use process name as unit name)
        self.assertEqual(0, eCAL.initialize())

        # Is eCAL API initialized?
        self.assertEqual(1, eCAL.is_initialized())

        # Set unit name
        self.assertEqual(0, eCAL.set_unitname("unit name"))

        # Set None unit name
        with self.assertRaises(TypeError):
            eCAL.set_unitname(None)

        # Set empty unit name
        self.assertEqual(-1, eCAL.set_unitname(""))

        # Finalize eCAL API, we expect return value 0 because it will be finalized
        self.assertEqual(0, eCAL.finalize(0))

    def test_ecal_ok(self):
        # Check uninitialized eCAL, should not be okay
        self.assertEqual(0, eCAL.ok())

        # Initialize eCAL API
        self.assertEqual(0, eCAL.initialize())

        # Check initialized eCAL, should be okay
        self.assertEqual(1, eCAL.ok())

        # Finalize eCAL API, we expect return value 0 because it will be finalized
        self.assertEqual(0, eCAL.finalize(0))

        # Check finalized eCAL, should not be okay
        self.assertEqual(0, eCAL.ok())

if __name__ == '__main__':
    unittest.main()
