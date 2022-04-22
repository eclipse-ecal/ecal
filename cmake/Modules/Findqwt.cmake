# Qt Widgets for Technical Applications
# available at http://www.http://qwt.sourceforge.net/
#
# The module defines the following variables:
#  qwt_FOUND - the system has Qwt
#  qwt_INCLUDE_DIR - where to find qwt_plot.h
#  qwt_INCLUDE_DIRS - qwt includes
#  qwt_LIBRARY - where to find the Qwt library
#  qwt_LIBRARIES - aditional libraries
#  qwt_VERSION_STRING - version (ex. 5.2.1)
#
# It also defines this imported target:
#  qwt::qwt

#=============================================================================
# Copyright 2010-2013, Julien Schueller
# Copyright 2018-2020, Rolf Eike Beer
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met: 
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer. 
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution. 
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies, 
# either expressed or implied, of the FreeBSD Project.
#=============================================================================

if(Qt5Gui_FOUND)
  get_target_property(QT_INCLUDE_DIR Qt5::Gui INTERFACE_INCLUDE_DIRECTORIES)
endif()

find_path(qwt_INCLUDE_DIR
  NAMES qwt_plot.h
  HINTS ${QT_INCLUDE_DIR}
  PATH_SUFFIXES qwt qwt-qt5 qwt6
)

set(qwt_INCLUDE_DIRS ${qwt_INCLUDE_DIR})

# version
set(_VERSION_FILE ${qwt_INCLUDE_DIR}/qwt_global.h)
if(EXISTS ${_VERSION_FILE})
  file(STRINGS ${_VERSION_FILE} _VERSION_LINE REGEX "define[ ]+QWT_VERSION_STR")
  if(_VERSION_LINE)
    string(REGEX REPLACE ".*define[ ]+QWT_VERSION_STR[ ]+\"([^\"]*)\".*" "\\1" qwt_VERSION_STRING "${_VERSION_LINE}")
  endif()
endif()
unset(_VERSION_FILE)

find_library(qwt_LIBRARY
  NAMES qwt qwt-qt5
  HINTS ${QT_LIBRARY_DIR}
)

set(qwt_LIBRARIES ${qwt_LIBRARY})

include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(qwt REQUIRED_VARS qwt_LIBRARY qwt_INCLUDE_DIR VERSION_VAR qwt_VERSION_STRING)

if (qwt_FOUND AND NOT TARGET qwt::qwt)
  add_library(qwt::qwt UNKNOWN IMPORTED)
  set_target_properties(qwt::qwt PROPERTIES
                        INTERFACE_INCLUDE_DIRECTORIES "${qwt_INCLUDE_DIRS}"
                        IMPORTED_LOCATION "${qwt_LIBRARIES}")
endif ()

mark_as_advanced (
  qwt_LIBRARY
  qwt_INCLUDE_DIR
)

