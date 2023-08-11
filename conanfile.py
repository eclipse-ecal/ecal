from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMakeDeps
import os
import pathlib

class eCALConan(ConanFile):
    name = "ecal"
    settings = "os", "compiler", "arch"
    license = "BSD-3-Clause"
    generators = "virtualenv", "virtualrunenv"
    python_requires = {"cmake-presets-generator/0.1.0@ild/stable", "conancontifile/0.0.19@ild/stable"}
    
    def build_requirements(self):
        self.build_requires("doxygen/[*]@ild/stable")
        self.build_requires("cmake/[*]@ild/stable")
        self.build_requires("cmake/[*]@ild/stable")
    
    def requirements(self):
        #self.requires("asio/[*]@ild/stable"")
        #self.requires("tclap/[*]@ild/stable"")
        self.requires("capnproto/[*]@ild/stable")  
        self.requires("cmakefunctions/[*]@ild/stable")  
        self.requires("curl/[*]@ild/stable")
        self.requires("fineftp-server/[*]@ild/stable")
        self.requires("flatbuffers/[*]@ild/stable")
        self.requires("ftxui/[*]@ild/stable")
        self.requires("gtest/[*]@ild/stable")
        self.requires("hdf5/[*]@ild/stable")
        self.requires("openssl/1.1.1i@ild/stable")
        self.requires("protobuf/3.23.4@ild/stable")
        self.requires("qt/[*]@ild/stable")
        self.requires("qwt/[*]@ild/stable")
        self.requires("recycle/[*]@ild/stable")  
        self.requires("simpleini/[*]@ild/stable")
        self.requires("spdlog/[*]@ild/stable")
        self.requires("tcp-pubsub/[*]@ild/stable")
        self.requires("termcolor/[*]@ild/stable")        
        self.requires("tinyxml2/[*]@ild/stable")
        self.requires("udpcap/[*]@ild/stable")
        self.requires("yaml-cpp/[*]@ild/stable")  
        
    def generate(self):
        CMakePresets = self.python_requires["cmake-presets-generator"].module.CMakePresets
        cmake_folder = self.recipe_folder
        tc = CMakePresets(self, cmake_folder=".")
        # customize toolchain "tc"
        tc.variables["BUILD_DOCS"] = "ON"
        tc.variables["BUILD_ECAL_TESTS"] = "ON"
        tc.variables["BUILD_PY_BINDING"] = "ON"
        tc.variables["CMAKE_FIND_PACKAGE_PREFER_CONFIG"] = "ON"          
        tc.variables["HAS_CAPNPROTO"] = "ON"        
        tc.variables["HAS_FLATBUFFERS"] = "ON"
        tc.variables["ECAL_INCLUDE_PY_SAMPLES"] = "ON"
        tc.variables["ECAL_THIRDPARTY_BUILD_CMAKE_FUNCTIONS"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_CURL"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_FINEFTP"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_FTXUI"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_GTEST"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_HDF5"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_PROTOBUF"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_QWT"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_RECYCLE"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_RECYCLE"] = "OFF"        
        tc.variables["ECAL_THIRDPARTY_BUILD_SPDLOG"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TCP_PUBSUB"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TERMCOLOR"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TINYXML2"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_UDPCAP"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_YAML-CPP"] = "OFF"
        tc.generate()
        
        
       