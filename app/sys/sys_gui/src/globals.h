/* ========================= eCAL LICENSE =================================
 *
 * Copyright (C) 2016 - 2020 Continental Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * ========================= eCAL LICENSE =================================
*/

#pragma once

#include "ecalsys/ecal_sys.h"
#include "ecalsys/esys_defs.h"

#include <QPixmap>
#include <QString>

class Globals
{
public:
  static EcalSys* EcalSysInstance()
  {
    static EcalSys* ecalsys_instance = new EcalSys();
    return ecalsys_instance;
  }

  class Icons
  {
  public:
    Icons() = delete;

    static QPixmap startAll()
    {
      static QPixmap icon(":/ecalicons/START");
      return icon;
    }

    static QPixmap stopAll()
    {
      static QPixmap icon(":/ecalicons/STOP");
      return icon;
    }

    static QPixmap restartAll()
    {
      static QPixmap icon(":/ecalicons/RESTART");
      return icon;
    }

    static QPixmap startSelected()
    {
      static QPixmap icon(":/ecalicons/START_SELECTED");
      return icon;
    }

    static QPixmap stopSelected()
    {
      static QPixmap icon(":/ecalicons/STOP_SELECTED");
      return icon;
    }

    static QPixmap restartSelected()
    {
      static QPixmap icon(":/ecalicons/RESTART_SELECTED");
      return icon;
    }

    static QPixmap add()
    {
      static QPixmap icon(":/ecalicons/ADD");
      return icon;
    }

    static QPixmap edit()
    {
      static QPixmap icon(":/ecalicons/EDIT");
      return icon;
    }

    static QPixmap duplicate()
    {
      static QPixmap icon(":/ecalicons/DUPLICATE");
      return icon;
    }

    static QPixmap remove()
    {
      static QPixmap icon(":/ecalicons/DELETE");
      return icon;
    }

    static QPixmap warning()
    {
      static QPixmap icon(":/ecalicons/WARNING");
      return icon;
    }
  };

  // Version number for storing settings
  static constexpr int ecalSysVersion()
  {
    return (ECAL_SYS_VERSION_PATCH
      + (ECAL_SYS_VERSION_MINOR * 1000)
      + (ECAL_SYS_VERSION_MAJOR * 1000 * 1000));
  }
};