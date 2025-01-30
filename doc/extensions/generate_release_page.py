import os

import sys
import re
import github
import jinja2
import semantic_version
from collections import OrderedDict

ubuntu_default_python_version_dict = \
{
    semantic_version.Version("18.4.0"):  semantic_version.Version("3.6.0"),
    semantic_version.Version("20.4.0"):   semantic_version.Version("3.8.0"),
    semantic_version.Version("22.4.0"):   semantic_version.Version("3.10.0"),
    semantic_version.Version("24.4.0"):   semantic_version.Version("3.12.0"),
}

ubuntu_codename_dict = \
{
    "noble":   semantic_version.Version("24.4.0"),
    "jammy":   semantic_version.Version("22.4.0"),
    "focal":   semantic_version.Version("20.4.0"),
    "bionic":  semantic_version.Version("18.4.0"),
    "xenial":  semantic_version.Version("16.4.0"),
    "trusty":  semantic_version.Version("14.4.0"),
}

"""
Retrieves a dictionary of release branches and their corresponding releases from the GitHub repository.
Args:
    gh_repo (github.Github): An authenticated GitHub instance.
Returns:
    dict: A dictionary where the keys are minor relase numbers (-> patch level
            set to 0) and Values are Dictionaries of
            {specific_release : gh_release_object}
Example:
    {
        Version('5.8.0'): {
            Version('5.8.0'): <github.Repository.Release object at 0x...>,
            Version('5.8.1'): <github.Repository.Release object at 0x...>
        },
        Version('5.9.0'): {
            Version('5.9.0'): <github.Repository.Release object at 0x...>
        }
    }
"""
def get_releases_dict(gh_repo):
    gh_releases  = gh_repo.get_releases()

    gh_release_branches_dict = OrderedDict()

    for gh_release in gh_releases:
        if gh_release.prerelease or gh_release.draft:
            continue
        
        version_string = gh_release.tag_name

        if version_string.startswith("v") or version_string.startswith("V"):
            version_string = version_string[1:]
        if version_string.startswith("."):
            version_string = version_string[1:]

        # Fix format, so it can be parsed by semantic_version:
        dot_components = version_string.split(".")
        if len(dot_components) == 4:
            version_string = '.'.join(dot_components[:-1]) + "+" + dot_components[3]
        elif len(dot_components) == 5:
            version_string = '.'.join(dot_components[:-2]) + "-" + dot_components[3] + "+" + dot_components[4]

        try:
            version = semantic_version.Version(version_string)    
        except:
            sys.stderr.write("Warning: eCAL Release \"" + gh_release.tag_name + "\" is not parsable to a proper version.\n")
            continue
            
        version        = semantic_version.Version(version_string)
        release_branch = semantic_version.Version(major = version.major, minor = version.minor, patch = 0)

        if not release_branch in gh_release_branches_dict:
            # Initialize dicitonary for this branch
            gh_release_branches_dict[release_branch] = OrderedDict()

        gh_release_branches_dict[release_branch][version] = gh_release

    # Sort the minor eCAL Release branches by version
    gh_release_branches_dict = OrderedDict(sorted(gh_release_branches_dict.items(), key = lambda x: x[0], reverse = True))

    # Sort the specific releases by version
    for release_branch in gh_release_branches_dict:
        gh_release_branches_dict[release_branch] = OrderedDict(sorted(gh_release_branches_dict[release_branch].items(), key = lambda x: x[0], reverse = True))

    return gh_release_branches_dict

