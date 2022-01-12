import github
import em
import sys
import os
import re
from io import StringIO

import empy_helpers

import pathlib
import semantic_version


def group_gh_release_branches(gh_releases):
    gh_release_branches_dict = {}

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
            gh_release_branches_dict[release_branch] = {}

        gh_release_branches_dict[release_branch][version] = gh_release

    return gh_release_branches_dict

def get_asset_properties(asset_name, ecal_version):
    os_group       = "other"
    os_version     = semantic_version.Version("0.0.0")
    is_python      = False
    python_version = semantic_version.Version("0.0.0")

    ext = os.path.splitext(asset_name)[1].lower()

    ubuntu_codename_dict = \
    {
        "jammy":   semantic_version.Version("22.4.0"),
        "impish":  semantic_version.Version("21.10.0"),
        "hirsute": semantic_version.Version("21.4.0"),
        "groovy":  semantic_version.Version("20.10.0"),
        "focal":   semantic_version.Version("20.4.0"),
        "eoan":    semantic_version.Version("19.10.0"),
        "disco":   semantic_version.Version("19.4.0"),
        "cosmic":  semantic_version.Version("18.10.0"),
        "bionic":  semantic_version.Version("18.4.0"),
        "artful":  semantic_version.Version("17.10.0"),
        "zesty":   semantic_version.Version("17.4.0"),
        "yakkety": semantic_version.Version("16.10.0"),
        "xenial":  semantic_version.Version("16.4.0"),
        "wily":    semantic_version.Version("15.10.0"),
        "vivid":   semantic_version.Version("15.4.0"),
        "utopic":  semantic_version.Version("14.10.0"),
        "trusty":  semantic_version.Version("14.4.0"),
    }

    if asset_name.lower().endswith(".tar.gz"):
        os_group     = "source"

    elif ext == ".msi" or ext == ".exe":
        os_group     = "windows"

    elif ext == ".deb":
        os_group     = "ubuntu"

        if ecal_version <= semantic_version.Version("5.7.2"):
            # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
            os_version = semantic_version.Version("20.4.0")
        else:
            # Match properly named releases
            for ubuntu_codename in ubuntu_codename_dict:
                if ubuntu_codename in asset_name:
                    os_version = ubuntu_codename_dict[ubuntu_codename]
                    break
        
        if os_version == semantic_version.Version("0.0.0"):
            sys.stderr.write("Warning: Unable to match ubuntu installer by ubuntu codenames: \"" + asset_name + "\" (from eCAL " + str(ecal_version) + ")\n")

    
    elif ext == ".dmg":
        os_group     = "macos"
    
    elif ext == ".egg" or ext == ".whl":
        is_python = True

        # ================================
        # ==== Match operating system
        # ================================

        if ecal_version <= semantic_version.Version("5.7.2") and "linux" in asset_name:
            # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
            os_group   = "ubuntu"
            os_version = semantic_version.Version("20.4.0")
        else:
            if "darwin" in asset_name or "macosx" in asset_name:
                os_group = "macos"
            elif "win64" in asset_name or "win_amd64" in asset_name:
                os_group = "windows"
            else:
                for ubuntu_codename in ubuntu_codename_dict:
                    if ubuntu_codename in asset_name:
                        os_group   = "ubuntu"
                        os_version = ubuntu_codename_dict[ubuntu_codename]
                        break
        
        if os_group == "":
            sys.stderr.write("Warning: Unable to match python binding to an operating system: \"" + asset_name + "\" (from eCAL " + str(ecal_version) + ")\n")

        # ================================
        # ==== Match python version
        # ================================

        if ext == ".whl":
            components = asset_name[:-4].split('-')
            # The python version is either index 2 or 3, depending on whether the optional build tag is used.
            for index in range(2,4):
                if re.match(r"[a-zA-Z]{2}[0-9]+", components[index]):
                    python_version.major = int(components[index][2])
                    if len(components[index]) > 3:
                        python_version.minor = int(components[index][3:])
                    break
        elif ext == ".egg":
            python_match_result = re.findall(r"py[0-9]+\.[0-9]+", asset_name)
            if len(python_match_result) > 0:
                python_version = semantic_version.Version(python_match_result[0][2:] + ".0")

        if python_version == semantic_version.Version("0.0.0"):
            sys.stderr.write("Warning: Unable to determine python version: \"" + asset_name + "\" (from eCAL " + str(ecal_version) + ")\n")

    else:
        sys.stderr.write("Warning: Cannot classify file: \"" + asset_name + "\" (from eCAL " + str(ecal_version) + ")\n")

    return (os_group, os_version, is_python, python_version)

