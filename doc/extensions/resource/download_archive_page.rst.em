====================
eCAL @(ecal_version)
====================

- Release Date: @(gh_release.published_at.strftime("%Y-%m-%d"))

- GitHub Release Page: @(gh_release.html_url)

@{
changelog = gh_release.body
changelog = changelog.replace('\\n', '\n')
}@

@[if changelog_file != ""]@
Changelog
=========

.. literalinclude:: @(changelog_file)
   :language: txt

@[end if]@
Downloads
=========

.. raw:: html
   :file: @(download_table_html_file)