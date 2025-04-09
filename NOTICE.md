# Notices for Eclipse eCAL

This content is produced and maintained by the Eclipse eCAL project.

* Project home: https://projects.eclipse.org/projects/automotive.ecal

## Trademarks

eCAL™ is a trademark of the Eclipse Foundation.

## Copyright

All content is the property of the respective authors or their employers. For
more information regarding authorship of content, please consult the listed
source code repository logs.

## Declared Project Licenses

This program and the accompanying materials are made available under the terms
of the Apache License, Version 2.0 which is available at
https://www.apache.org/licenses/LICENSE-2.0.

SPDX-License-Identifier: Apache-2.0

## Source Code

The project maintains the following source code repositories:

* https://github.com/eclipse-ecal/ecal
* https://github.com/eclipse-ecal/ecal-algorithm-samples
* https://github.com/eclipse-ecal/ecal-camera-samples
* https://github.com/eclipse-ecal/ecal-carla-bridge
* https://github.com/eclipse-ecal/ecal-core
* https://github.com/eclipse-ecal/ecal-foxglove-bridge
* https://github.com/eclipse-ecal/ecal-gpsd-client
* https://github.com/eclipse-ecal/ecal-matlab-binding
* https://github.com/eclipse-ecal/ecal-mcap-tools
* https://github.com/eclipse-ecal/ecal-mqtt-bridge
* https://github.com/eclipse-ecal/ecal-rs
* https://github.com/eclipse-ecal/ecal-utils
* https://github.com/eclipse-ecal/ecaludp
* https://github.com/eclipse-ecal/fineftp-server
* https://github.com/eclipse-ecal/protobuf-datatypes-collection
* https://github.com/eclipse-ecal/rmw_ecal
* https://github.com/eclipse-ecal/rosidl_typesupport_protobuf
* https://github.com/eclipse-ecal/tcp_pubsub
* https://github.com/eclipse-ecal/udpcap

## Third party Content

The following components are developed by third parties.
They are used by Eclipse eCAL, but not associated to it in any way.

### Components by Eclipse eCAL

The following external components are developed and maintained by the Eclipse eCAL team.
They may have a different license than eCAL.

ecaludp
  - License: Apache 2.0
  - Project: https://github.com/eclipse-ecal/ecaludp
  - Copyright: 2024, Continental
  - Included in:
    - Git submodule `/tirdparty/ecaludp/ecaludp`
    - Binary distributions for Windows
    - Binary distributions for Linux

fineFTP Server
  - License: MIT
  - Project: https://github.com/eclipse-ecal/fineftp-server
  - Copyright: 2020 Continental Corporation
  - Included in:
    - Git submodule `/tirdparty/fineftp/fineftp-server`
    - Binary distributions for Windows
    - Binary distributions for Linux

tcp_pubsub
  - License: MIT
  - Project: https://github.com/eclipse-ecal/tcp_pubsub
  - Copyright: 2021, Continental
  - Included in:
    - Git submodule `/tirdparty/tcp_pubsub/tcp_pubsub`
    - Binary distributions for Windows
    - Binary distributions for Linux

Udpcap
  - License: Apache 2.0
  - Project: https://github.com/eclipse-ecal/udpcap
  - Copyright: 2022, Continental
  - Included in:
    - Git submodule `/tirdparty/udpcap/udpcap`
    - Binary distributions for Windows

### Thirdparty components

The following components are developed by third parties.
They are used by Eclipse eCAL, but not associated to it in any way.

Asio
  - License: Boost 1.0
  - Project: https://think-async.com
  - Copyright: 2003-2018 Christopher M. Kohlhoff
  - Included in:
    - Git submodule `/tirdparty/asio/asio`
    - Binary distributions for Windows
    - Binary distributions for Linux

Bootstrap
  - License: MIT
  - Project: https://getbootstrap.com/
  - Copyright: 2011-2019 The Bootstrap Authors, 2011-2019 Twitter, Inc.
  - Included in:
    - Documentation
    - Binary distributions for Windows
    - Binary distributions for Linux

Cap'n Proto
  - License: MIT
  - Project: https://capnproto.org
  - Copyright: 2013-2014 Sandstorm Development Group, Inc. and contributors
  - Included in:
    - Binary distributions for Linux

convert_utf
  - License: Convert UTF License
  - Copyright: 2001-2004 Unicode, Inc.
  - Included in:
    - Copy in repository: `convert_utf.cpp / .h`
    - Binary distributions for Windows
    - Binary distributions for Linux

Findqwt.cmake
  - License: Modified BSD 2-Clause
  - Project: https://gitlab.kitware.com/cmake/community/-/wikis/contrib/modules/FindQwt
  - Copyright: 2010-2013, Julien Schueller; 2018-2020, Rolf Eike Beer
  - Included in:
    - Copy in repository (modified): `cmake/Modules/Findqwt.cmake`

Flaticons
  - License: Creative Commons 3.0
  - Project: https://www.flaticon.com/
  - Copyright: Multiple individual authors
  - Included in:
    - Copy in repository: `/app/iconset/flaticon/*`
    - Documentation
    - Binary distributions for Windows
    - Binary distributions for Linux

Font Awesome
  - License: Font Awesome Free License
  - Project: https://fontawesome.com
  - Copyright: Fonticons, Inc.
  - Included in:
    - Documentation
    - Binary distributions for Windows
    - Binary distributions for Linux

