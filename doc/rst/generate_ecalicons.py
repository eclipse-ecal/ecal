import xml.etree.ElementTree as ET
import os
import posixpath

def generate_ecalicons(qt_resource_file, output_file_path):
    base_path  = os.path.abspath(os.path.dirname(qt_resource_file))
    icons_dict = __read_qt_resource_file(qt_resource_file)

    sizes = {
        "" : 16,
        "16" : 16,
        "20" : 20,
        "32" : 32,
        "48" : 48,
        "64" : 64,
    }

    rst_code = []

    for icon_alias in icons_dict:
        icon_path = os.path.join(base_path, icons_dict[icon_alias])
        for size_name in sizes:

            size_alias = icon_alias
            if size_name != "":
                size_alias = size_alias + "_" + size_name
            size = sizes[size_name]

            rst_code.append(get_rst_code(size_alias, icon_path, size))

    with open(output_file_path, 'w') as output_file:
        output_file.write("".join(rst_code))

def get_rst_code(alias, path, size):
    posix_path = path.replace(os.sep, posixpath.sep)
    return ".. |" + alias + "| image:: " + posix_path + "\n   :width: " + str(size) + "px" + "\n\n"

def __read_qt_resource_file(qt_resource_file):
    ecalicons_dict = {}

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

            ecalicons_dict[prefix + "_" + file_alias] = file_path
    
    return ecalicons_dict

if __name__ == "__main__":
    # generate_ecalicons(r"D:\Projects\ecal\ecal-githubcom\app\iconset\ecalicons.qrc", "ecalicons.txt")
    generate_ecalicons(r"D:/Projects/ecal/ecal-githubcom/app/iconset/ecalicons.qrc", "ecalicons.txt")