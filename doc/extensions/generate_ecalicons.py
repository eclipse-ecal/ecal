import xml.etree.ElementTree as ET
import os
import posixpath
import sys

def generate_ecalicons(qt_resource_file_list, output_file_path):
    icons_dict = {}
    for qt_resource_path in qt_resource_file_list:
        this_dict = __read_qt_resource_file(qt_resource_path)
        icons_dict = {**icons_dict, **this_dict}  # => Merge the two dictionaries

    sizes = {
        "" : 16,
        "h1" : 38,
        "h2" : 33,
        "h3" : 28,
        # "16" : 16,
        # "32" : 32,
        # "48" : 48,
        # "64" : 64,
    }

    rst_code = []

    for icon_alias in icons_dict:
        icon_path = icons_dict[icon_alias]
        for size_name in sizes:

            size_alias = icon_alias
            if size_name != "":
                size_alias = size_alias + "_" + size_name
            size = sizes[size_name]

            rst_code.append(__get_rst_code(size_alias, icon_path, size))

    with open(output_file_path, 'w') as output_file:
        output_file.write("".join(rst_code))

def __get_rst_code(alias, path, size):
    posix_path = path.replace(os.sep, posixpath.sep)
    return ".. |" + alias + "| image:: " + posix_path + "\n   :width: " + str(size) + "px" + "\n\n"

def __read_qt_resource_file(qt_resource_file):
    ecalicons_dict = {}
    base_path      = os.path.abspath(os.path.dirname(qt_resource_file))

    tree     = ET.parse(qt_resource_file)
    rcc_root = tree.getroot()

    for qresource_child in rcc_root:

        # Skip everything that is not a QResource
        if qresource_child.tag != "qresource":
            continue

        # Evaluate the prefix
        prefix = ""
        if ("prefix" in qresource_child.attrib):
            prefix = qresource_child.attrib["prefix"]
            if (prefix.startswith("/")):
                prefix = prefix[1:]
            prefix.replace("/", "_")
        
        # Iterate over all "file" children
        for file_child in qresource_child:

            # Skip when this is by some means not a file
            if file_child.tag != "file":
                continue
            
            # Read attributes
            file_alias = ""
            file_path  = ""

            if ("alias" in file_child.attrib):
                file_alias = file_child.attrib["alias"]

            file_path = file_child.text

            if file_alias == "":
                file_alias = os.path.basename(file_path)

            # Unfortunatelly, sphinx wants to interpret paths as relative paths
            # and simply removes a leading "/". This is no issue on Windows,
            # as absolute paths start with a drive-letter.
            # On non-Windows OS we however have to add a second "/", so Sphinx
            # can remove one of them and we still end up with an absolute path.

            if sys.platform.startswith('win32'):
                # No workaround needed
                ecalicons_dict[prefix + "_" + file_alias] = os.path.join(base_path, file_path)
            else:
                # Add leading "/"
                ecalicons_dict[prefix + "_" + file_alias] = "/" + os.path.join(base_path, file_path)
    
    return ecalicons_dict

if __name__ == "__main__":
    qresource_list = [
        r"../../app/iconset/ecalicons.qrc",
        r"../../lib/QEcalParser/resources/qecalparser.qrc",
        r"../../app/mon/mon_gui/resources/resources.qrc",
        r"../../app/play/play_gui/resources/resources.qrc",
        r"../../app/rec/rec_gui/resources/resources.qrc",
        r"../../app/sys/sys_gui/resources/resources.qrc",
        r"../../app/util/launcher/resources/resources.qrc"
    ]

    generate_ecalicons(qresource_list, "../rst/_include_ecalicons.txt")