Google Protobuf
  - License: BSD 3-Clause
  - Project: https://developers.google.com/protocol-buffers
  - Copyright: 2008 Google Inc.
  - Included in:
    - Git submodule `/tirdparty/protobuf/protobuf`
    - Binary distributions for Windows

Google Test
  - License: BSD 3-Clause
  - Project: https://github.com/google/googletest
  - Copyright: 2008 Google Inc.
  - Included in:
    - Git submodule `/tirdparty/gtest/googletest`

HDF5
  - License: Modified BSD 3-Clause
  - Project: https://www.hdfgroup.org/solutions/hdf5
  - Copyright: 1998-2006 by The Board of Trustees of the University of Illinois, 2006 by The HDF Group
  - Included in:
    - Git submodule `/tirdparty/hdf5/hdf5`
    - Binary distributions for Windows

libcurl
  - License: MIT
  - Project: https://curl.se/libcurl/
  - Copyright: 1996 - 2021, Daniel Stenberg, daniel@haxx.se, and many contributors (see the THANKS file)
  - Included in:
    - Git submodule `/tirdparty/curl/curl`
    - Binary distributions for Windows

modpath.iss
  - License: LGPL 3.0
  - Project: http://www.legroom.net/software
  - Copyright: Jared Breland
  - Included in:
    - Copy in repository: `/cpack/innosetup/modpath.iss`
    - Binary distributions for Windows (Installer only)

nanopb
  - License: Zlib License
  - Project: https://github.com/nanopb/nanopb
  - Copyright: 2011 Petteri Aimonen \<jpa at nanopb.mail.kapsi.fi\>
  - Included in:
    - Copy in repository: `ecal/ecal/core/src/serialization/nanopb/*`
    - Binary distributions for Windows
    - Binary distributions for Linux

npcap
  - License: Npcap License
  - Project: https://nmap.org/npcap/
  - Copyright: 2013-2021 by Insecure.Com LLC
  - Not included due to licensing issues. eCAL for Windows links against the npcap SDK.

PcapPlusPlus
  - License: Unlicense
  - Project: https://pcapplusplus.github.io/
  - Copyright: 2021 seladb
  - Included in:
    - Binary distributions for Windows

Qt
  - License: LGPL 3.0
  - Project: https://www.qt.io/
  - Copyright: The Qt Company Ltd.
  - Included in:
    - Binary distributions for Windows

qwt
  - License: Qwt License
  - Project: https://sourceforge.net/projects/qwt/
  - Copyright: Uwe Rathmann
  - Included in:
    - Git submodule `/tirdparty/qwt/qwt`
    - Binary distributions for Windows
    - Binary distributions for Linux

recycle
  - License: BSD 3-Clause
  - Project: https://github.com/steinwurf/recycle
  - Copyright: 2014, Steinwurf ApS
  - Included in:
    - Git submodule `/tirdparty/recycle/recycle`
    - Binary distributions for Windows
    - Binary distributions for Linux

spdlog
  - License: MIT
  - Project: https://github.com/gabime/spdlog
  - Copyright: 2016 Gabi Melman
  - Included in:
    - Git submodule `/tirdparty/spdlog/spdlog`
    - Binary distributions for Windows
    - Binary distributions for Linux

Sphinx Book Theme
  - License: BSD 3-Clause
  - Project: https://github.com/executablebooks/sphinx-book-theme
  - Copyright: 2020 Chris Holdgraf
  - Included in:
    - Documentation
    - Binary distributions for Windows
    - Binary distributions for Linux

tclap
  - License: MIT
  - Project: http://tclap.sourceforge.net
  - Copyright: 2003 Michael E. Smoot, 2004 Daniel Aarno, 2017 Google Inc.
  - Included in:
    - Git submodule `/tirdparty/tclap/tclap`
    - Binary distributions for Windows
    - Binary distributions for Linux

termcolor
  - License: BSD 3-Clause
  - Project: https://github.com/ikalnytskyi/termcolor
  - Copyright: 2013, Ihor Kalnytskyi
  - Included in:
    - Git submodule `/tirdparty/termcolor/termcolor`
    - Binary distributions for Windows
    - Binary distributions for Linux

TinyXML-2
  - License: Zlib License
  - Project: https://github.com/leethomason/tinyxml2
  - Copyright: Lee Thomason
  - Included in:
    - Git submodule `/tirdparty/tinyxml2/tinyxml2`
    - Binary distributions for Windows
    - Binary distributions for Linux

yaml-cpp
  - License: MIT
  - Project: https://github.com/jbeder/yaml-cpp/
  - Copyright: 2008-2015 Jesse Beder
  - Included in:
    - Git submodule `/thirdparty/yaml-cpp/yaml-cpp`
    - Binary distributions for Windows
    - Binary distributions for Linux

abseil
  - License: Apache 2.0
  - Project: https://github.com/abseil/abseil-cpp
  - Copyright: 2017 The Abseil Authors.
  - Included in:
    - Git submodule `/thirdparty/absl/absl`
    - Binary distributions for Windows
    - Binary distributions for Linux

## Cryptography

Content may contain encryption software. The country in which you are currently
may have restrictions on the import, possession, and use, and/or re-export to
another country, of encryption software. BEFORE using any encryption software,
please check the country's laws, regulations and policies concerning the import,
possession, or use, and re-export of encryption software, to see if this is
permitted.
