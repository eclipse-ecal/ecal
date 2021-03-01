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
    os_group       = ""
    os_version     = semantic_version.Version("0.0.0")
    is_installer   = False
    is_python      = False
    python_version = semantic_version.Version("0.0.0")

    ext = os.path.splitext(asset_name)[1].lower()

    ubuntu_codename_dict = \
    {
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

    if ext == ".msi" or ext == ".exe":
        is_installer = True
        os_group     = "windows"

    elif ext == ".deb":
        is_installer = True
        os_group     = "ubuntu"

        if ecal_version <= semantic_version.Version("5.7.2"):
            # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
            os_version = semantic_version.Version("20.04.0")
        else:
            # Match properly named releases
            for ubuntu_codename in ubuntu_codename_dict:
                if ubuntu_codename in asset_name:
                    os_version = ubuntu_codename_dict[ubuntu_codename]
                    break
        
        if os_version == semantic_version.Version("0.0.0"):
            sys.stderr.write("Warning: Unable to match ubuntu installer by ubuntu codenames: \"" + asset_name + "\" (from eCAL " + str(ecal_version) + ")\n")

    
    elif ext == ".dmg":
        is_installer = True
        os_group     = "macos"
    
    elif ext == ".egg" or ext == ".whl":
        is_python = True

        # ================================
        # ==== Match operating system
        # ================================

        if ecal_version <= semantic_version.Version("5.7.2") and "linux" in asset_name:
            # Special case for old releases. They only had Ubuntu 20.04 releases that were just named "linux".
            os_version = semantic_version.Version("20.04.0")
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
            for index in range(2,3):
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

    return (os_group, os_version, is_installer, is_python, python_version)
    

def get_downloads_list(gh_assets, ecal_version):
    
    download_list = []

    for asset in gh_assets:
        asset_name = asset.name
        (os_group, os_version, is_installer, is_python, python_version) = get_asset_properties(asset_name, ecal_version)

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
                "ecal_installer_link":   "",
                "python_download_links": [],
            }
            download_list.append(download_dict)
        else:
            # Use existing download dict
            download_dict = existing_dicts[0]

        if is_installer:
            download_dict["ecal_installer_link"] = asset.browser_download_url
        elif is_python:
            download_dict["python_download_links"].append((python_version, asset.browser_download_url))

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

if __name__ == "__main__":
    gh = github.Github("")

    gh_ecal_repo = gh.get_repo("continental/ecal")
    gh_releases  = gh_ecal_repo.get_releases()

    gh_release_branches_dict = group_gh_release_branches(gh_releases)
           
    # for rel_branch in gh_release_branches_dict:
    #     print("========= eCAL " + str(rel_branch.major) + "." + str(rel_branch.minor) + " =========")
    #     for rel in gh_release_branches_dict[rel_branch]:
    #         print ("  " + str(rel))

    all_branches = sorted(gh_release_branches_dict.keys())
    
    latest_branch   = all_branches[-1]
    previous_branch = all_branches[-2]

    latest_branch_latest_release_version   = sorted(gh_release_branches_dict[latest_branch].keys())[-1]
    previous_branch_latest_release_version = sorted(gh_release_branches_dict[previous_branch].keys())[-1]
    
    latest_branch_latest_release_downloads   = get_downloads_list(gh_release_branches_dict[latest_branch][latest_branch_latest_release_version].get_assets(), latest_branch_latest_release_version)
    previous_branch_latest_release_downloads = get_downloads_list(gh_release_branches_dict[previous_branch][previous_branch_latest_release_version].get_assets(), previous_branch_latest_release_version)

    print(str(latest_branch_latest_release_downloads))
    print(str(previous_branch_latest_release_downloads))

    a = 1



    #rate_limit = g.get_rate_limit()
    #print("Rate Limit: " + str(rate_limit.core))

    #ecal_repo = g.get_repo("continental/ecal")

    #releases = ecal_repo.get_releases()
    #for release in releases:
    #    if release.prerelease:
    #        continue
    #    a = 1
    #    print("")
    #    print(release.title)
    #    print("===========")
    #    print("Link to Github: " + release.html_url)
    #    print("Sourcecode: " + release.zipball_url)
    
    #    assets = release.get_assets()
    #    for asset in assets:
    #        print("- " + asset.browser_download_url + " (Downloads: " + str(asset.download_count) + ")")

    #        a = 1
     
    input = "resource/__download_table.html.em"
    output = "autogen/__download_table.html"
    
    empy_helpers.expand_template(input, { "downloads" : latest_branch_latest_release_downloads}, pathlib.Path(output))

    pass
    