# This is a qick-and-dirty script to grab external license files from third-party components that are not available from submodules.

import os
import sys

thirdparty_dir = os.path.join(os.path.dirname(os.path.realpath(__file__)), "..", "thirdparty")
external_licenses_dir = os.path.join(thirdparty_dir, "external/license_files")

def copy_qt_license_files(qt_src_dir):
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
                target_file_path = os.path.join(external_licenses_dir, "qt", f"{component}_{file}")

                with open(src_file_path, 'rb') as src_file:
                    with open(target_file_path, 'wb') as dst_file:
                        dst_file.write(src_file.read())
                        print(f"Copied {src_file_path} to {target_file_path}")

if __name__ == "__main__":
    copy_qt_license_files("C:/Qt/6.6.3/Src")