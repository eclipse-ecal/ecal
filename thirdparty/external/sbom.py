import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), "..")) # Add ecal_license_utils to path
import ecal_license_utils

def get_sbom_qt():
    component_name = "Qt"
    license_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/qt")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                        ]
    sbom[component_name]["path"] =                      None
    sbom[component_name]["license"] =                   "LGPL-3.0-only"

    # add all license files from the license directory
    sbom[component_name]["license_files"] = []
    for filename in os.listdir(license_dir):
        if os.path.isfile(os.path.join(license_dir, filename)):
            # add the license file to the list
            sbom[component_name]["license_files"].append(os.path.join(license_dir, filename))

    # add all third-party license files from the license directory
    sbom[component_name]["thirdparty_license_files"] = []
    for filename in os.listdir(os.path.join(license_dir, "thirdparty")):
        if os.path.isfile(os.path.join(license_dir, "thirdparty", filename)):
            sbom[component_name]["thirdparty_license_files"].append(os.path.join(license_dir, "thirdparty", filename))

    sbom[component_name]["copyright"] =                 "The Qt Company Ltd."
    sbom[component_name]["homepage"] =                  "https://www.qt.io/"
    sbom[component_name]["repo_url"] =                  "https://github.com/qt/qtbase"
    sbom[component_name]["git_version"] =               "v6.6.3"
    sbom[component_name]["git_version_url"] =           sbom[component_name]["repo_url"] + "/tree/" + sbom[component_name]["git_version"]

    return sbom

def get_sbom_capnp():
    component_name = "Cap'n Proto"
    license_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/capnproto")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              []
    sbom[component_name]["path"] =                      None
    sbom[component_name]["license"] =                   "MIT"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(license_dir, "LICENSE"),
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 ecal_license_utils.get_copyright_from_file(sbom[component_name]["license_files"][0])
    sbom[component_name]["homepage"] =                  "https://capnproto.org/"
    sbom[component_name]["repo_url"] =                  "https://github.com/capnproto/capnproto"
    sbom[component_name]["git_version"] =               None
    sbom[component_name]["git_version_url"] =           None

    return sbom

def get_sbom_convertutf():
    component_name = "convert_utf"
    license_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/convert_utf")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.COPY_IN_REPO,
                                                        ]
    sbom[component_name]["path"] =                      "contrib/ecaltime/linuxptp/src"
    sbom[component_name]["license"] =                   "convert-utf"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(license_dir, "LICENSE"),
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 ecal_license_utils.get_copyright_from_file(sbom[component_name]["license_files"][0])
    sbom[component_name]["homepage"] =                  None
    sbom[component_name]["repo_url"] =                  None
    sbom[component_name]["git_version"] =               None
    sbom[component_name]["git_version_url"] =           None

    return sbom

def get_sbom_flaticon():
    component_name = "Flaticon"
    license_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/flaticon")
    icon_dir     = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../app/iconset/flaticon")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.COPY_IN_REPO,
                                                            ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                            ecal_license_utils.include_type.LINUX_BINARIES,
                                                            ecal_license_utils.include_type.DOCUMENTATION,
                                                        ]
    sbom[component_name]["path"] =                      icon_dir
    sbom[component_name]["license"] =                   "CC-BY-3.0"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(license_dir, "LICENSE"),
                                                            os.path.join(icon_dir, "license.htm"),
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 "Multiple authors, see license file"
    sbom[component_name]["homepage"] =                  "https://www.flaticon.com/"
    sbom[component_name]["repo_url"] =                  None
    sbom[component_name]["git_version"] =               None
    sbom[component_name]["git_version_url"] =           None

    return sbom

def get_sbom_modpath():
    component_name = "modpath.iss"
    license_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/modpath_iss")
    file_path    = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../../cpack/innosetup/modpath.iss")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.COPY_IN_REPO,
                                                            ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                        ]
    sbom[component_name]["path"] =                      file_path
    sbom[component_name]["license"] =                   "LGPL-3.0-only"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(license_dir, "lgpl-3.0.txt"),
                                                            os.path.join(license_dir, "gpl-3.0.txt"),
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 "Jared Breland <jbreland@legroom.net>"
    sbom[component_name]["homepage"] =                  "http://www.legroom.net/software"
    sbom[component_name]["repo_url"] =                  None
    sbom[component_name]["git_version"] =               None
    sbom[component_name]["git_version_url"] =           None

    return sbom

def get_sbom_nanopb():
    component_name = "nanopb"

    license_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "license_files/nanopb")
    nanopb_dir   = os.path.join(os.path.dirname(os.path.realpath(__file__)), "ecal/ecal/core/src/serialization/")

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.COPY_IN_REPO,
                                                            ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                            ecal_license_utils.include_type.LINUX_BINARIES,
                                                            ecal_license_utils.include_type.PYTHON_BINDINGS,
                                                        ]
    sbom[component_name]["path"] =                      nanopb_dir
    sbom[component_name]["license"] =                   "Zlib"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(license_dir, "LICENSE.txt")
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 ecal_license_utils.get_copyright_from_file(sbom[component_name]["license_files"][0])
    sbom[component_name]["homepage"] =                  "https://jpa.kapsi.fi/nanopb/"
    sbom[component_name]["repo_url"] =                  "https://github.com/nanopb/nanopb"
    sbom[component_name]["git_version"] =               None
    sbom[component_name]["git_version_url"] =           None

    return sbom

def get_sbom():
    sbom = {}
    sbom.update(get_sbom_qt())
    sbom.update(get_sbom_capnp())
    sbom.update(get_sbom_convertutf())
    sbom.update(get_sbom_flaticon())
    sbom.update(get_sbom_modpath())
    sbom.update(get_sbom_nanopb())

    return sbom