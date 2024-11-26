# -*- coding: utf-8 -*-
#
# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys

# -- Variable setup ----------------------------------------------------------

try:
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from conf_py_buildvars import *
    
    is_cmake_build = True
    
except ImportError:
    print("WARNING: conf_py_buildvars NOT imported. Setting in-source-build variables.")
    ecal_python_dir             = ''
    sphinx_custom_extension_dir = r'../extensions'
    rst_source_dir              = '.'
    ecal_source_root_dir        = '../..'
    
    is_cmake_build              = False

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#

if is_cmake_build:
    sys.path.insert(0, ecal_python_dir)

sys.path.insert(0, sphinx_custom_extension_dir)

# -- Generate ecalicons include file-------------------------------------------

import generate_ecalicons
qresource_list = [
    os.path.join(ecal_source_root_dir, r"app/iconset/ecalicons.qrc"),
    os.path.join(ecal_source_root_dir, r"lib/QEcalParser/resources/qecalparser.qrc"),
    os.path.join(ecal_source_root_dir, r"app/mon/mon_gui/resources/resources.qrc"),
    os.path.join(ecal_source_root_dir, r"app/play/play_gui/resources/resources.qrc"),
    os.path.join(ecal_source_root_dir, r"app/rec/rec_gui/resources/resources.qrc"),
    os.path.join(ecal_source_root_dir, r"app/sys/sys_gui/resources/resources.qrc"),
    os.path.join(ecal_source_root_dir, r"app/util/launcher/resources/resources.qrc")
]
generate_ecalicons.generate_ecalicons(qresource_list, os.path.join(rst_source_dir, r"_include_ecalicons.txt"))

# -- Generate download archive and tables for the homepage --------------------
import generate_download_tables

download_tables_main_page_dir = os.path.join(rst_source_dir, r"_download_main_page")
download_archive_dir          = os.path.join(rst_source_dir, r"_download_archive")
ppa_tabs_file                 = os.path.join(rst_source_dir, r"getting_started/_ppa_tabs.rst.txt")

if not os.path.exists(download_tables_main_page_dir) or not os.path.exists(download_archive_dir):
    # Only generate download tables, if the directories do not exist.
    # Otherwise we may run out of API calls very quickly.
    gh_api_key = os.getenv("ECAL_GH_API_KEY")
    if gh_api_key:
        os.makedirs(download_tables_main_page_dir)
        os.makedirs(download_archive_dir)
        generate_download_tables.generate_download_tables(gh_api_key, download_tables_main_page_dir, download_archive_dir, ppa_tabs_file)
    else:  
        print("WARNING: Environment variable ECAL_GH_API_KEY not set. Skipping generating download tables.")

# -- Project information -----------------------------------------------------

project = u'Eclipse eCAL™'
copyright = u'2023, Continental'
#author = u'Continental'

# The short X.Y version
version = u''
# The full version, including alpha/beta/rc tags
release = u''


# -- General configuration ---------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    #'sphinx_book_theme',
    'sphinx_tabs.tabs',
    'sphinx.ext.githubpages',
    'sphinx.ext.todo',
    'sphinxcontrib.youtube',
]

if is_cmake_build:
    extensions += [ \
    #    'sphinxcontrib.moderncmakedomain',
        'breathe',
        'exhale',
    #    'ini-directive'
    ]
    
    # For some reason the apidoc crashes on macOS
    if sys.platform.lower() != "darwin":
        extensions += [ \
            'sphinx.ext.autodoc',
            'sphinxcontrib.apidoc',
        ]

# Todo Configurations
if is_cmake_build:
    todo_include_todos = False
    todo_emit_warnings = True

apidoc_module_dir = os.path.join(ecal_python_dir)
apidoc_output_dir = os.path.join(rst_source_dir, '_api_python')
apidoc_excluded_paths = ['tests']
apidoc_separate_modules = True
apidoc_extra_args = ['-f']

# Breathe Configuration
breathe_default_project = "eCAL"

# Setup the exhale extension
exhale_args = {
    # These arguments are required
    "containmentFolder":     "./_api",
    "rootFileName":          "ecal_root.rst",
    "rootFileTitle":         "C++",
    "doxygenStripFromPath":  ecal_source_root_dir,
    # Suggested optional arguments
    "createTreeView":        True,
    # TIP: if using the sphinx-bootstrap-theme, you need
    # "treeViewIsBootstrap": True,
    "exhaleExecutesDoxygen": False
    #"exhaleDoxygenStdin":    "INPUT = ../include"
}

# Tell sphinx what the primary language being documented is.
#primary_domain = 'cpp'

# Tell sphinx what the pygments highlight language should be.
#highlight_language = 'cpp'


# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = None

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = [u'_build', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
# pygments_style = 'sphinx'

# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'sphinx_book_theme'


# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_css_files = [
    'css/bignums.css',                              # Enable the bignum feature from the sphinx-typo3-theme
    'css/sphinx-book-theme-1.1.2-ecaladdon.css',    # Change colors of the sphinx-book-theme
    'css/tabs-3.4.5-ecaladdon.css',                 # Change colors of the sphinx-tabs
]

html_title = "Eclipse eCAL™"
html_logo = "_static/img/ecal-logo.svg"
html_favicon = "_static/img/favicon.png"

html_theme_options = {
    "logo_only": True,
    "show_navbar_depth": 1,
    "show_toc_level": 2,
    "repository_url": "https://github.com/eclipse-ecal/ecal/",
    "use_repository_button": True,
    "use_issues_button": True,
    "use_edit_page_button": True,
    "repository_branch": "master",
    "path_to_docs": "doc/rst/",
    "extra_navbar": "", # => Remove the default text
    "footer_start": ["footer.html"],
    "extra_footer": '',
}

# -- Options for HTMLHelp output ---------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'eCALdoc'

# C++ defines used in function definitions
cpp_id_attributes = ['ECAL_API', 'ECALTIME_API']