"""
Retrieves the properties of a given GitHub asset.

Args:
    ecal_version (semantic_version.Version):    The version of eCAL.
    gh_asset (github.Asset):                    The GitHub asset object.

Returns:
    dict: A dictionary containing the properties of the asset, including:

    {
        'filename' :     'actual filename',
        'download_link': 'browser download link',
        'type' :         'source / ecal_installer / python_binding',
        'properties' :   SEE BELOW,
    }

        Properties for "source":
        {}

        Properties for "ecal_installer":
        {
            'os':             'windows / macos / ubuntu',
            'os_version':     Semver('0.0.0') (Only for Ubuntu specific installers),
            'cpu':            'amd64 / arm64'
        }

        Properties for "python_binding":
        {
            'os':                       'manylinux / macos / windows / ubuntu',
            'os_version':               Semver('0.0.0') (Only for Ubuntu specific bindings),
            'cpu':                      'amd64 / arm64',
            'python_version':           Semver('3.6.0') (example)
            'python_implementation':    'cp / pp etc.' (i.e. CPython or PyPy)
        }
"""
def get_asset_properties(ecal_version, gh_asset):
    asset_properties = {
        'filename' :     '',
        'download_link': '',
        'type' :         '',
        'properties' :   {},
    }

    asset_properties['filename']      = gh_asset.name
    asset_properties['download_link'] = gh_asset.browser_download_url

    # Source
    if asset_properties['filename'].endswith('tar.gz'):
        asset_properties['type'] = 'source'

    # eCAL Installer for Windows
    elif asset_properties['filename'].endswith('.msi') or asset_properties['filename'].endswith('.exe'):
        asset_properties['type'] = 'ecal_installer'
        asset_properties['properties']['os'] = 'windows'
        asset_properties['properties']['cpu'] = 'amd64'

    # eCAL installer for macOS
    elif asset_properties['filename'].endswith('.dmg'):
        asset_properties['type'] = 'ecal_installer'
        asset_properties['properties']['os'] = 'macos'
        asset_properties['properties']['cpu'] = 'amd64'

    # eCAL Installer for Linux
    elif asset_properties['filename'].endswith('.deb'):
        asset_properties['type'] = 'ecal_installer'
        asset_properties['properties']['os'] = 'ubuntu'
        asset_properties['properties']['os_version'] = semantic_version.Version('0.0.0')
        asset_properties['properties']['cpu'] = 'amd64'

        if ecal_version <= semantic_version.Version("5.7.2"):
            # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
            asset_properties['properties']['os_version'] = semantic_version.Version("20.4.0")
        else:
            for codename, version in ubuntu_codename_dict.items():
                if codename in asset_properties['filename']:
                    asset_properties['properties']['os_version'] = version
                    break
        
        if ecal_version <= semantic_version.Version("5.7.2"):
            # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
            asset_properties['properties']['os_version'] = semantic_version.Version("20.4.0")

    # Python binding (whl)
    elif asset_properties['filename'].endswith('.whl'):
        asset_properties['type'] = 'python_binding'
        
        # Get Operating system
        if 'manylinux' in asset_properties['filename']:
            asset_properties['properties']['os'] = 'manylinux'
        elif 'darwin' in asset_properties['filename'] or 'macos' in asset_properties['filename']:
            asset_properties['properties']['os'] = 'macos'
        elif "win64" in asset_properties['filename'] or "win_amd64" in asset_properties['filename']:
            asset_properties['properties']['os'] = 'windows'
        elif 'linux' in asset_properties['filename']:
            # Old eCAL 5.x wheels were Ubuntu specific
            asset_properties['properties']['os'] = 'ubuntu'
            for codename, version in ubuntu_codename_dict.items():
                    if codename in asset_properties['filename']:
                        asset_properties['properties']['os_version'] = version
                        break
        else:
            sys.stderr.write("Warning: Unable to determine OS of python binding: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")

        # Get Python CPU Architecture
        filename_without_extension = os.path.splitext(asset_properties['filename'])[0]
        if filename_without_extension.endswith('amd64') or filename_without_extension.endswith('x86_64') or filename_without_extension.endswith('win64'):
            asset_properties['properties']['cpu'] = 'amd64'
        elif filename_without_extension.endswith('arm64') or filename_without_extension.endswith('aarch64'):
            asset_properties['properties']['cpu'] = 'arm64'
        else:
            sys.stderr.write("Warning: Unable to determine CPU Architecture of python binding: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")

        # Get Python version
        python_version        = semantic_version.Version("0.0.0")
        python_implementation = ''
        components = asset_properties['filename'][:-4].split('-')
        # The python version is either index 2 or 3, depending on whether the optional build tag is used.
        for index in range(2,4):
            if re.match(r"[a-z]{2}[0-9]+", components[index]):
                python_implementation = components[index][:2]
                python_version_major_string = components[index][2]
                python_verstion_minor_string = "0"
                if len(components[index]) > 3:
                    python_verstion_minor_string = components[index][3:]
                python_version = semantic_version.Version(python_version_major_string + "." + python_verstion_minor_string + ".0")
                break

        if python_version == semantic_version.Version("0.0.0"):
            sys.stderr.write("Warning: Unable to determine python version: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")
        if not python_implementation:
            sys.stderr.write("Warning: Unable to determine python implementation: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")
        
        asset_properties['properties']['python_version']        = python_version
        asset_properties['properties']['python_implementation'] = python_implementation

    # Python binding (.egg)
    elif asset_properties['filename'].endswith('.egg'):
        asset_properties['type'] = 'python_binding'

        # Get Operating system
        if 'darwin' in asset_properties['filename'] or 'macos' in asset_properties['filename']:
            asset_properties['properties']['os'] = 'macos'
        elif "win64" in asset_properties['filename'] or "win_amd64" in asset_properties['filename']:
            asset_properties['properties']['os'] = 'windows'
        elif 'linux' in asset_properties['filename'] or 'bionic' in asset_properties['filename'] or 'focal' in asset_properties['filename']:
            asset_properties['properties']['os'] = 'ubuntu'
            if ecal_version <= semantic_version.Version("5.7.2"):
                # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
                asset_properties['properties']['os_version'] = semantic_version.Version("20.4.0")
            else:
                for codename, version in ubuntu_codename_dict.items():
                    if codename in asset_properties['filename']:
                        asset_properties['properties']['os_version'] = version
                        break
        
        if not asset_properties['properties'].get('os'):
            sys.stderr.write("Warning: Unable to determine OS of python binding: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")

        # Get python version
        python_version = semantic_version.Version("0.0.0")
        python_match_result = re.findall(r"py[0-9]+\.[0-9]+", asset_properties['filename'])
        if len(python_match_result) > 0:
            python_version = semantic_version.Version(python_match_result[0][2:] + ".0")

        if python_version == semantic_version.Version("0.0.0"):
            sys.stderr.write("Warning: Unable to determine python version: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")
        
        asset_properties['properties']['python_version']        = python_version
        asset_properties['properties']['python_implementation'] = 'cp'              # The eggs were all CPython

        # CPU Architecture was always amd64 back then
        asset_properties['properties']['cpu'] = 'amd64'

    # Warning, as we have no idea what this file is for
    else:
        sys.stderr.write("Warning: Unknown asset type: \"" + asset_properties['filename'] + "\" (from eCAL " + str(ecal_version) + ")\n")

    return asset_properties