def get_downloads_list(gh_assets, ecal_version):
    
    download_list = []

    for asset in gh_assets:
        asset_name = asset.name
        (os_group, os_version, is_python, python_version) = get_asset_properties(asset_name, ecal_version)

        # Search for a download dict that already exists
        existing_dicts = []
        if os_group == "ubuntu":
            existing_dicts = list(filter(lambda d: d["os_group"] == os_group and d["os_version"] == os_version, download_list))
        else:
            existing_dicts = list(filter(lambda d: d["os_group"] == os_group, download_list))

        if len(existing_dicts) == 0:
            # Create new download dict
            download_dict = {
                "os_group":              os_group,
                "os_version":            os_version,
                "ecal_installer_link":   [],
                "python_download_links": [],
            }
            download_list.append(download_dict)
        else:
            # Use existing download dict
            download_dict = existing_dicts[0]


        if is_python:
            download_dict["python_download_links"].append((python_version, asset.browser_download_url))
        else:
            download_dict["ecal_installer_link"].append(asset.browser_download_url)

    # Sort python download links
    for download_dict in download_list:
        download_dict["python_download_links"] = sorted(download_dict["python_download_links"], 
                                                        key= lambda a: a[0])

    windows_download_links = list(filter(lambda d: d["os_group"] == "windows", download_list))
    ubuntu_download_links  = list(filter(lambda d: d["os_group"] == "ubuntu", download_list))
    macos_download_links   = list(filter(lambda d: d["os_group"] == "macos", download_list))

    other_download_links   = list(filter(lambda d: d["os_group"] != "windows" and d["os_group"] != "ubuntu" and d["os_group"] != "macos", download_list))

    ubuntu_download_links = sorted(ubuntu_download_links, key= lambda a: a["os_version"])

    return windows_download_links + ubuntu_download_links + macos_download_links + other_download_links

def get_ppa_enabled_releases(all_releases):
    ppa_enabled_releases = []

    for ecal_release in all_releases:
        if ecal_release >= semantic_version.Version("5.7.0"):
            ppa_enabled_releases.append(ecal_release)

    ppa_enabled_releases = sorted(ppa_enabled_releases, reverse = True)

    return ppa_enabled_releases

