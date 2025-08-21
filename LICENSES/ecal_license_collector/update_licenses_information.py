import os
import sys
import shutil
import jinja2

repo_root_dir  = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../..")
thirdparty_dir = os.path.join(repo_root_dir, "thirdparty")

# Add thirdparty directory to the system path
sys.path.insert(0, thirdparty_dir)

import ecal_license_utils
import copy
from collections import OrderedDict

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

def escape_component_name(component_name, lowercase = False):
    escaped_name = ''.join(c if c.isalnum() or c == '_' else '_' for c in component_name)

    if lowercase:
        escaped_name = escaped_name.lower()

    return escaped_name
    

def copy_license_files_to(sbom_dict, target_dir, lowercase_dirs = False, change_to_relative_path = False):
    """
    Copies license files from the SBOM dictionary to the specified target directory. For each sbom entry, a subdirectory is created in the target directory, and the license files are copied there.
    """
    for component_name, component_info in sbom_dict.items():
        if "license_files" in component_info:
            component_license_dir = escape_component_name(component_name, lowercase_dirs)

            # Create a list of relative license files, that we may have to fill
            relative_license_files            = []
            relative_thirdparty_license_files = []
            
            # Create a subdirectory for the component in the target directory
            target_component_dir = os.path.join(target_dir, component_license_dir)
            os.makedirs(target_component_dir, exist_ok=True)

            # Copy (main) License files
            for license_file in component_info["license_files"]:
                if os.path.exists(license_file):
                    target_file_path = os.path.join(target_component_dir, os.path.basename(license_file))
                    with open(license_file, 'rb') as src_file:
                        with open(target_file_path, 'wb') as dst_file:
                            dst_file.write(src_file.read())

                    if change_to_relative_path:
                        # Calculate new relative path
                        rel_path = os.path.relpath(target_file_path, target_dir)
                        rel_path = rel_path.replace(os.sep, "/")
                        relative_license_files.append(rel_path)

                else:
                    print(f"License file {license_file} does not exist and cannot be copied.")

            if change_to_relative_path:
                # Change original dict with the new relative paths
                component_info["license_files"] = relative_license_files

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

                        if change_to_relative_path:
                            # Calculate new relative path
                            rel_path = os.path.relpath(target_file_path, target_dir)
                            rel_path = rel_path.replace(os.sep, "/")
                            relative_thirdparty_license_files.append(rel_path)

                    else:
                        print(f"Third-party license file {thirdparty_license} does not exist and cannot be copied.")

                if change_to_relative_path:
                    # Change original dict with the new relative paths
                    component_info["thirdparty_license_files"] = relative_thirdparty_license_files

def get_ecal_sbom():
    component_name = "eCAL"
    component_dir  = repo_root_dir

    sbom = {}
    sbom[component_name] =  {}
    sbom[component_name]["include_type"] =              [
                                                            ecal_license_utils.include_type.COPY_IN_REPO,
                                                            ecal_license_utils.include_type.WINDOWS_BINARIES,
                                                            ecal_license_utils.include_type.LINUX_BINARIES,
                                                            ecal_license_utils.include_type.PYTHON_BINDINGS,
                                                            ecal_license_utils.include_type.DOCUMENTATION,
                                                            ecal_license_utils.include_type.TESTING,
                                                        ]
    sbom[component_name]["path"] =                      component_dir
    sbom[component_name]["license"] =                   "Apache-2.0"
    sbom[component_name]["license_files"] =             [
                                                            os.path.join(sbom[component_name]["path"], "LICENSE.txt"),
                                                        ]
    sbom[component_name]["thirdparty_license_files"] =  []
    sbom[component_name]["copyright"] =                 ecal_license_utils.get_copyright_from_file(sbom[component_name]["license_files"][0], skip_lines=188)
    sbom[component_name]["homepage"] =                  "http://ecal.io"
    sbom[component_name]["repo_url"] =                  "https://github.com/eclipse-ecal/ecal"
    sbom[component_name]["git_version"] =               None
    sbom[component_name]["git_version_url"] =           None

    return sbom