def generate_release_index_page(releases_dict, list_of_supported_minor_versions, output_filename):
    # Create the output directory if it does not exist
    output_dir = os.path.dirname(output_filename)
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Load the Jinja2 template
    template_loader     = jinja2.FileSystemLoader(searchpath="resource/")
    template_env        = jinja2.Environment(loader=template_loader)
    template_file       = "release_page_index.rst.jinja"
    template            = template_env.get_template(template_file)

    # Render the template with the context
    context = {
        'releases_dict': releases_dict,
        'list_of_supported_minor_versions': list_of_supported_minor_versions,
        'group_asset_list_by_os_and_arch': group_asset_list_by_os_and_arch,
        'get_rst_release_page_label': get_rst_release_page_label,
    }
    output = template.render(context)

    # Save the rendered template to a file
    with open(output_filename, "w") as f:
        f.write(output)

def generate_release_page(gh_release,
                           ecal_version,
                           asset_list,
                           minor_is_supported,
                           latest_version_of_minor,
                           output_dir):
    ecal_version_string  = str(ecal_version).replace('.', '_').replace('-', '_').replace('+', '_')
    output_filename      = "ecal_" + ecal_version_string + ".rst"
    changelog_file       = "changelog_ecal_" + ecal_version_string + ".txt"

    # Create the output directory if it does not exist
    if output_dir and not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Save the changelog to a file
    changelog = gh_release.body
    changelog = changelog.replace('\r\n', '\n')
    with open(os.path.join(output_dir, changelog_file), "w") as f:
        f.write(changelog)

    # Load the Jinja2 template
    template_loader     = jinja2.FileSystemLoader(searchpath="resource/")
    template_env        = jinja2.Environment(loader=template_loader)
    template_file       = "release_page.rst.jinja"
    template            = template_env.get_template(template_file)

    # Render the template with the context
    context = {
        'gh_release':                           gh_release,
        'ecal_version':                         ecal_version,
        'asset_list':                           asset_list,
        'changelog_file_path':                  changelog_file,
        'minor_is_supported':                   minor_is_supported,
        'latest_version_of_minor':              latest_version_of_minor,
        'ubuntu_default_python_version_dict':   ubuntu_default_python_version_dict,
        'group_asset_list_by_os_and_arch':      group_asset_list_by_os_and_arch,
        'get_rst_release_page_label':           get_rst_release_page_label,
    }
    output = template.render(context)

    # Save the rendered template to a file
    path = os.path.join(output_dir, output_filename)
    with open(path, "w") as f:
        f.write(output)

