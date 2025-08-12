import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")) # Add ecal_license_utils to path
import ecal_license_utils

def get_sbom():
    component_name = "HDF5"
    component_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "hdf5")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.SUBMODULE,
                                                            ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                            ecal_license_utils.include_type.PYTHON_BINDINGS,
                                                        ]
    sbom[component_name]["path"] =                      component_dir
    sbom[component_name]["license"] =                   "HDF5"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(sbom[component_name]["path"], "COPYING"),
                                                            os.path.join(sbom[component_name]["path"], "COPYING_LBNL_HDF5"),
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 "Copyright 2006 by The HDF Group. Copyright 1998-2006 by The Board of Trustees of the University of Illinois."
    sbom[component_name]["homepage"] =                  "https://www.hdfgroup.org/solutions/hdf5/"
    sbom[component_name]["repo_url"] =                  ecal_license_utils.get_repo_url_from_submodule(component_dir)
    sbom[component_name]["git_version"] =               ecal_license_utils.get_git_version_from_submodule(component_dir)
    sbom[component_name]["git_version_url"] =           sbom[component_name]["repo_url"] + "/tree/" + sbom[component_name]["git_version"]

    return sbom