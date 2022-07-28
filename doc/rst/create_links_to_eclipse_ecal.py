import os
import sys
from fnmatch import fnmatch

def listFiles(dir, patterns):
    """
    Creates a list of all files in the given directory (and all subdirectories)
    that match the given pattern.

    Parameters
    ----------
    dir: String
        the directory to look in

    patterns: List of String
        Patterns for fnmatch. The function will only list files that match the
        pattern. Example: patterns = ["*.txt"]

    Returns
    -------
    list [String]
        A list of all files as relative / absolute path (whatever the input path
        was)
    """
    file_list = []
    for path, subdirs, files in os.walk(dir):
        for name in files:
            for p in patterns:
                if fnmatch(name, p):
                    file_list += [os.path.join(path, name)]
                    break
    return file_list

def getNewRstContent(rel_rst_file_path):
    rel_html_path = rel_rst_file_path[:-3] + "html"
    rel_html_path = rel_html_path.replace("\\", "/")

    return R"""===================
eCAL has been moved
===================

eCAL has been moved under the umbrella of the Eclipse Foundation.
This documentation page can be found in the new Eclise eCAL Documentation:

https://eclipse-ecal.github.io/ecal/""" + rel_html_path + "\n"


if __name__ == "__main__":
    patterns = ["*.rst"]

    file_list = listFiles(os.getcwd(), patterns)
    file_list = list(map(lambda f: os.path.relpath(f, os.getcwd()), file_list))

    for file in file_list:
        with open(file, 'w') as f:
            f.write(getNewRstContent(file))