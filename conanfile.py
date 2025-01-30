from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMakeDeps
import os
import pathlib

class eCALConan(ConanFile):
    name = "ecal"
    settings = "os", "compiler", "arch", "build_type"
    license = "BSD-3-Clause"
    generators = "virtualenv"
    build_requires = "cmake/3.21.1"
    
    def build_requirements(self):
        self.build_requires("doxygen/1.9.1")
    
    def requirements(self):
        self.requires("hdf5/1.10.6")
        self.requires("protobuf/3.17.1")
        self.requires("libcurl/7.78.0")
        self.requires("qt/5.15.2")
        self.requires("spdlog/1.9.2")
        #self.requires("tclap/1.2.4")
        #self.requires("asio/1.19.2")
        self.requires("gtest/1.11.0")
        self.requires("tinyxml2/8.0.0")
        self.requires("openssl/1.1.1l", override=True)
        
    def configure(self):
        if self.settings.os == "Windows":
            self.options["qt"].shared = True
            self.options["qt"].qtwinextras = True
        
    def generate(self):
        # Don't generate the config files for doxygen. They don't work properlyexit
        self.deps_cpp_info["doxygen"].set_property("cmake_find_mode", "none")
        cmake = CMakeDeps(self)
        cmake.generate()
        
        tc = CMakeToolchain(self)
        # customize toolchain "tc"
        tc.variables["ECAL_THIRDPARTY_BUILD_CURL"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_SPDLOG"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_GTEST"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_PROTOBUF"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_HDF5"] = "OFF"
        tc.variables["ECAL_THIRDPARTY_BUILD_TINYXML2"] = "OFF"
        tc.variables["ECAL_BUILD_DOCS"] = "ON"
        tc.variables["CMAKE_FIND_PACKAGE_PREFER_CONFIG"] = "ON"
        if self.settings.os == "Windows":
            tc.variables["Protobuf_PROTOC_EXECUTABLE"] = os.path.join(self.deps_cpp_info["protobuf"].rootpath, "bin", "protoc.exe").replace('\\', '/')
        else:
            tc.variables["Protobuf_PROTOC_EXECUTABLE"] = os.path.join(self.deps_cpp_info["protobuf"].rootpath, "bin", "protoc")
        tc.generate()
        
        
       