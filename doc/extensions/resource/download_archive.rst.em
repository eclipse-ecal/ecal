.. include:: /include.txt

.. _download_archive:

============
All releases
============

Here you can find a list of all eCAL Versions ever released on GitHub.

@{latest_branch_counter = 0}@
@[for release_branch in reversed(list(download_pages.keys()))]@
@{
latest_branch_counter = latest_branch_counter + 1
branch_is_supported   = (latest_branch_counter <= 2)

latest_release_in_branch_counter = 0
}@
eCAL @(release_branch.major).@(release_branch.minor)
============

@[    if not branch_is_supported]
*eCAL @(release_branch.major).@(release_branch.minor) has reached its end of life.*

@[    end if]
.. list-table:: 
   :widths: 3 3 4
   :header-rows: 1
   
   * - Release
     - Release Date
     - Support Status
@[    for ecal_version in reversed(list(download_pages[release_branch].keys()))]@
@{
latest_release_in_branch_counter = latest_release_in_branch_counter + 1
release_is_supported = (branch_is_supported and (latest_release_in_branch_counter <= 1))

if release_is_supported:
    support_status_string = "Supported"
else:
    support_status_string = "EOL"
}@
@{        current_ecal_version = download_pages[release_branch][ecal_version]}@
   * - :ref:`eCAL @(current_ecal_version["gh_release"].tag_name) <@(current_ecal_version["download_archive_page_rst_label"])>`
     - @(current_ecal_version["gh_release"].published_at.strftime("%Y-%m-%d"))
     - @(support_status_string)
@[    end for]

@[end for]@