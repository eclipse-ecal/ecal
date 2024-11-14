#!/usr/bin/env python3

import os
import sys
import subprocess
import argparse

def get_submodule_list(repo_path):
    # Invoke git to list submodules. Then parse the output and return it as a list
    result = subprocess.run("git submodule foreach --quiet \"echo $path\"", cwd=repo_path, capture_output=True)

    # If git command fails, print error message and exit
    if result.returncode != 0:
        print("ERROR: Failed to list submodules for \"" + repo_path + "\":")
        print(result.stderr.decode("utf-8").strip())
        sys.exit(1)

    submodule_list = result.stdout.decode("utf-8").split("\n")
    submodule_list = [x.strip() for x in submodule_list]
    submodule_list = [x for x in submodule_list if x]
    return submodule_list

def get_clearlydefined_id(submodule_path):
    # Get Git URL  
    url_result = subprocess.run("git config --get remote.origin.url", cwd=submodule_path, capture_output=True)
    git_url = url_result.stdout.decode("utf-8").strip()

    if not git_url:
        return ""

    # Strip the .git extension if present
    if git_url.endswith(".git"):
        git_url = git_url[:-4]

    # Get the owner/repo part of the URL, as that is the part that is used in the clearlydefined identifier
    if git_url.startswith("https://github.com/"):
        clearlydefined_identifier = "git/github/" + git_url[19:] + "/"
    elif git_url.startswith("git@github.com:"):
        clearlydefined_identifier = "git/github/" + git_url[15:] + "/"
    elif git_url.startswith("https://git.code.sf.net/p/"):
        project_name = git_url[26:]
        if project_name.endswith("/git"):
            project_name = project_name[:-4]
        clearlydefined_identifier = "git/sourceforge/" + project_name + "/" + project_name + "/"
    else:
        print ("WARNING: Unknown URL format, not adding to dependency list: " + git_url)
        return ""

    # Get Git Tag (if it exists)
    describe_result = subprocess.run("git describe --exact-match --tags", cwd=submodule_path, capture_output=True)
    git_ref = describe_result.stdout.decode("utf-8").strip()

    # Get commit hash (if tag does not exist)
    if not git_ref:
        rev_parse_result = subprocess.run("git rev-parse --verify HEAD", cwd=submodule_path, capture_output=True)
        git_ref = rev_parse_result.stdout.decode("utf-8").strip()

    clearlydefined_identifier = clearlydefined_identifier + git_ref

    return clearlydefined_identifier

def run_dash_license_tool(dependency_file_path):
    # Get the path to the dash-licensing tool from the environment variable
    dash_license_tool = os.environ.get("ECLIPSE_DASH_LICENSE_TOOL")
    if not dash_license_tool:
        print("ERROR: Environment variable ECLIPSE_DASH_LICENSE_TOOL not set")
        sys.exit(1)
    
    # Get the path to the GitLab token from the environment variable
    gitlab_token = os.environ.get("ECLIPSE_GITLAB_TOKEN")
    if not gitlab_token:
        print("ERROR: Environment variable ECLIPSE_GITLAB_TOKEN not set")
        sys.exit(1)

    print ("-- Running dash-licensing tool")
    # Run the dash-licensing tool
    result = subprocess.run("java -jar \"" + dash_license_tool + "\" -project automotive.ecal -summary summary.txt -review -token " + gitlab_token + " \"" + dependency_file_path + "\"", shell=True)

    # If the dash-licensing tool fails, print error message and exit
    if result.returncode != 0:
        print("ERROR: Failed to run the dash-licensing tool")
        sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Scans an (eCAL) repository for submodules and generates a list of clearlydefined identifiers in a file. The output can be used automatically or manually by the dash-licensing tool to scan the licenses. Visit https://github.com/eclipse-dash/dash-licenses for more information.")
    parser.add_argument("--repo", help="Path to the local repository to scan. Default is current directory.")
    parser.add_argument("--output", help="Path to the output file. Default is dependencies.txt in the current directory.")
    parser.add_argument("--run-dash", action="store_true", help="Run the dash-licensing tool after generating the dependency list. Needs the ECLIPSE_DASH_LICENSE_TOOL (-> path to the dash-license.jar) and ECLIPSE_GITLAB_TOKEN (API token for creating reviews) environment variables to be set.")

    args = parser.parse_args()

    # Get the path to the repository to scan
    repo_path = args.repo
    if not repo_path:
        repo_path = os.getcwd()
    repo_path = os.path.abspath(repo_path)

    # Get the path to the output file
    output_path = args.output
    if not output_path:
        output_path = os.path.join(os.getcwd(), "dependencies.txt")
    output_path = os.path.abspath(output_path)

    # Get list of submodules
    submodule_list = get_submodule_list(repo_path)

    print ("-- Found " + str(len(submodule_list)) + " submodules in " + repo_path)

    # Get clearlydefined identifier for each submodule
    clearlydefined_ids = []
    for submodule in submodule_list:
        submodule_path = os.path.join(repo_path, submodule)
        clearlydefined_id = get_clearlydefined_id(submodule_path)
        if clearlydefined_id:
            clearlydefined_ids.append(clearlydefined_id)

    # Hardcoded dependencies that are not submodules
    clearlydefined_ids.append("git/github/seladb/PcapPlusPlus/v22.11")
    clearlydefined_ids.append("git/github/qt/qtbase/v6.6.1")
    clearlydefined_ids.append("git/github/nanopb/nanopb/0.4.9")
    
    # Write the clearlydefined identifiers to the output file
    with open(output_path, 'w') as f:
        for clearlydefined_id in clearlydefined_ids:
            f.write(clearlydefined_id + "\n")

    print("-- " + str(len(clearlydefined_ids)) + " dependencies written to " + output_path)

    if (args.run_dash):
        run_dash_license_tool(output_path)
    else:
        # print instructions for the user on how to call the dash-licensing tool now
        print("")
        print("To scan the dependencies, run the following command:")
        print("")
        print("  java -jar %ECLIPSE_DASH_LICENSE_TOOL% -project automotive.ecal -summary summary.txt -review -token %ECLIPSE_GITLAB_TOKEN% \"" + output_path + "\"")