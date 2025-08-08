import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")) # Add ecal_license_utils to path
import ecal_license_utils

def _get_sbom_udpcap():
    component_name = "Udpcap"
    component_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "udpcap")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =      [
                                                    ecal_license_utils.include_type.SUBMODULE,
                                                    ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                    ecal_license_utils.include_type.PYTHON_BINDINGS,
                                                ]
    sbom[component_name]["path"] =              component_dir
    sbom[component_name]["license"] =           "Apache-2.0"
    sbom[component_name]["license_files"] =     [
                                                    os.path.join(sbom[component_name]["path"], "LICENSE"),
                                                ]
    sbom[component_name]["copyright"] =         ecal_license_utils.get_copyright_from_file(sbom[component_name]["license_files"][0], skip_lines=188)
    sbom[component_name]["homepage"] =          None
    sbom[component_name]["repo_url"] =          ecal_license_utils.get_repo_url_from_submodule(component_dir)
    sbom[component_name]["git_version"] =       ecal_license_utils.get_git_version_from_submodule(component_dir)
    sbom[component_name]["git_version_url"] =   sbom[component_name]["repo_url"] + "/tree/" + sbom[component_name]["git_version"]

    return sbom

def _get_sbom_npcap():
    component_name = "Npcap"
    license_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/npcap")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =      []
    sbom[component_name]["special_include_text"] = "Not included due to licensing issues, but can be used as plugin. Please check out the npcap chapter. eCAL for Windows links against the npcap SDK, which is fetched by CMake."
    sbom[component_name]["path"] =              None
    sbom[component_name]["license"] =           "npcap"
    sbom[component_name]["license_files"] =     [
                                                    os.path.join(license_dir, "LICENSE"),
                                                ]
    sbom[component_name]["copyright"] =         'copyright (c) 2013-2025 by Nmap Software LLC ("The Nmap Project").  All rights reserved.'
    sbom[component_name]["homepage"] =          "https://npcap.com/"
    sbom[component_name]["repo_url"] =          "https://github.com/nmap/npcap"
    sbom[component_name]["git_version"] =       "v1.81"
    sbom[component_name]["git_version_url"] =   sbom[component_name]["repo_url"] + "/tree/" + sbom[component_name]["git_version"]

    return sbom

def _get_sbom_pcappp():
    component_name = "PcapPlusPlus"

    license_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/pcappp")
    make_available_cmake_file_path = os.path.join(os.path.dirname(os.path.realpath(__file__)), "udpcap/thirdparty/pcapplusplus/pcapplusplus_make_available.cmake")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =      [
                                                    ecal_license_utils.include_type.FETCHED_BY_CMAKE,
                                                    ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                    ecal_license_utils.include_type.PYTHON_BINDINGS,
                                                ]
    sbom[component_name]["path"] =              None
    sbom[component_name]["license"] =           "Unlicense"
    sbom[component_name]["license_files"] =     [
                                                    os.path.join(license_dir, "LICENSE"),
                                                ]
    sbom[component_name]["copyright"] =         'Copyright © 2025 seladb'
    sbom[component_name]["homepage"] =          "https://pcapplusplus.github.io/"

    # Read the make_available_cmake_file to get the repo URL and git tag (GIT_REPOSITORY and GIT_TAG)
    with open(make_available_cmake_file_path, 'r') as file:
        for line in file:
            line = line.strip()
            if line.startswith("GIT_REPOSITORY"):
                sbom[component_name]["repo_url"] = line.split(' ')[1]
            elif line.startswith("GIT_TAG"):
                sbom[component_name]["git_version"] = line.split(' ')[1]

    sbom[component_name]["git_version_url"] =   sbom[component_name]["repo_url"] + "/tree/" + sbom[component_name]["git_version"]

    return sbom

def get_sbom():
    sbom = {}
    sbom.update(_get_sbom_udpcap())
    sbom.update(_get_sbom_npcap())
    sbom.update(_get_sbom_pcappp())
    
    return sbom