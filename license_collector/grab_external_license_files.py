# This is a qick-and-dirty script to grab external license files from
# third-party components that are not available from submodules.
#
# For running this script, you may need to adapt some paths.

import os
import sys

thirdparty_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "thirdparty")

# Add thirdparty directory to the system path
sys.path.insert(0, thirdparty_dir)

import ecal_license_utils

external_licenses_dir = os.path.join(thirdparty_dir, "external/license_files")

def copy_qt_license_files(qt_src_dir):
    license_target_dir = os.path.join(external_licenses_dir, "qt")
    if not os.path.exists(license_target_dir):
        os.makedirs(license_target_dir)

    # Copy everything from the LICENSES directory to the target directory
    qt_licenses_dir = os.path.join(qt_src_dir, "LICENSES")
    for root, dirs, files in os.walk(qt_licenses_dir):
        for file in files:
            src_file_path = os.path.join(root, file)
            target_file_path = os.path.join(license_target_dir, file)
            target_file_path = os.path.normpath(target_file_path)

            with open(src_file_path, 'rb') as src_file:
                with open(target_file_path, 'wb') as dst_file:
                    dst_file.write(src_file.read())
                    print(f"Copied {src_file_path} to {target_file_path}")
    
    # Copy licenses of specific Qt components

    thirdparty_license_target_dir = os.path.join(external_licenses_dir, "qt/thirdparty")
    if not os.path.exists(thirdparty_license_target_dir):
        os.makedirs(thirdparty_license_target_dir)

    qt_components = [
        "qtbase",
        "qtsvg",
        "qttranslations"
    ]

    for component in qt_components:
        component_licenses_dir = os.path.join(qt_src_dir, component, "LICENSES")

        for root, dirs, files in os.walk(component_licenses_dir):
            for file in files:
                # Copy each license file to the external licenses directory, but prefix with the component name
                src_file_path = os.path.join(root, file)
                target_file_path = os.path.join(thirdparty_license_target_dir, f"{component}_{file}")
                target_file_path = os.path.normpath(target_file_path)

                with open(src_file_path, 'rb') as src_file:
                    with open(target_file_path, 'wb') as dst_file:
                        dst_file.write(src_file.read())
                        print(f"Copied {src_file_path} to {target_file_path}")