def generate_download_tables(gh_api_key, main_page_output_dir, download_archive_output_dir, ppa_tabs_output_file):
    gh = github.Github(gh_api_key)

    gh_ecal_repo = gh.get_repo("continental/ecal")
    gh_releases  = gh_ecal_repo.get_releases()

    gh_release_branches_dict = group_gh_release_branches(gh_releases)

    sorted_branches = sorted(gh_release_branches_dict.keys())
    
    latest_branch   = sorted_branches[-1]
    previous_branch = sorted_branches[-2]

    latest_branch_latest_release_version   = sorted(gh_release_branches_dict[latest_branch].keys())[-1]
    previous_branch_latest_release_version = sorted(gh_release_branches_dict[previous_branch].keys())[-1]
    
    latest_branch_latest_release_downloads   = get_downloads_list(gh_release_branches_dict[latest_branch][latest_branch_latest_release_version].get_assets(), latest_branch_latest_release_version)
    latest_branch_latest_release_downloads   = filter(lambda x: x["os_group"] != "macos", latest_branch_latest_release_downloads)
    previous_branch_latest_release_downloads = get_downloads_list(gh_release_branches_dict[previous_branch][previous_branch_latest_release_version].get_assets(), previous_branch_latest_release_version)
    previous_branch_latest_release_downloads = filter(lambda x: x["os_group"] != "macos", previous_branch_latest_release_downloads)

    latest_branch_latest_release_date   = gh_release_branches_dict[latest_branch][latest_branch_latest_release_version].published_at
    previous_branch_latest_release_date = gh_release_branches_dict[previous_branch][previous_branch_latest_release_version].published_at


    # ===========================
    # Main Page download tables
    # ===========================
    
    root_dir = os.path.dirname(os.path.realpath(__file__))
   
    # Generate heading and text
    data = {
        "ecal_latest_version":        latest_branch_latest_release_version,
        "ecal_latest_release_date":   latest_branch_latest_release_date,
        "ecal_previous_version":      previous_branch_latest_release_version,
        "ecal_previous_release_date": previous_branch_latest_release_date,
    }
    empy_helpers.expand_template(os.path.join(root_dir, "resource/main_page_download_section.rst.em"), data, pathlib.Path(os.path.join(main_page_output_dir, "_main_page_download_section.rst.txt")))

    # ===========================
    # Download Archive
    # ===========================

    ecal_branches_download_archive_pages = {}

    for ecal_branch in sorted_branches:
        # Collect the pages we generate to link them in the main page
        current_branch_download_archive_pages             = {}
        ecal_branches_download_archive_pages[ecal_branch] = current_branch_download_archive_pages

        sorted_releases = sorted(gh_release_branches_dict[ecal_branch].keys())
        for ecal_version in sorted_releases:
            gh_release                     = gh_release_branches_dict[ecal_branch][ecal_version]
            download_list                  = get_downloads_list(gh_release.get_assets(), ecal_version)
            ecal_version_string            = str(ecal_version).replace('.', '_').replace('-', '_').replace('+', '_')
            download_table_html_file_name  = "download_archive_table_ecal_" + ecal_version_string + ".html"
            download_archive_rst_file_name = "download_archive_ecal_" + ecal_version_string + ".rst"
            changelog_file                 = "changelog_ecal_" + ecal_version_string + ".txt"

            current_branch_download_archive_pages[ecal_version] = download_archive_rst_file_name

            # Generate download table as html rst code
            data = {
                "downloads" :    download_list, 
                "ecal_version" : ecal_version,
            }
            empy_helpers.expand_template(os.path.join(root_dir, "resource/download_table.html.em"), data, pathlib.Path(os.path.join(download_archive_output_dir, download_table_html_file_name)))

            # Generate changelog file if necessary
            changelog = gh_release.body
            changelog = changelog.replace('\r\n', '\n')
            if changelog  != "":
                with open(os.path.join(download_archive_output_dir, changelog_file), "w") as text_file:
                    text_file.write(changelog)
            else:
                changelog_file = ""

            # Generate surrounding rst code
            data = {
                "ecal_version" :            ecal_version,
                "download_table_html_file": download_table_html_file_name, 
                "gh_release":               gh_release,
                "changelog_file":           changelog_file,
            }
            empy_helpers.expand_template(os.path.join(root_dir, "resource/download_archive_page.rst.em"), data, pathlib.Path(os.path.join(download_archive_output_dir, download_archive_rst_file_name)))

    # Main Download archive page
    data = {
        "download_pages" :  ecal_branches_download_archive_pages
    }
    empy_helpers.expand_template(os.path.join(root_dir, "resource/download_archive.rst.em"), data, pathlib.Path(os.path.join(download_archive_output_dir, "download_archive.rst")))

    # ===========================
    # PPA tabs
    # ===========================

    ppa_enabled_releases = get_ppa_enabled_releases(sorted_branches)
    
    # Only show the last 3 Releases
    ppa_enabled_releases = ppa_enabled_releases[:3]

    data = {
        "ppa_list": ppa_enabled_releases
    }
    empy_helpers.expand_template(os.path.join(root_dir, "resource/ppa_tabs.rst.em"), data, pathlib.Path(ppa_tabs_output_file))

if __name__=="__main__":
    # This main function is meant for debugging purposes.
    gh_api_key = os.getenv("ECAL_GH_API_KEY")
    if gh_api_key:
        generate_download_tables(gh_api_key, "_autogen/main_page", "_autogen/download_archive", "_autogen/ppa_tabs.rst.txt")
    else:
        sys.stderr.write("ERROR: Environment variable ECAL_GH_API_KEY not set. Without an API key, GitHub will not provide enough API calls to generate the download tables.\n")
        exit(1)
    
