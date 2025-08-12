import os
import sys
import shutil

thirdparty_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "thirdparty")

# Add thirdparty directory to the system path
sys.path.insert(0, thirdparty_dir)

import ecal_license_utils

# Iterate over subdirs in thirdparty directory to import their get_sbom modules
sbom_module_list = []
for subdir in os.listdir(thirdparty_dir):
    subdir_path = os.path.join(thirdparty_dir, subdir)
    if os.path.isdir(subdir_path):
        module_name = f"{subdir}.sbom"
        try:
            sbom_module_list.append(__import__(module_name, fromlist=['get_sbom']))
        except ImportError as e:
            print(f"Failed to import {module_name}: {e}")

def copy_license_files_to(sbom_dict, target_dir):
    """
    Copies license files from the SBOM dictionary to the specified target directory. For each sbom entry, a subdirectory is created in the target directory, and the license files are copied there.
    """
    for component_name, component_info in sbom_dict.items():
        if "license_files" in component_info:
            # replace all characters that are not alphanumeric or underscore with an underscore
            component_license_dir = ''.join(c if c.isalnum() or c == '_' else '_' for c in component_name)
            # Create a subdirectory for the component in the target directory
            target_component_dir = os.path.join(target_dir, component_license_dir)
            os.makedirs(target_component_dir, exist_ok=True)
            for license_file in component_info["license_files"]:
                if os.path.exists(license_file):
                    target_file_path = os.path.join(target_component_dir, os.path.basename(license_file))
                    with open(license_file, 'rb') as src_file:
                        with open(target_file_path, 'wb') as dst_file:
                            dst_file.write(src_file.read())
                else:
                    print(f"License file {license_file} does not exist and cannot be copied.")

            # Copy thirdparty licenses if available
            if component_info["thirdparty_license_files"]:
                thirdparty_license_dir = os.path.join(target_component_dir, "thirdparty")
                os.makedirs(thirdparty_license_dir, exist_ok=True)
                for thirdparty_license in component_info["thirdparty_license_files"]:
                    if os.path.exists(thirdparty_license):
                        target_file_path = os.path.join(thirdparty_license_dir, os.path.basename(thirdparty_license))
                        with open(thirdparty_license, 'rb') as src_file:
                            with open(target_file_path, 'wb') as dst_file:
                                dst_file.write(src_file.read())
                    else:
                        print(f"Third-party license file {thirdparty_license} does not exist and cannot be copied.")

if __name__ == "__main__":
    # Create a dictionary to hold License information from all modules
    sbom_dict = {}
    for sbom_module in sbom_module_list:
        # Apend the SBOM from each module to the dictionary
        sbom = sbom_module.get_sbom()
        sbom_dict.update(sbom)

    licenses_target_dir        = os.path.realpath(os.path.join(os.path.dirname(__file__), "../licenses"))
    python_licenses_target_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), "../licenses_python_bindings"))

    # Make sure that the target directories exist but is empty
    os.makedirs(licenses_target_dir, exist_ok=True)
    os.makedirs(python_licenses_target_dir, exist_ok=True)

    # Clear the contents of the target directories
    for target_dir in [licenses_target_dir, python_licenses_target_dir]:
        for item in os.listdir(target_dir):
            item_path = os.path.join(target_dir, item)
            if os.path.isdir(item_path):
                shutil.rmtree(item_path)
            else:
                os.remove(item_path)

    # Copy license files from the SBOM dictionary to the target directories
    copy_license_files_to(sbom_dict, licenses_target_dir)

    # Filter the SBOM dictionary for Python bindings only
    sbom_dict_python_only = dict(filter(lambda x: ecal_license_utils.include_type.PYTHON_BINDINGS in x[1]["include_type"], sbom_dict.items()))
    copy_license_files_to(sbom_dict_python_only, python_licenses_target_dir)