def copy_pcapplusplus_license_files(pcap_plus_plus_source_dir):
    pcapplusplus_thirdparty_dir     = os.path.join(pcap_plus_plus_source_dir, "3rdParty")
    target_license_dir              = os.path.join(thirdparty_dir, "udpcap/license_files/pcappp")
    target_thirdparty_license_dir   = os.path.join(target_license_dir, "thirdparty")

    # Copy the main LICENSE file
    pcapplusplus_license_file_path = os.path.join(pcap_plus_plus_source_dir, "LICENSE")
    target_pcapplusplus_license_path = os.path.join(target_license_dir, "LICENSE")
    with open(pcapplusplus_license_file_path, 'rb') as src_file:
        with open(target_pcapplusplus_license_path, 'wb') as dst_file:
            dst_file.write(src_file.read())
            print(f"Copied PcapPlusPlus license to {target_pcapplusplus_license_path}")

    handled_thirdparty_components = []

    # Extract EndianPortable license and write to file
    endian_portable_license = ecal_license_utils.extract_license_from_cpp_source_file(os.path.join(pcapplusplus_thirdparty_dir, "EndianPortable/include/EndianPortable.h"))
    endian_portable_target_file_path = os.path.join(target_thirdparty_license_dir, "EndianPortable_license.txt")
    if not endian_portable_license:
        print("ERROR: Failed to extract EndianPortable license.")
    else:
        with open(endian_portable_target_file_path, 'w', encoding='utf-8') as license_file:
            license_file.write(endian_portable_license)
            print(f"Copied EndianPortable license to {endian_portable_target_file_path}")
    handled_thirdparty_components.append("EndianPortable")

    # Extract Getopt-for-Visual-Studio license and write to file
    getopt_license = ecal_license_utils.extract_license_from_cpp_source_file(os.path.join(pcapplusplus_thirdparty_dir, "Getopt-for-Visual-Studio/getopt.h"))
    getopt_target_file_path = os.path.join(target_thirdparty_license_dir, "Getopt-for-Visual-Studio_license.txt")
    if not getopt_license:
        print("ERROR: Failed to extract Getopt-for-Visual-Studio license.")
    else:
        with open(getopt_target_file_path, 'w', encoding='utf-8') as license_file:
            license_file.write(getopt_license)
            print(f"Copied Getopt-for-Visual-Studio license to {getopt_target_file_path}")
    handled_thirdparty_components.append("Getopt-for-Visual-Studio")

    # Copy hash-library LICENSE file
    hash_lib_license_file_path = os.path.join(pcapplusplus_thirdparty_dir, "hash-library/LICENSE")
    target_hash_lib_license_path = os.path.join(target_thirdparty_license_dir, "hash-library_LICENSE")
    with open(hash_lib_license_file_path, 'rb') as src_file:
        with open(target_hash_lib_license_path, 'wb') as dst_file:
            dst_file.write(src_file.read())
            print(f"Copied hash-library license to {target_hash_lib_license_path}")
    handled_thirdparty_components.append("hash-library")

    # Extract json license and write to file
    json_license = ecal_license_utils.extract_license_from_cpp_source_file(os.path.join(pcapplusplus_thirdparty_dir, "json/include/json.hpp"))
    json_target_file_path = os.path.join(target_thirdparty_license_dir, "json_license.txt")
    if not json_license:
        print("ERROR: Failed to extract json license.")
    else:
        with open(json_target_file_path, 'w', encoding='utf-8') as license_file:
            license_file.write(json_license)
            print(f"Copied json license to {json_target_file_path}")
    handled_thirdparty_components.append("json")

    # Copy LightPcapNg LICENSE file
    light_pcap_ng_license_file_path = os.path.join(pcapplusplus_thirdparty_dir, "LightPcapNg/LightPcapNg/LICENSE.txt")
    light_pcap_ng_target_file_path = os.path.join(target_thirdparty_license_dir, "LightPcapNg_LICENSE.txt")
    with open(light_pcap_ng_license_file_path, 'rb') as src_file:
        with open(light_pcap_ng_target_file_path, 'wb') as dst_file:
            dst_file.write(src_file.read())
            print(f"Copied LightPcapNg license to {light_pcap_ng_target_file_path}")
    handled_thirdparty_components.append("LightPcapNg")

    # Copy MemPlumber LICENSE file
    mem_plumber_license_file_path = os.path.join(pcapplusplus_thirdparty_dir, "MemPlumber/MemPlumber/LICENSE")
    mem_plumber_target_file_path = os.path.join(target_thirdparty_license_dir, "MemPlumber_LICENSE.txt")
    with open(mem_plumber_license_file_path, 'rb') as src_file:
        with open(mem_plumber_target_file_path, 'wb') as dst_file:
            dst_file.write(src_file.read())
            print(f"Copied MemPlumber license to {mem_plumber_target_file_path}")
    handled_thirdparty_components.append("MemPlumber")

    # Ignore OUIDataset
    handled_thirdparty_components.append("OUIDataset")

    # Collect all direct subdirectories of the pcaplusplus thirdparty directory
    # and check if they are handled. Print a warning for any unhandled
    # components.
    for subdir in os.listdir(pcapplusplus_thirdparty_dir):
        subdir_path = os.path.join(pcapplusplus_thirdparty_dir, subdir)
        if os.path.isdir(subdir_path) and subdir not in handled_thirdparty_components:
            print(f"WARNING: Unhandled third-party component found: {subdir}")

if __name__ == "__main__":
    qt_source_dir           = "C:/Qt/6.6.3/Src"  # Adjust this path as necessary
    pcapplusplus_source_dir = os.path.normpath(os.path.join(os.path.dirname(__file__), "../_build/complete/_deps/pcapplusplus-src"))

    copy_qt_license_files(qt_source_dir)
    copy_pcapplusplus_license_files(pcapplusplus_source_dir)