"""
Groups a list of assets by their OS, OS version, and CPU architecture.
Args:
    asset_list (list): A list of asset properties dictionaries.
Returns:
    dict: A dictionary where the keys are tuples of (os, os_version, cpu) and
            the values are lists of asset properties dictionaries.
"""
def group_asset_list_by_os_and_arch(asset_list):
    os_arch_dict = OrderedDict()

    for asset_properties in asset_list:
        os         = asset_properties['properties'].get('os', 'unknown')
        os_version = asset_properties['properties'].get('os_version', semantic_version.Version('0.0.0'))
        cpu        = asset_properties['properties'].get('cpu', 'unknown')

        key = (os, os_version, cpu)
        if key not in os_arch_dict:
            os_arch_dict[key] = []
        os_arch_dict[key].append(asset_properties)

    return os_arch_dict

def get_rst_release_page_label(ecal_version):
    return "ecal_release_page_" + str(ecal_version).replace('.', '_').replace('-', '_').replace('+', '_')

def generate_release_documentation(gh_api_key, index_filepath, release_dir_path):
    gh = github.Github(gh_api_key)
    gh_repo = gh.get_repo("eclipse-ecal/ecal")
    releases_dict = get_releases_dict(gh_repo)

    list_of_supported_minor_versions = list(releases_dict.keys())[:2]

    generate_release_index_page(releases_dict,
                                list_of_supported_minor_versions,
                                index_filepath)

    for minor_version in releases_dict:

        # Get the latest eCAL Version of this release branch
        latest_release_for_this_minor = list(releases_dict[minor_version].keys())[0]
        this_minor_is_supported       = minor_version in list_of_supported_minor_versions

        for ecal_version in releases_dict[minor_version]:
            
            # Check the support status of this release

            gh_release = releases_dict[minor_version][ecal_version]
            gh_asset_list = gh_release.get_assets()

            asset_list = []

            for gh_asset in gh_asset_list:
                asset_properties = get_asset_properties(ecal_version, gh_asset)
                asset_list.append(asset_properties)

            # Sort the asset list by:
            # 1. OS (windows > manylinux > ubuntu > macos)
            # 2. OS version (descending)
            # 3. CPU (amd64 > arm64)
            # 4. Python version (descending)

            asset_list.sort(key = lambda x: x['properties'].get('python_version', semantic_version.Version("0.0.0")), reverse = True)
            asset_list.sort(key = lambda x: x['properties'].get('os_version', semantic_version.Version("0.0.0")), reverse = True)
            asset_list.sort(key = lambda x: x['properties'].get('cpu', 'unknown'))
            asset_list.sort(key = lambda x: ['windows', 'manylinux', 'ubuntu', 'macos'].index(x['properties'].get('os', 'unknown')) if x['properties'].get('os', 'unknown') in ['windows', 'manylinux', 'ubuntu', 'macos'] else float('inf'))

            generate_release_page(gh_release,
                                    ecal_version,
                                    asset_list,
                                    this_minor_is_supported,
                                    latest_release_for_this_minor,
                                    release_dir_path)

if __name__=="__main__":
    gh_api_key = os.getenv("ECAL_GH_API_KEY")
    if gh_api_key:
        generate_release_documentation(gh_api_key, "index.rst", "release")
    else:
        sys.stderr.write("ERROR: Environment variable ECAL_GH_API_KEY not set. Without an API key, GitHub will not provide enough API calls to generate the download tables.\n")
        exit(1)







