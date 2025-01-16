.. include:: /include.txt

.. _thirdparty_licenses:

===================
External components
===================

Parts of eCAL have a different license than the main project.
These components may either be shipped with the repository as source code, or along with / included in the eCAL binaries.
The information in this list may not be exhaustive.
Some dependencies, like GoogleTest, are not used in our officially distributed builds, even though they can be used with eCAL and therefore appear in this list.

Components by Eclipse eCAL™
===========================

The following external components are developed and maintained by the Eclipse eCAL team.
They may have a different license than eCAL.

.. list-table:: Components by Eclipse eCAL™
   :widths: 10 15 30 45
   :header-rows: 1

   * - Component
     - License
     - Copyright
     - Included in...

   * - `fineFTP Server <https://github.com/continental/fineftp-server>`_
     - :ref:`mit_license`
     - 2020 Continental Corporation
     - - |fa-github| Git submodule ``/tirdparty/fineftp-server``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `tcp_pubsub <https://github.com/continental/tcp_pubsub>`_
     - :ref:`mit_license`
     - Copyright (c) 2021 Continental
     - - |fa-github| Git submodule ``/tirdparty/tcp_pubsub``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `Udpcap <https://github.com/eclipse-ecal/udpcap>`_
     - :ref:`apache_2_0`
     - 2022, Continental
     - - |fa-github| Git submodule ``/tirdparty/udpcap``
       - |fa-windows| Binary distributions for Windows

Thirdparty components
=====================

The following components are developed by third parties.
They are used by Eclipse eCAL, but not associated to it in any way.

