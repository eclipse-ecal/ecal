@{
import os

#ecal_version = "5.8.3"
#
#downloads = [
#    {
#        "os_group" :             "windows",
#        "os_name" :              "Windows",
#        "ecal_installer_link" :  ["https://github.com/continental/ecal/releases/download/v5.8.3/ecal_5.8.3-win64.exe"],
#        "python_download_links" : [
#                ("3.9" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#                ("3.8" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#                ("3.7" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#                ("3.6" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#                ("3.5" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#            ],
#    },
#
#    {
#        "os_group" :             "ubuntu",
#        "os_name" :              "Ubuntu 18.04",
#        "ecal_installer_link" :  ["https://github.com/continental/ecal/releases/download/v5.8.3/ecal_5.8.3-bionic_amd64.deb"],
#        "python_download_links" : [
#                ("3.8" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#            ],
#    },
#
#    {
#        "os_group" :             "ubuntu",
#        "os_name" :              "Ubuntu 20.04",
#        "ecal_installer_link" :  ["https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.8-focal_amd64.egg"],
#        "python_download_links" : [
#                ("3.8" , "https://github.com/continental/ecal/releases/download/v5.8.3/ecal-5.8.3_py3.9-win64.egg"),
#            ],
#    },
#]
}@
<div class="table-responsive"><table class="colwidths-given docutils align-default" id="ecal-@(str(ecal_version).replace('.', '-'))-download">
<colgroup>
<col style="width: 20%">
<col style="width: 80%">
</colgroup>
<tbody>
@{odd_row_counter = 0}@
@[for download_dict in downloads]@
@{
odd_row_counter += 1
odd_row_counter = odd_row_counter % 2
if odd_row_counter == 1:
    table_row_class = "row-odd"
else:
    table_row_class = "row-even"

if download_dict["os_group"] == "windows":
    os_icon_name = "fab fa-windows"
    os_name      = "Windows"
elif download_dict["os_group"] == "ubuntu":
    os_icon_name = "fab fa-ubuntu"
    os_name      = "Ubuntu " + str(download_dict["os_version"].major) + "." + str(download_dict["os_version"].minor).rjust(2, '0')
elif download_dict["os_group"] == "macos":
    os_icon_name = "fab fa-apple"
    os_name      = "macOS"
elif download_dict["os_group"] == "source":
    os_icon_name = "fa fa-code"
    os_name      = "Source"
else:
    os_icon_name = "fas fa-download"
    os_name      = "Other"
}@
@[for download_link in download_dict["ecal_installer_link"]]@
@{
file_name           = os.path.split   (download_link)[1]
installer_extension = os.path.splitext(download_link)[1]
if download_dict["os_group"] == "source" and ("fat" in file_name):
    download_button_suffix = " (Includes git submodules)"
else:
    download_button_suffix = ""
}@
<tr class="@(table_row_class)"><td><p><i class="@(os_icon_name)"></i> @(os_name)</p></td>
@[    if download_dict["os_group"] == "source"]@
<td><p><a class="reference external" href="@(download_link)"><i class="fas fa-download"></i> @(file_name)@(download_button_suffix)</a></p>
@[    else]@
<td><p><a class="reference external" href="@(download_link)"><i class="fas fa-download"></i> Download eCAL @(str(ecal_version)) installer for @(os_name) (@(installer_extension))</a></p>
@[    end if]@
@[end for]@
@[    if len(download_dict["python_download_links"]) > 0]@
<ul class="simple">
@[        for (python_version, python_download_link) in download_dict["python_download_links"]]@
@{
python_binding_extension = os.path.splitext(python_download_link)[1]
}@
<li><p><a class="reference external" href="@(python_download_link)"><i class="fab fa-python"></i> Python @(python_version.major).@(python_version.minor) integration for eCAL @(str(ecal_version)) on @(os_name) (@(python_binding_extension))</a></p></li>
@[        end for]@
</ul>
@[    end if]@
</td>
</tr>
@[end for]@
</tbody>
</table></div>
</div>