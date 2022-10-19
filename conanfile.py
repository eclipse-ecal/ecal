from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMakeDeps
import os
import pathlib

class eCALConan(ConanFile):
    name = "ecal"
    settings = "os", "compiler", "arch"
    license = "BSD-3-Clause"
    generators = "virtualenv"
    python_requires = {"cmake-presets-generator/0.1.0@heat-rae/stable", "conancontifile/0.0.15@heat-rae/stable"}
    
    def build_requirements(self):
        self.build_requires("doxygen/[*]@heat-rae/stable")
        self.build_requires("cmake/[*]@heat-rae/stable")
    
    def requirements(self):
        self.requires("termcolor/[*]@heat-rae/stable")
        self.requires("fineftp-server/[*]@heat-rae/stable")
        self.requires("curl/[*]@heat-rae/stable")
        self.requires("hdf5/[*]@heat-rae/stable")
        self.requires("protobuf/[*]@heat-rae/stable")
        self.requires("curl/[*]@heat-rae/stable")
        self.requires("qt/[*]@heat-rae/stable")
        self.requires("spdlog/[*]@heat-rae/stable")
        self.requires("tclap/[*]@heat-rae/stable")
        self.requires("asio/[*]@heat-rae/stable")
        self.requires("gtest/[*]@heat-rae/stable")
        self.requires("tinyxml2/[*]@heat-rae/stable")
        self.requires("openssl/1.1.1i@heat-rae/stable")
        self.requires("tcp-pubsub/[*]@heat-rae/stable")
        self.requires("recycle/[*]@heat-rae/stable")
        if self.settings.os == "Windows":
            self.requires("npcap/[*]@heat-rae/stable")
            # We need to fix this version, sorry!
            self.requires("pcapplusplus/21.05@heat-rae/stable")
        
       
    def generate(self):
        CMakePresets = self.python_requires["cmake-presets-generator"].module.CMakePresets
        cmake_folder = self.recipe_folder
        tc = CMakePresets(self, cmake_folder=".")
        # customize toolchain "tc"
        tc.variables["ECAL_THIRDPARTY_BUILD_CURL"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_SPDLOG"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_GTEST"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_PROTOBUF"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_HDF5"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TINYXML2"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TCP_PUBSUB"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_RECYCLE"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_FINEFTP"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TERMCOLOR"] = "OFF"

        if self.settings.os == "Windows":
            tc.variables["ECAL_NPCAP_SUPPORT"] = "ON"
        tc.variables["BUILD_DOCS"] = "ON"
        tc.variables["BUILD_ECAL_TESTS"] = "ON"
        tc.variables["CMAKE_FIND_PACKAGE_PREFER_CONFIG"] = "ON"
        tc.generate()
        
        
       