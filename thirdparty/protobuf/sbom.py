import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")) # Add ecal_license_utils to path
import ecal_license_utils

def get_sbom():
    component_name = "protobuf"
    component_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "protobuf")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =      [
                                                    ecal_license_utils.include_type.SUBMODULE,
                                                    ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                    ecal_license_utils.include_type.PYTHON_BINDINGS,
                                                ]
    sbom[component_name]["path"] =              component_dir
    sbom[component_name]["license"] =           "BSD-3-Clause"
    sbom[component_name]["license_files"] =     [
                                                    os.path.join(sbom[component_name]["path"], "LICENSE"),
                                                ]
    sbom[component_name]["copyright"] =         ecal_license_utils.get_copyright_from_file(sbom[component_name]["license_files"][0])
    sbom[component_name]["homepage"] =          "https://protobuf.dev/"
    sbom[component_name]["repo_url"] =          ecal_license_utils.get_repo_url_from_submodule(component_dir)
    sbom[component_name]["git_version"] =       ecal_license_utils.get_git_version_from_submodule(component_dir)
    sbom[component_name]["git_version_url"] =   sbom[component_name]["repo_url"] + "/tree/" + sbom[component_name]["git_version"]

    return sbom