.. list-table:: Thirdparty components
   :widths: 10 15 30 45
   :header-rows: 1

   * - Component
     - License
     - Copyright
     - Included in...

   * - `Asio <https://think-async.com>`_
     - :ref:`boost_1_0`
     - 2003-2018 Christopher M. Kohlhoff
     - - |fa-github| Git submodule ``/tirdparty/asio``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `Bootstrap <https://getbootstrap.com/>`_
     - :ref:`mit_license`
     - 2011-2019 The Bootstrap Authors

       2011-2019 Twitter, Inc.
     - - |fa-book| Documentation
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `Cap'n Proto <https://capnproto.org>`_
     - :ref:`mit_license`
     - 2013-2014 Sandstorm Development Group, Inc. and contributors
     - - |fa-ubuntu| Binary distributions for Linux

   * - convert_utf
     - :ref:`convert_utf_license`
     - 2001-2004 Unicode, Inc.
     - - |fa-file-alt| Copy in repository: :file:`convert_utf.cpp / .h`
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `empy_helpers <https://github.com/ros2/rosidl/blob/master/rosidl_adapter/rosidl_adapter/resource/__init__.py>`_
     - :ref:`apache_2_0`
     - 2018 Open Source Robotics Foundation, Inc.
     - - |fa-file-alt| Copy in repository: :file:`/doc/extensions/empy_helpers/__init__.py` (modified)

   * - `Findqwt.cmake <https://gitlab.kitware.com/cmake/community/-/wikis/contrib/modules/FindQwt>`_
     - :ref:`findqwt-bsd-2`
     - Copyright 2010-2013, Julien Schueller

       Copyright 2018-2020, Rolf Eike Beer
     - - |fa-file-alt| Copy in repository (modified): ``cmake/Modules/Findqwt.cmake``

   * - `Flaticons <https://www.flaticon.com/>`_
     - :ref:`creative_commons_3_0`
     - see :ref:`icon_licenses`
     - - |fa-file-alt| Copy in repository: :file:`/app/iconset/flaticon/*`
       - |fa-book| Documentation
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `Font Awesome <https://fontawesome.com>`_
     - :ref:`fontawesome_free_license`
     - Fonticons, Inc.
     - - |fa-book| Documentation
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `Google Protobuf <https://developers.google.com/protocol-buffers>`_
     - :ref:`bsd_3`
     - Copyright 2008 Google Inc.
     - - |fa-github| Git submodule ``/tirdparty/protobuf``
       - |fa-windows| Binary distributions for Windows

   * - `Google Test <https://github.com/google/googletest>`_
     - :ref:`bsd_3`
     - Copyright 2008 Google Inc.
     - - |fa-github| Git submodule ``/tirdparty/googletest``

   * - `HDF5 <https://www.hdfgroup.org/solutions/hdf5>`_
     - :ref:`hdf5_bsd_3`
     - 1998-2006 by The Board of Trustees of the University of Illinois
       
       2006 by The HDF Group
     - - |fa-github| Git submodule ``/tirdparty/hdf5``
       - |fa-windows| Binary distributions for Windows

   * - `libcurl <https://curl.se/libcurl/>`_
     - :ref:`mit_license`
     - 1996 - 2021, Daniel Stenberg, daniel@haxx.se, and many contributors, see the THANKS file
     - - |fa-github| Git submodule ``/tirdparty/curl``
       - |fa-windows| Binary distributions for Windows

   * - `modpath.iss <http://www.legroom.net/software>`_
     - :ref:`lgpl_3_0`
     - Jared Breland
     - - |fa-file-alt| Copy in repository: :file:`/cpack/innosetup/modpath.iss`
       - |fa-windows| Binary distributions for Windows (Installer only)

   * - `npcap <https://nmap.org/npcap/>`_
     - :ref:`npcap_license`
     - 2013-2021 by Insecure.Com LLC
     - Not included due to licensing issues.
       Please check out the :ref:`npcap chapter <configuration_npcap>`.
       eCAL for Windows links against the npcap SDK.

   * - `PcapPlusPlus <https://pcapplusplus.github.io/>`_
     - :ref:`unlicense_license`
     - 2021 seladb
     - - |fa-windows| Binary distributions for Windows

   * - `Qt <https://www.qt.io/>`_
     - :ref:`lgpl_3_0`
     - The Qt Company Ltd.
     - - |fa-windows| Binary distributions for Windows

   * - `qwt <https://sourceforge.net/projects/qwt/>`_
     - :ref:`qwt_license`
     - 	Uwe Rathmann
     - - |fa-github| Git submodule ``/tirdparty/qwt/qwt``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `recycle <https://github.com/steinwurf/recycle>`_
     - :ref:`bsd_3`
     - Copyright (c) 2014, Steinwurf ApS
     - - |fa-github| Git submodule ``/tirdparty/recycle``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `SimpleIni <https://github.com/brofield/simpleini>`_
     - :ref:`mit_license`
     - 2006-2013 Brodie Thiesfield
     - - |fa-github| Git submodule ``/tirdparty/simpleini``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `spdlog <https://github.com/gabime/spdlog>`_
     - :ref:`mit_license`
     - 2016 Gabi Melman
     - - |fa-github| Git submodule ``/tirdparty/spdlog``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `Sphinx Book Theme <https://github.com/executablebooks/sphinx-book-theme>`_
     - :ref:`bsd_3`
     - 2020 Chris Holdgraf
     - - |fa-book| Documentation
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `tclap <http://tclap.sourceforge.net>`_
     - :ref:`mit_license`
     - Copyright (c) 2003 Michael E. Smoot 
       
       Copyright (c) 2004 Daniel Aarno

       Copyright (c) 2017 Google Inc.
     - - |fa-github| Git submodule ``/tirdparty/tclap``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `termcolor <https://github.com/ikalnytskyi/termcolor>`_
     - :ref:`bsd_3`
     - 2013, Ihor Kalnytskyi
     - - |fa-github| Git submodule ``/tirdparty/termcolor``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `TinyXML-2 <http://www.grinninglizard.com/tinyxml2/>`_
     - :ref:`zlib_license`
     - 	Lee Thomason
     - - |fa-github| Git submodule ``/tirdparty/tinyxml``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

   * - `yaml-cpp <https://github.com/jbeder/yaml-cpp/>`_
     - :ref:`mit_license`
     - 	Copyright (c) 2008-2015 Jesse Beder
     - - |fa-github| Git submodule ``/thirdparty/yaml-cpp``
       - |fa-windows| Binary distributions for Windows
       - |fa-ubuntu| Binary distributions for Linux

License texts
=============

.. note::

  Even though the GPL License is included here, eCAL does not use any GPL code.
  The license is only listed here, as it is required as base for LGPL licenses.

.. toctree::

   thirdparty_licenses/apache-2.0.rst
   thirdparty_licenses/boost-1.0.rst
   thirdparty_licenses/bsd-3.rst
   thirdparty_licenses/convert-utf.rst
   thirdparty_licenses/creative-commons-3.0.rst
   thirdparty_licenses/findqwt-bsd-2.rst
   thirdparty_licenses/fontawesome-free.rst
   thirdparty_licenses/gpl-2.0.rst
   thirdparty_licenses/gpl-3.0.rst
   thirdparty_licenses/hdf5-bsd-3.rst
   thirdparty_licenses/lgpl-2.1.rst
   thirdparty_licenses/lgpl-3.0.rst
   thirdparty_licenses/mit.rst
   thirdparty_licenses/npcap.rst
   thirdparty_licenses/qwt.rst
   thirdparty_licenses/unlicense.rst
   thirdparty_licenses/zlib.rst