def update_license_dirs(ecal_sbom_dict, sbom_dict):
    licenses_target_dir        = os.path.realpath(os.path.join(os.path.dirname(__file__), "../LICENSES_all"))
    python_licenses_target_dir = os.path.realpath(os.path.join(os.path.dirname(__file__), "../LICENSES_python_bindings"))

    # Make sure that the target directories exist but is empty
    os.makedirs(licenses_target_dir, exist_ok=True)
    os.makedirs(python_licenses_target_dir, exist_ok=True)

    # Clear the contents of the target directories
    for target_dir in [licenses_target_dir, python_licenses_target_dir]:
        for item in os.listdir(target_dir):
            item_path = os.path.join(target_dir, item)
            if os.path.isdir(item_path):
                shutil.rmtree(item_path)

    # Copy license files from the SBOM dictionary to the target directories
    copy_license_files_to(sbom_dict,      licenses_target_dir)
    copy_license_files_to(ecal_sbom_dict, licenses_target_dir)

    # Filter the SBOM dictionary for Python bindings only
    sbom_dict_python_only = dict(filter(lambda x: ecal_license_utils.include_type.PYTHON_BINDINGS in x[1]["include_type"], sbom_dict.items()))
    copy_license_files_to(sbom_dict_python_only, python_licenses_target_dir)
    copy_license_files_to(ecal_sbom_dict,        python_licenses_target_dir)

def update_documentation(thirdparty_sbom_dict):
    documentation_licenses_dir            = os.path.realpath(os.path.join(os.path.dirname(__file__), "../../doc/rst/license/"))
    documentation_thirdparty_licenses_dir = os.path.join(documentation_licenses_dir, "thirdparty_licenses")

    ############## Make component paths relative #################

    # Make all ["path"] entries relative to the repo dir
    for component in thirdparty_sbom_dict.values():
        if component["path"] and os.path.isabs(component["path"]):
            rel_path = os.path.relpath(component["path"], start=repo_root_dir)
            rel_path = rel_path.replace(os.sep, "/")
            component["path"] = "/" + rel_path

    ############## Copy licenses to documentation #################

    # Make sure that the target directories exist but is empty
    if os.path.isdir(documentation_thirdparty_licenses_dir):
        shutil.rmtree(documentation_thirdparty_licenses_dir)
    os.makedirs(documentation_thirdparty_licenses_dir, exist_ok=True)

    copy_license_files_to(thirdparty_sbom_dict, documentation_thirdparty_licenses_dir, lowercase_dirs=True, change_to_relative_path = True)

    ############## Main Thirdparty Licenses Table #################

    licenses_main_output_rst_file_path = os.path.join(documentation_licenses_dir, "thirdparty_licenses.rst")

    # Load the Jinja2 template
    script_dir = os.path.dirname(os.path.abspath(__file__))
    resource_path = os.path.join(script_dir, "jinja2")
    
    template_loader     = jinja2.FileSystemLoader(searchpath=resource_path)
    template_env        = jinja2.Environment(loader=template_loader)
    template            = template_env.get_template("licenses_main.rst.jinja")

    # Render the template with the context
    context = {
        'sbom_dict':                  thirdparty_sbom_dict,
        'to_escaped_component_name':  lambda name: escape_component_name(name, lowercase=True)
    }
    output = template.render(context)

    # Save the rendered template to a file
    with open(licenses_main_output_rst_file_path, "w") as f:
        f.write(output)

    ############## Individual component details #################

    for component_name, details in thirdparty_sbom_dict.items():
        escaped_component_name = escape_component_name(component_name, lowercase=True)
        details_output_file_path = os.path.join(documentation_thirdparty_licenses_dir, escaped_component_name + ".rst")

        # Load the Jinja2 template
        script_dir = os.path.dirname(os.path.abspath(__file__))
        resource_path = os.path.join(script_dir, "jinja2")
        
        template_loader     = jinja2.FileSystemLoader(searchpath=resource_path)
        template_env        = jinja2.Environment(loader=template_loader)
        template            = template_env.get_template("component_details.rst.jinja")

        # Render the template with the context
        context = {
            'component_name':         component_name,
            'component_details':      details,
            'escaped_component_name': escaped_component_name,
            'ecal_license_utils':     ecal_license_utils
        }
        output = template.render(context)

        # Save the rendered template to a file
        with open(details_output_file_path, "w") as f:
            f.write(output)



if __name__ == "__main__":
    # Create a dictionary to hold License information from all thirdparty modules
    thirdparty_sbom_dict = {}
    for sbom_module in sbom_module_list:
        # Apend the SBOM from each module to the dictionary
        sbom = sbom_module.get_sbom()
        thirdparty_sbom_dict.update(sbom)

    # Also create an SBOM Dict for eCAL itself
    ecal_sbom_dict = get_ecal_sbom()

    # Update the LICENSES/... dirs
    update_license_dirs(ecal_sbom_dict, thirdparty_sbom_dict)

    # Generate the documentation
    thirdparty_sbom_dict_for_documentation = copy.deepcopy(thirdparty_sbom_dict)
    thirdparty_sbom_dict_for_documentation = OrderedDict(
        sorted(thirdparty_sbom_dict_for_documentation.items(), key=lambda x: x[0].lower())
    )
    
    update_documentation(thirdparty_sbom_dict_for_documentation)
